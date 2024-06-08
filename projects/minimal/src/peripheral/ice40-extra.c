
/*
  could be prefixed gpio_ice40_port

*/


#include <libopencm3/stm32/gpio.h>


#include <peripheral/ice40-extra.h>


/*
  none of this should be exposed in header files.
  want to localize.
  and then have peripheral specific accessors.

*/


/*
// jan 2024.
// wants an external pullup fitted. because can be drivern by connector header.
#define ER_EXTRA_PORT  GPIOB
#define ER_CRESET_PIN   GPIO5   // PB5

#define ER_EXTRA_PORT   GPIOB
#define ER_CDONE        GPIO4   // PB4
*/



/*
// Feb 2024.
// current on mcu side was 70mA. with wrong port assignment.

#define ER_EXTRA_PORT    GPIOC
#define ER_CDONE        GPIO6   // PC6
#define ER_CRESET_PIN   GPIO7   // PC7

#define ER_TRIGGER_INT_OUT GPIO8   // PC8
#define ER_UNUSED1_OUT    GPIO9   // PC9
*/


// Feb 2024.
#define ER_EXTRA_PORT    GPIOC


#define ER_TRIGGER_SA_LOCAL GPIO7

#define ER_CDONE        GPIO8
#define ER_CRESET_PIN   GPIO9
#define ER_UNUSED1_OUT    GPIO11
#define ER_UNUSED2_OUT    GPIO12
#define ER_ER_UNUSED3_CTL GPIO10




void ice40_port_extra_creset_enable(void)
{
  /* IMPORTANT - eg. follow same interface as spi_assert(),
    enable means normal function.   eg. output enable
  */

  // active lo.
  gpio_set(ER_EXTRA_PORT, ER_CRESET_PIN);
}


void ice40_port_extra_creset_disable(void)
{
  // active lo.
  gpio_clear(ER_EXTRA_PORT, ER_CRESET_PIN);
}


bool ice40_port_extra_cdone_get(void)
{
   return gpio_get(ER_EXTRA_PORT, ER_CDONE)  != 0;

}





void ice40_port_trigger_source_internal_enable(void)
{
  // better name
  gpio_set(ER_EXTRA_PORT, ER_TRIGGER_SA_LOCAL);
}


void ice40_port_trigger_source_internal_disable(void)
{
  // better name
  gpio_clear(ER_EXTRA_PORT, ER_TRIGGER_SA_LOCAL);
}








void ice40_port_extra_setup(void)
{
  // rcc_periph_clock_enable(RCC_GPIOA);
  // rcc_periph_clock_enable(RCC_GPIOB);
  // rcc_periph_clock_enable(RCC_GPIOC);


  // set high, as initial value
  // set first, before initialize, to reduce glitch

  // actually should probably hold in reset - at startup.
  // gpio_set(ER_EXTRA_PORT, ER_CRESET_PIN);

  // perhaps add an interupt.
  // actually we can just poll in main loop. for power cycle issues.
  gpio_mode_setup(ER_EXTRA_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, ER_CDONE);


  gpio_mode_setup(ER_EXTRA_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, ER_CRESET_PIN);
  gpio_set_output_options(ER_EXTRA_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, ER_CRESET_PIN);
  // cdone - has no input ;  //

  /////////////////////

  gpio_mode_setup(ER_EXTRA_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, ER_TRIGGER_SA_LOCAL);
  gpio_set_output_options(ER_EXTRA_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, ER_TRIGGER_SA_LOCAL);


  // define to avoid floating inputs on adum

  // unused pins.
  gpio_mode_setup(ER_EXTRA_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, ER_UNUSED1_OUT | ER_UNUSED2_OUT);
  gpio_set_output_options(ER_EXTRA_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, ER_UNUSED1_OUT | ER_UNUSED2_OUT);
}


