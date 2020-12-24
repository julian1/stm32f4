
#include <libopencm3/stm32/gpio.h>


#include "usart.h"
#include "rails.h"

// Fixme. we swapped these for rev03.
#define RAILS_PORT    GPIOE
// #define RAILS_NEG     GPIO13
// #define RAILS_POS     GPIO14

#define RAILS_POS   GPIO12
#define RAILS_NEG   GPIO13




// TODO should pass argument. eg. 1 for on. 0 for off.



void rails_setup( void )
{
  uart_printf("rails setup\n\r");

  // define, before configure
  gpio_clear(RAILS_PORT, RAILS_POS);
  gpio_clear(RAILS_PORT, RAILS_NEG);

  gpio_mode_setup(RAILS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, RAILS_POS | RAILS_NEG);

  uart_printf("rails setup done\n\r");
}



void rails_positive_on( void )
{
  uart_printf("rails positive on\n\r");
  gpio_set(RAILS_PORT, RAILS_POS);
}

void rails_negative_on( void )
{
  uart_printf("rails negative on\n\r");
  gpio_set(RAILS_PORT, RAILS_NEG);
}


