/*
  ref
*/

#include <libopencm3/stm32/gpio.h>

#include "serial.h"
#include "ref.h"


#define REF_PORT    GPIOB

#define DAC_MUX_REFA_CTL  GPIO8
#define DAC_MUX_REFB_CTL  GPIO9


/*
  remember dg444 active low.
  So *MUST* set state/config... before we poweron rails...
*/

void ref_setup( void )
{
  // call *before* bringing up rails
  // WE SHOULD DO ALL PINS here???
  usart_printf("ref setup\n\r");

  gpio_set(REF_PORT, DAC_MUX_REFA_CTL | DAC_MUX_REFB_CTL);  // hi == off for dg444, as init condition.
  gpio_mode_setup(REF_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, DAC_MUX_REFA_CTL | DAC_MUX_REFB_CTL);

  usart_printf("ref setup done\n\r");
}



void refa_off( void )
{
  gpio_set(REF_PORT, DAC_MUX_REFA_CTL);
  usart_printf("refa off\n\r");
}



// TODO pass an argument. eg. 1 for on. 0 for off.  ref_ctrl(bool)

void refa_on( void )
{
  gpio_clear(REF_PORT, DAC_MUX_REFA_CTL);
  usart_printf("refa on\n\r");
}


