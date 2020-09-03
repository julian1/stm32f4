/*
  change name to supplies?
*/

#include <libopencm3/stm32/gpio.h>


#include "usart.h"
#include "utility.h"
#include "rails.h"


// rails...  can we do it in order...
#define RAILS_PORT    GPIOE
// #define RAILS_POS     GPIO8   // pull high to turn on.  I think we fucked this mcu pin with overvoltage...

#define RAILS_NEG     GPIO9   // pull low to turn on
#define RAILS_POS     GPIO10

#define RAILS_VREF    GPIO11


// TODO change to separate functions - as DAC expects negative rail on first...
// or move this function entirely





void rails_positive_on( void )
{
  gpio_set  (RAILS_PORT, RAILS_POS);  // pull up.
}

void rails_negative_on( void )
{
  gpio_clear(RAILS_PORT, RAILS_NEG);  // pull down.
}

void rails_vref_on( void )
{
  gpio_clear(RAILS_PORT, RAILS_VREF);  // pull down.

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
  gpio_set  (RAILS_PORT, RAILS_VREF);   // pull-down off

  gpio_mode_setup(RAILS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE ,  /* broken GPIO8 */ RAILS_POS | RAILS_NEG | RAILS_VREF  );

}
