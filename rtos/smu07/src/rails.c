
#include <libopencm3/stm32/gpio.h>


#include "usart.h"
#include "rails.h"

#define RAILS_PORT    GPIOE
// #define LN15V_CTL     GPIO13
// #define LP15V_CTL     GPIO14

#define LP15V_CTL   GPIO12
#define LN15V_CTL   GPIO13
#define LP30V_CTL   GPIO14
#define LN30V_CTL   GPIO15


// TODO should pass argument. eg. 1 for on. 0 for off.



void rails_setup( void )
{
  uart_printf("rails setup\n\r");

  const uint16_t all = LP15V_CTL | LN15V_CTL | LP30V_CTL |  LN30V_CTL ;

  // define, before configure
  gpio_clear(RAILS_PORT, all );

  gpio_mode_setup(RAILS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, all );

  // TODO Set speed. should be slow...

  uart_printf("rails setup done\n\r");
}



void rails_positive_on( void )
{
  uart_printf("rails positive on\n\r");
  gpio_set(RAILS_PORT, LP15V_CTL);
}

void rails_negative_on( void )
{
  uart_printf("rails negative on\n\r");
  gpio_set(RAILS_PORT, LN15V_CTL);
}


// there must be a better way to do this... than lots of messy ...
// expose the ports..?
// change name? rails_power_on?.

void rails_p30V_on( void )
{
  uart_printf("rails p30V on\n\r");
  gpio_set(RAILS_PORT, LP30V_CTL);
}





