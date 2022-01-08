

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









#if 0

// Seems impossible to do interrupts. because interrupts occur on stuff like
  output compare and wrap around. while we are just counting.
  we cannot just update the output compare value to use in the isr - because
  we don't know the direction eg. +1, or -1.

static QueueHandle_t rotary_txq;

static void rotary_setup_interupt(void)
{
  rotary_txq = xQueueCreate(256,sizeof(char));
  // timer_continuous_mode( TIM3);

  // Ahhh interupt - cannot be set
  nvic_enable_irq(NVIC_TIM3_IRQ);

  // timer_enable_irq(TIM3, TIM_DIER_CC1IE);
  // timer_enable_irq(TIM3, TIM_DIER_CC1IE | TIM_DIER_CC2IE | TIM_DIER_CC3IE | TIM_DIER_CC4IE);
  timer_enable_irq(TIM3, TIM_DIER_UIE );  // this also gets an occasional value...

  // TIM_DIER_UIE
}

/*
  OK. hang on.
    if we toggle back and forth over the same values - then it's always emitting something.
  tim3 interrupt 65535
  tim3 interrupt 0
  tim3 interrupt 0
  tim3 interrupt 65535
  tim3 interrupt 65535
  tim3 interrupt 0
  tim3 interrupt 0
  tim3 interrupt 65535
  tim3 interrupt 65535
  tim3 interrupt 0
  tim3 interrupt 0

  So the issue is that there are no interupt events for the normal process of counting.
  If we set the,  timer_set_period(tim, 1 );
  then we always get the events.

  tim3 interrupt 0
  tim3 interrupt 1
  tim3 interrupt 1
  tim3 interrupt 0
  tim3 interrupt 0
  tim3 interrupt 1
  tim3 interrupt 1

  So the easy way to do it, is just loop and test the last value, and if it changes emit an
  event. becomes a bit more ugly, but will work fine. if we really need an event.
*/

void tim3_isr(void)
{
  // timer_clear_flag(TIM3, TIM_DIER_UIE );  // not clearing the interrupt will freeze it.

  gpio_toggle(GPIOE,GPIO0);
  uart_printf("tim3 interrupt %d\n\r", timer_get_counter( TIM3 ));
  timer_clear_flag(TIM3, TIM_DIER_UIE);
}

static void rotary_task(void *args __attribute__((unused))) {
  char ch;

  for (;;) {
    if ( xQueueReceive(rotary_txq,&ch,500) == pdPASS ) {
      uart_printf("x");
      // uart_printf("x\n\r");
    }
    //taskYIELD();
  }
}


#endif

