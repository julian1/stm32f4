


#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>


#include <ice40-extra.h>

/*
  - should move all simple gpio, to separate port c perhaps.
  - maybe want interupt for cdone.  to detect power cycle. no can do it in a poll
  ---------------


  OR.

  we wrap the functionality - to avoid exposing.

  eg.

  ice40_port_extra_creset_set(  1 );
  ice40_port_extra_cdone_get(  1 );
  ice40_port_extra_unused1_set(  1 );

  likewise for the led,  should hide the implementation detail,

  led_set( 1 );

  Yes. this is better.
  It is still OK, to pass a descriptor.

  Issue. for led blink, is passing out the details for the critical error blink functionality.
  BUT. but nothing else needs to know about it.

  so should have local led.c  file.

*/

/*
  no these should *not* be in the header.
    want to localize.
    and then expose accessors.

*/

// jan 2024.
// wants an external pullup fitted. because can be drivern by connector header.
#define ER_CRESET_PORT  GPIOB
#define ER_CRESET_PIN   GPIO5   // PB5

#define ER_CDONE_PORT   GPIOB
#define ER_CDONE        GPIO4   // PB4


#define ER_TRIGGER_INT_OUT GPIO4   // PA3
#define ER_UNUSED1_OUT    GPIO10   // PA10





void ice40_port_extra_creset_enable(void)
{
  /* IMPORTANT - eg. follow same interface as spi_assert(),
    enable means normal function.   eg. output enable
  */

  // active lo.
  gpio_set(ER_CRESET_PORT, ER_CRESET_PIN);
}


void ice40_port_extra_creset_disable(void)
{
  // active lo.
  gpio_clear(ER_CRESET_PORT, ER_CRESET_PIN);
}


bool ice40_port_extra_cdone_get(void)
{
   return gpio_get(ER_CDONE_PORT, ER_CDONE)  != 0;

}



void ice40_port_extra_setup(void)
{
  // rcc_periph_clock_enable(RCC_GPIOA);
  // rcc_periph_clock_enable(RCC_GPIOB);


  // set high, as initial value
  // set first, before initialize, to reduce glitch

  // actually should probably hold in reset - at startup.
  // gpio_set(ER_CRESET_PORT, ER_CRESET_PIN);


  gpio_mode_setup(ER_CRESET_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, ER_CRESET_PIN);

  // perhaps add an interupt.
  // actually we can just poll in main loop. for power cycle issues.
  gpio_mode_setup(ER_CDONE_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, ER_CDONE);


  gpio_set_output_options(ER_CRESET_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, ER_CRESET_PIN);
  // cdone - has no input ;  //

  /////////////////////

  // should define to avoid floating adum inputs.
  gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, ER_TRIGGER_INT_OUT);
  gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, ER_UNUSED1_OUT);


  gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, ER_TRIGGER_INT_OUT);
  gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, ER_UNUSED1_OUT);


}


