/* Simple LED task demo, using timed delays:
 *
 * The LED on PC13 is toggled in task1.


  see, for
  ~/devel/stm32/FreeRTOSv10.3.1/FreeRTOS/Demo/CORTEX_M4F_STM32F407ZG-SK/FreeRTOSConfig.h

  doc,
  https://www.freertos.org/FreeRTOS-for-STM32F4xx-Cortex-M4F-IAR.html

  so, i think 'proper' usart will use dma.

 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>

#include <stdarg.h>
#include <stdio.h>

// using the ww library...
//#include "uartlib.h"
#include "miniprintf.h"



static void uart_puts(const char *s) ;

extern void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName);


void vApplicationStackOverflowHook(
  xTaskHandle *pxTask __attribute((unused)),
  signed portCHAR *pcTaskName __attribute((unused))
) {
  int i;

	// for(;;);	// Loop forever here..

    for(;;) { 
      gpio_toggle(GPIOE, GPIO0);  // JA

      for (i = 0; i < 1000000; i++) { 
        __asm__("nop");
      }
    }

}



static void task1(void *args __attribute((unused))) {

	for (;;) {
		gpio_toggle(GPIOE,GPIO0);
		vTaskDelay(pdMS_TO_TICKS(500)); // 1Hz
	}
}






// TODO - consider if should be global, global struct, or returned from usart_setup()
static QueueHandle_t uart_txq;        // TX queue for UART


// think we have to use an interrupt - so we don't miss anything. eg. we take the character
// in the interuupt.
static QueueHandle_t uart_rxq;        // TX queue for UART



static void usart_setup(void)
{
  // this is sets up the rx interupt, but does not enable
  nvic_enable_irq(NVIC_USART1_IRQ); // JA

  // TODO - use  GPIO9 | GPIO10
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



extern void usart1_isr(void)
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

    xQueueSend(uart_txq, &data,portMAX_DELAY);  // send to tx queue to echo back to console.
                                                // probably don't want to do this here. but somewhere else for more
                                                // control

    // Toggle LED to show signs of life
    gpio_toggle(GPIOE,GPIO0);
  }

}


#if 0
static void uart_task(void *args) {
  int gc;
  char kbuf[256], ch;

  (void)args;

  uart_puts("\n\ruart_task() has begun:\n\r");

  // puts_uart(1,"\n\ruart_task() has begun:\n\r");

  for (;;) {
    if ( (gc = getc_uart_nb(1)) != -1 ) {
      puts_uart(1,"\r\n\nENTER INPUT: ");

      ch = (char)gc;
      if ( ch != '\r' && ch != '\n' ) {
        /* Already received first character */
        kbuf[0] = ch;
        putc_uart(1,ch);
        getline_uart(1,kbuf+1,sizeof kbuf-1);
      } else  {
        /* Read the entire line */
        getline_uart(1,kbuf,sizeof kbuf);
      }

      puts_uart(1,"\r\nReceived input '");
      puts_uart(1,kbuf);
      puts_uart(1,"'\n\r\nResuming prints...\n\r");
    }

    /* Receive char to be TX */
    if ( xQueueReceive(uart_txq,&ch,10) == pdPASS )
      putc_uart(1,ch);
    /* Toggle LED to show signs of life */
    // gpio_toggle(GPIOC,GPIO13);
    gpio_toggle(GPIOE,GPIO0); // JA
  }
}

static void demo_task(void *args) {

  (void)args;

  for (;;) {
    uart_puts("Now this is a message..\n\r");
    uart_puts("  sent via FreeRTOS queues.\n\n\r");
    vTaskDelay(pdMS_TO_TICKS(1000));
    uart_puts("Just start typing to enter a line, or..\n\r"
      "hit Enter first, then enter your input.\n\n\r");
    vTaskDelay(pdMS_TO_TICKS(1500));
  }
}

#endif


// see the getline function... just needs to have get() implemented...
// should be possible to implement gets using just get.
// and we just need the producer consumer setup. again. I think...
// use same non blocking approach...
// eg.
// most of the time - that we want to receive - will be at a prompt.
// so we really only need to do it once. and we can perhaps directly block...
// so dont need the queue mechanism.
// is this blocking or non blocking...
// int16_t usart_recv(uint32_t usart);
// Actually not sure - we want blocking... for character. So we don't miss a character.

// So just use - an interrupt. and then push value onto the queue...
// if the queue blocks its ok...
// for getline ... we clear queue - then just keep processing the receive queue until we hit a \r\n

/*
333   if ( !uptr )
334     return -1;  // No known uart
335   while ( (rch = get_char(uptr)) == -1 )
336     taskYIELD();
337   return (char)rch;
*/



static void
uart_task(void *args __attribute__((unused))) {
  char ch;

  for (;;) {
    // Receive char to be TX
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


static inline void uart_puts(const char *s) {

  for ( ; *s; ++s )
    xQueueSend(uart_txq,s,portMAX_DELAY); /* blocks when queue is full */
}


/*
static int mcu_printf1(const char *format,...) {
  va_list ap;
  int rc;
  char buf[ 20] ; // has to be on the stack... if pre-emtive multitasking.

  // va args really don't seem to want to work...
   // vsnprintf(char *str, size_t size, const char *format, va_list ap);
  va_start(ap,format);
  rc = vsnprintf(buf, 20 , format, ap); // calling this... kills us...
  va_end(ap);
  //rc = snprintf(buf, 100, format); 

  // so its the va args? or somthing else?

  uart_puts(buf );


  return rc;
}
*/


static void uart_putc(char ch) {
    //usart_send_blocking(USART1,ch); // libopencm3

    xQueueSend(uart_txq, &ch ,portMAX_DELAY); /* blocks when queue is full */
}


static int uart_printf(const char *format,...) {
  va_list args;
  int rc;

  va_start(args,format);
  rc = mini_vprintf_cooked(uart_putc,format,args);
  va_end(args);
  return rc;
}



static void
demo_task(void *args __attribute__((unused))) {

  // sprintf...
  int i = 0;

  for (;;) {

    // mcu_printf("hi there %d\n\r", 123 ); 
    // mcu_printf1("hi there %d\n\r", 123 ); 
    uart_printf("hi there %d\n\r", i++); 

    uart_puts("Now this is a message..\n\r");
    uart_puts("  sent via FreeRTOS queues.\n\n\r");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}



int main(void) {

	// rcc_clock_setup_in_hse_8mhz_out_72mhz(); // For "blue pill"
  rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

  // LED
	// rcc_periph_clock_enable(RCC_GPIOC);
	// gpio_set_mode( GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
  rcc_periph_clock_enable(RCC_GPIOE); // JA
  gpio_mode_setup(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO0); // JA - move to function led_setup.

  // USART
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_USART1);


  ///////////////
  usart_setup();

  ///////////////

	xTaskCreate(task1,"LED",100,NULL,configMAX_PRIORITIES-1,NULL);

  xTaskCreate(uart_task,"UART",200,NULL,configMAX_PRIORITIES-1,NULL); /* Highest priority */
  xTaskCreate(demo_task,"DEMO",100,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */



	vTaskStartScheduler();

	for (;;);
	return 0;
}

// End
