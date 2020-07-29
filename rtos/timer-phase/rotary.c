

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>

#include "rotary.h"

int rotary_setup(uint32_t tim, uint32_t portA, uint16_t pinA, uint8_t afA, uint32_t portB, uint16_t pinB, uint8_t afB) {
  /*
    https://gitlab.cs.fau.de/diy/wiki/-/wikis/Inkrementaldrehgeber

    ok, this works...
  */

  gpio_mode_setup(portA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, pinA);
  gpio_set_af(portA, afA, pinA);

  gpio_mode_setup(portB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, pinB);
  gpio_set_af(portB, afB, pinB);

  // timer_reset(tim);
  rcc_periph_reset_pulse(tim); //

  timer_set_mode(tim, TIM_CR1_CKD_CK_INT, //For dead time and filter sampling, not important for now.
   TIM_CR1_CMS_EDGE,
   TIM_CR1_DIR_UP);

  timer_set_prescaler(tim, 0);
  timer_set_repetition_counter(tim, 0);
  timer_enable_preload(tim);
  timer_continuous_mode(tim);

  timer_slave_set_mode(tim, TIM_SMCR_SMS_EM3); // encoder

  timer_set_oc_polarity_high(tim, TIM_OC1);
  timer_set_oc_polarity_high(tim, TIM_OC2);

  timer_ic_disable(tim, TIM_IC1);
  timer_ic_disable(tim, TIM_IC2);

  timer_ic_set_input(tim, TIM_IC1, TIM_IC_IN_TI1);
  timer_ic_set_input(tim, TIM_IC2, TIM_IC_IN_TI1);

  timer_disable_oc_output(tim, TIM_OC1);
  timer_disable_oc_output(tim, TIM_OC2);

  timer_disable_preload_complementry_enable_bits(tim);
  timer_set_period(tim, 0xFFFFFFFF);
  // timer_set_period(tim, 1 ); // this will force interrupts to be triggered.
                                // but is obviously not what we want

  timer_enable_counter(tim);

  return 0;
}




