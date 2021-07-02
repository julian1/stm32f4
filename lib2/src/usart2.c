////////////////////////////////////////////////////////

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>

// #include <stddef.h> // size_t

#include "usart2.h"
#include "cbuffer.h"




static CBuf *output_buf = NULL;
static CBuf *input_buf = NULL;


// TODO change name usart1_setup_portB

void usart_setup_gpio_portB(void)
{
  // we moved usart 1 for stm32f410. to different pins,
  // PB6 = tx, PB7=rx
  // still AF7
  gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6 | GPIO7);
  gpio_set_af(GPIOB, GPIO_AF7, GPIO6 | GPIO7);
  //gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO7);
  gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO6);
}


// TODO change name usart1_setup_portA
// Actually maybe better to pass the arguments...

void usart_setup_gpio_portA(void)
{
  // stm32f407 usart1.
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9 | GPIO10);
  gpio_set_af(GPIOA, GPIO_AF7, GPIO9 | GPIO10);

  // TODO. REVIEW think this should be GPIO9...
  // gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO10);
  gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO9);
}



void usart_setup( CBuf *input, CBuf *output)
{
  // we have to setup the input_buf pointer for the isr...
  // althouth output is only needed in update()
  input_buf = input;
  output_buf = output;


  nvic_enable_irq(NVIC_USART1_IRQ);
  nvic_set_priority(NVIC_USART1_IRQ, 5);    // value???

#if 0
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9 | GPIO10);
  gpio_set_af(GPIOA, GPIO_AF7, GPIO9 | GPIO10);
  gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO10);
#endif



  usart_set_baudrate(USART1, 115200);
  usart_set_databits(USART1, 8);
  usart_set_stopbits(USART1, USART_STOPBITS_1);
  // usart_set_mode(USART1, USART_MODE_TX_RX);

  usart_set_mode(USART1, USART_MODE_TX_RX);
  usart_set_parity(USART1, USART_PARITY_NONE);
  usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);


  /* Enable USART1 Receive interrupt. */
  usart_enable_rx_interrupt(USART1);

  usart_enable(USART1);
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
    cBufPut(input_buf, ch);
  }


  /*
    eg.
    https://github.com/libopencm3/libopencm3-examples/blob/master/examples/stm32/f4/stm32f429i-discovery/usart_irq/usart_irq.c
    https://src.xengineering.eu/xengineering/stm32f103c8-examples/src/commit/a68a6b6a088cac53231e3910947d88e9167a3962/libraries/usart.c
  */

  /* Check if we were called because of TXE. */
  if (((USART_CR1(USART1) & USART_CR1_TXEIE) != 0) &&
      ((USART_SR(USART1) & USART_SR_TXE) != 0)) {

    if(cBufisEmpty(output_buf)) {
      // no more chars
      // disable transmit interupt
      usart_disable_tx_interrupt(USART1);
      return;
    }

    // else send next char
    int ch = cBufPop(output_buf);
    usart_send(USART1,ch);
  }


  return ;
}




void usart_output_update()
{
  if(!cBufisEmpty(output_buf)) {
    usart_enable_tx_interrupt(USART1);
  }
}




void usart_sync_flush()
{
  // useful when logging from a long/blocking init function.
  // avoid waiting until update() called, to configure the tx interupt handler, to restart transmit

  usart_output_update();
}





