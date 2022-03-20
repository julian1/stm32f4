/*
  need to rename usart2.c to usart.c
*/

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>


#include "usart.h"
#include "cbuffer.h"
#include "util.h"   // critical_error_blink()



static CBuf *coutput = NULL;
static CBuf *cinput  = NULL;


// TODO change name usart1_setup_portB

void usart1_setup_gpio_portB(void)
{
  // we moved usart 1 for stm32f410. to different pins,
  // PB6 = tx, PB7=rx
  // still AF7
  gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6 | GPIO7);
  gpio_set_af(GPIOB, GPIO_AF7, GPIO6 | GPIO7);
  gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO6);
}


// TODO change name usart1_setup_portA
// Actually maybe better to pass the arguments...

void usart1_setup_gpio_portA(void)
{
  // stm32f407 usart1.
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9 | GPIO10);
  gpio_set_af(GPIOA, GPIO_AF7, GPIO9 | GPIO10);
  gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO9);
}



/*
  feb 2022.
  - we should be passing the usart argument explicitly. like we do with spi.
  - the actual usart configu called setup() or init() oconfigure()...
  - separate the buffer from the actual configuration.
  - but cbuffers means can only actually deal with one usart here.
*/



static void usart_configure( uint32_t usart )
{

  switch(usart) {
    case USART1:
      nvic_enable_irq(NVIC_USART1_IRQ);
      nvic_set_priority(NVIC_USART1_IRQ, 5);    // value???
      break;

    default:
      // assert is problematic here... because no usart configured to debug.
      // assert(0);
      critical_error_blink();
  }


  usart_set_baudrate(usart, 115200);
  usart_set_databits(usart, 8);
  usart_set_stopbits(usart, USART_STOPBITS_1);

  usart_set_mode(usart, USART_MODE_TX_RX);
  usart_set_parity(usart, USART_PARITY_NONE);
  usart_set_flow_control(usart, USART_FLOWCONTROL_NONE);


  /* Enable usart Receive interrupt. */
  usart_enable_rx_interrupt(usart);

  usart_enable(usart);
}




void usart1_set_buffers( CBuf *input, CBuf *output)
{
  // TODO change name  usart1_set_buffers
  // set buffers before configure and interupt enable.
  cinput = input;
  coutput = output;

  usart_configure( USART1);
}


void usart1_isr(void)
{

  /* Check if we were called because of RXNE. */
  if (((USART_CR1(USART1) & USART_CR1_RXNEIE) != 0) &&
      ((USART_SR(USART1) & USART_SR_RXNE) != 0)) {

    /* Retrieve the data from the peripheral.
      and clear flags.
     */

    // write the input buffer
    char ch = usart_recv(USART1);
    cBufPush(cinput, ch);
  }


  /*
    eg.
    https://github.com/libopencm3/libopencm3-examples/blob/master/examples/stm32/f4/stm32f429i-discovery/usart_irq/usart_irq.c
    https://src.xengineering.eu/xengineering/stm32f103c8-examples/src/commit/a68a6b6a088cac53231e3910947d88e9167a3962/libraries/usart.c
  */

  /* Check if we were called because of TXE. */
  if (((USART_CR1(USART1) & USART_CR1_TXEIE) != 0) &&
      ((USART_SR(USART1) & USART_SR_TXE) != 0)) {

    if(cBufisEmpty(coutput)) {
      // no more chars
      // disable transmit interupt
      usart_disable_tx_interrupt(USART1);
      return;
    }

    // else send next char
    int ch = cBufPop(coutput);
    usart_send(USART1,ch);
  }


  return ;
}


void usart1_enable_output_interupt()
{
  /*
    note. we check in the interupt handler if more data in output buffer.
    and explicitly de-enable interupt there. so don't do it here.
  */

  // data in buf, then ensure that txe interupt is enabled to process
  if(!cBufisEmpty(coutput)) {
    usart_enable_tx_interrupt(USART1);
  }
}



void usart1_flush()
{
  // block until flushed

  usart1_enable_output_interupt();

  while(!cBufisEmpty(coutput));
}






