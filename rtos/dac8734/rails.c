
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
// #include <libopencm3/stm32/timer.h>

// #include <libopencm3/stm32/spi.h>



#include "usart.h"
#include "utility.h"
#include "rails.h"


// rails...  can we do it in order...
#define RAILS_PORT    GPIOE
// #define RAILS_POS     GPIO8   // pull high to turn on.  I think we fucked this port...

#define RAILS_NEG     GPIO9   // pull low to turn on
#define RAILS_POS     GPIO10



void rails_turn_on( void )
{

  uart_printf("turn rails on \n\r");
  gpio_clear(RAILS_PORT, RAILS_NEG);
  msleep(50);
  gpio_set  (RAILS_PORT, RAILS_POS);
  uart_printf("rails on \n\r");
  msleep( 50);

}


void rails_setup( void )
{

  uart_printf("rails setup\n\r");

  // ok. define before enabling...
  // if we do this after setup - then the neg rail, needs high, will glitch on reset.
  // turn off

  uart_printf("rails off \n\r");
  gpio_clear(RAILS_PORT, RAILS_POS);
  gpio_set  (RAILS_PORT, RAILS_NEG);

  gpio_mode_setup(RAILS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE /*GPIO_PUPD_PULLDOWN */, RAILS_POS  );
  gpio_mode_setup(RAILS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE /*GPIO_PUPD_PULLUP*/,   RAILS_NEG );
  gpio_mode_setup(RAILS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO8 ); // broken.. gpio.

}
