/*

  so, i think 'proper' usart will use dma.

 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>

#include <string.h> // strlen
//#include <stdarg.h>
// #include <stdio.h>

// using the ww library...
//#include "uartlib.h"
#include "miniprintf.h"

#include "usart.h"



/*
  This function should be moved to main(). it is *not* usart..
  We may just want to blink

*/

extern void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName);


void vApplicationStackOverflowHook(
  xTaskHandle *pxTask __attribute((unused)),
  signed portCHAR *pcTaskName __attribute((unused))
) {
  int i, j = 0, len;
    // ok seems to be catching some stack overflow conditions
    // this won't work - because relies on sending to queues.
    // we need to bit-bang the uart.
    // TODO - could probably still be cleaned up

    len = strlen((const char *)pcTaskName);

    for(;;) {
      for(i = 0; i <= len; ++i) {
        // usart_send(USART1, "overflow"[ i ] );
        if(i != len)
          usart_send(USART1, pcTaskName [ i ] );
        else
          usart_send(USART1, ' ' );

         for (j = 0; j < 3000000; j++) {
          __asm__("nop");
        }
      }
    }
}






///////////////////////////


// TODO - consider if should be global, global struct, or returned from usart_setup()
static QueueHandle_t uart_txq;


// think we have to use an interrupt - so we don't miss anything. eg. we take the character
// in the isr, before it is replaced
static QueueHandle_t uart_rxq;



void usart_setup(void)
{
  // this is sets up the rx interupt, but does not enable
  nvic_enable_irq(NVIC_USART1_IRQ); // JA

  /* Setup GPIO pins  */
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9  | GPIO10);

  // TODO - 100MHZ? only need tx bit to be set
  gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO10);

  // USART1 alternate function.
  gpio_set_af(GPIOA, GPIO_AF7, GPIO9  | GPIO10);

  /* Setup USART1 parameters. */
  // usart_set_baudrate(USART1, 38400);
  usart_set_baudrate(USART1, 115200); // JA
  usart_set_databits(USART1, 8);
  usart_set_stopbits(USART1, USART_STOPBITS_1);
  usart_set_mode(USART1, USART_MODE_TX_RX);

  usart_set_mode(USART1, USART_MODE_TX_RX);
  usart_set_parity(USART1, USART_PARITY_NONE);
  usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);

  /* Enable USART1 Receive interrupt. */
  usart_enable_rx_interrupt(USART1);

  /* Finally enable the USART. */
  usart_enable(USART1);

  // not sure but queues, should be created in main()
  // setup() could take the queue as argument for better composibility
  uart_txq = xQueueCreate(256,sizeof(char));
  uart_rxq = xQueueCreate(256,sizeof(char));

}



void usart1_isr(void)
{
  // Only thing this is doing - is echoing the output.
  // for console read I think we need blocking.
  static uint8_t data = 'A';


  /* Check if we were called because of RXNE. */
  if (((USART_CR1(USART1) & USART_CR1_RXNEIE) != 0) &&
      ((USART_SR(USART1) & USART_SR_RXNE) != 0)) {

    /* Retrieve the data from the peripheral. */
    data = usart_recv(USART1);


    /* Enable transmit interrupt so it sends back the data. */
    // JA - so this is a cheap way of echoing data???
    // usart_enable_tx_interrupt(USART1);

    xQueueSend(uart_rxq, &data ,portMAX_DELAY); /* blocks when queue is full */
                                                // if piping input to uart - then blocking is probably desirable,

    xQueueSend(uart_txq, &data,portMAX_DELAY);  /* blocks */
                                                // send to tx queue to echo back to console.
                                                // probably don't want to do this here. but somewhere else for more
                                                // control

    /*
    DISABLE - It is confusing, if stack overflow fast blink - or just normal logging.

    // Toggle LED to show signs of life
    gpio_toggle(GPIOE,GPIO0);
    */
  }

}




