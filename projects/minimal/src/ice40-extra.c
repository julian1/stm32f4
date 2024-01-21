


#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>


#include <ice40-extra.h>

/*
  - should move all simple gpio, to separate port c perhaps.
  - maybe want interupt for cdone.  to detect power cycle. no can do it in a poll
*/

// these need to be in the header. if we are going to toggle, read them them.

// jan 2024.
// wants an external pullup fitted. because can be drivern by connector header.
#define ER_CRESET_PORT  GPIOB
#define ER_CRESET_PIN   GPIO5   // PB5

#define ER_CDONE_PORT   GPIOB
#define ER_CDONE        GPIO4   // PB4


#define ER_TRIGGER_INT_OUT GPIO4   // PA3
#define ER_UNUSED1_OUT    GPIO10   // PA10


// freaking messy.



void ice40_port_extra_setup(void)
{
  // rcc_periph_clock_enable(RCC_GPIOA);
  // rcc_periph_clock_enable(RCC_GPIOB);


  // set high, as initial value
  // set first, before initialize, to reduce glitch
  gpio_set(ER_CRESET_PORT, ER_CRESET_PIN);


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


