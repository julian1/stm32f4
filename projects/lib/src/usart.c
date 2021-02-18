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
QueueHandle_t usart_txq = NULL;


// think we have to use an interrupt - so we don't miss anything. eg. we take the character
// in the isr, before it is replaced
QueueHandle_t usart_rxq = NULL;






void usart_setup(void)
{

  // not sure but queues, should be created in main()
  // setup() could take the queue as argument for better composibility
  // BUG.... QUEUES SHOULD BE CREATED FIRST!!!!!!!
  // OR BEFORE ENABLE
  usart_txq = xQueueCreate(256,sizeof(char));
  usart_rxq = xQueueCreate(256,sizeof(char));


  // this is sets up the rx interupt, but does not enable
  nvic_enable_irq(NVIC_USART1_IRQ); // JA
  nvic_set_priority(NVIC_USART1_IRQ,0xff );

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

// OK. enabling this
// we have to clear the damn flag.... interrupt flag...


  /* Finally enable the USART. */
  usart_enable(USART1);

#if 1
  /* Enable USART1 Receive interrupt. */
  usart_enable_rx_interrupt(USART1);
#endif

}


// simply enabling the interrupt hangs...
// so it returns from the func. but then the tx hangs? 


void usart1_isr(void)
{

  return ; 

  // Only thing this is doing - is echoing the output.
  // for console read I think we need blocking.
  static uint32_t data = 'A';   // THINK MUST be 32 not 8

  do {
    // consume
  }
  while ((data & USART_SR_RXNE) != 0);

  return ; 


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


