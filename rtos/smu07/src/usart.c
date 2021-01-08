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

#include "usart.h"





///////////////////////////


// TODO - consider if should be global, global struct, or returned from usart_setup()
QueueHandle_t usart_txq;


// think we have to use an interrupt - so we don't miss anything. eg. we take the character
// in the isr, before it is replaced
QueueHandle_t usart_rxq;



/*
  This function should be moved to main(). it is *not* usart..
  We may just want to blink

*/


// this shouldn't be here either... probably...

extern void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName);


void vApplicationStackOverflowHook(
  xTaskHandle *pxTask __attribute((unused)),
  signed portCHAR *pcTaskName __attribute((unused))
) {
  int i, j = 0, len;
    // ok seems to be catching some stack overflow conditions
    // this won't work - because relies on sending to queues.
    // we need to bit-bang the usart.
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
  usart_txq = xQueueCreate(256,sizeof(char));
  usart_rxq = xQueueCreate(256,sizeof(char));

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

  //  SHOULD be
    // xQueueSendFromISR not xQueueSend 
    xQueueSendFromISR(usart_rxq, &data , NULL ); /* blocks when queue is full */
                                                // if piping input to usart - then blocking is probably desirable,

    xQueueSendFromISR(usart_txq, &data, NULL );  /* blocks */
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


#if 0
 BaseType_t xQueueSendFromISR
           (
               QueueHandle_t xQueue,
               const void *pvItemToQueue,
               BaseType_t *pxHigherPriorityTaskWoken
           );
#endif

void usart_task(void *args __attribute__((unused))) 
{

  // waits for chars on the serial queue, then push them out on the hardware usart.

  char ch;

  for (;;) {
    /*
      500 - is the time in ticks to wait. 1/2 second is too long?
    */
    if ( xQueueReceive(usart_txq,&ch,500) == pdPASS ) {
      while(!usart_get_flag(USART1,USART_SR_TXE))
        taskYIELD();  // Yield until ready
                      // JA - doesn't seem to use coroutines...
      usart_send(USART1,ch);
    }
    // Toggle LED to show signs of life
    // gpio_toggle(GPIOE,GPIO0);
  }
}