void uart_task(void *args __attribute__((unused))) {
  char ch;

  for (;;) {
    /*
      500 - is the time in ticks to wait. 1/2 second is too long?
    */
    if ( xQueueReceive(uart_txq,&ch,500) == pdPASS ) {
      while ( !usart_get_flag(USART1,USART_SR_TXE) )
        taskYIELD();  // Yield until ready
                      // JA - doesn't seem to use coroutines...
      usart_send(USART1,ch);
    }
    // Toggle LED to show signs of life
    // gpio_toggle(GPIOE,GPIO0);
  }
}

#if 0
static void uart_puts(const char *s) {

  for ( ; *s; ++s )
    xQueueSend(uart_txq,s,portMAX_DELAY); /* blocks when queue is full */
}
#endif

//////////////////


static void uart_putc(char ch) {
  xQueueSend(uart_txq, &ch ,portMAX_DELAY); /* blocks when queue is full */
}


int uart_printf(const char *format,...) {
  // very nice. writes to the uart_putc()/queue so no buffer to overflow
  // COOKED means that a CR is sent after every LF is sent out
  va_list args;
  int rc;

  va_start(args,format);
  rc = mini_vprintf_cooked(uart_putc,format,args);
  va_end(args);
  return rc;
}

// add motor control again - b.  with external power, chose
// needs to be real time - unless on interupt.
// control motor with rotary encoder.
// control motor with prompt.

// ok - motor is a loop. but i think there's an issue..
// cannot just have it running at some subdivision of 1ms. eg. if tick time.
// 1ms tick time is twice as fast as 2ms.

// or we multiplex the gpio signals with external ic logic?

// or we just have a high-priority interupt timer - that works independently of any other tasks?
// if the gpio update is running in an isr. then cannot see a problem.
// we need to code this. though independenty.
// so setting the timer - will set the speed...
// actually timer just has to call the isr.

// OK - EXTREME - we can use the setting of the OC value - to determine the speed.
// eg. in the interupt - we set the next oc value - for the next interrupt.  eg. would just add a delay.
// eg. the pwm-with-interrupts approach.

// actually don't need the delay - just use regular 50% pulse.


char *uart_gets( char *buf, size_t len) {
  // will truncate if overflows...
  char ch;
	char *p = buf;

  for (;;) {
    // Receive char to be TX
    // if( xQueueReceive(uart_rxq,&ch,1) == pdPASS ) {
    if( xQueueReceive(uart_rxq,&ch,500) == pdPASS ) {

      // should we continue consuming - if past buf size...
      // or return immediately without having received '\r' ?

      // screen only ever gives us a '\r'... i think and not a '\n'
      // don't return the \r in the return string...
      if(ch == '\r') {
        // need to decide - if should be returning/including the '\r' and '\n' or not?
        // *p++ = ch;
        *p = 0;
        return buf;
      }

      // OK. this line kills it... but why? because it blows the stack...
      if((size_t) (p - buf) < (len - 1))
        *p++ = ch;


    } else {
     taskYIELD();
    }
  }
  // should never arrive here...
}



#if 0
static void demo_task1(void *args __attribute__((unused))) {

  for (;;) {
    uart_puts("Now this is a message..\n\r");
    uart_puts("  sent via FreeRTOS queues.\n\n\r");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
#endif

/*
  prompt task should perhaps be moved outside of elsewhere...
*/


static char buf[100];

void usart_prompt_task(void *args __attribute__((unused))) {

  // buf size of 10 - seems ok
  // buf size of 50 - ok.
  // OK. buf size of 100. fails and stack exception condition caught - led blinks fast.
  // so we have a margin of somewhere between 50 - 100 bytes or so...
  // ok - at 70 - short input strings are ok - but will stack overflow on longer input strings..
  // char buf[70];

  for (;;) {
    // uart_printf is cooked ... so it should already be giving us stuff...
    uart_printf("\n\r> ");
    uart_gets( buf, 100 ); // ie. should block...
    uart_printf("\n\ryou said '%s'", buf );   // there looks like a bug in the formatting...
                                              // no it's just returning the \n but not the \r...
  }
}


