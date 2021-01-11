

#include "FreeRTOS.h"
#include "task.h"

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>

#include <libopencm3/stm32/rcc.h>   // remove...
#include <libopencm3/stm32/timer.h>


//////////////////////////////////////////


#include "sleep.h"
#include "serial.h"
#include "slope_adc.h"


#define ADC_PORT              GPIOD   // rename ADC_OUT_PORT... albeit this is just standard gpio.
#define ADC_OUT               GPIO0

// these need to be on a timer.
// there is a timer port.
// on port d, need pd12 to pd15. gahhh.

// pb4 and pb5 are tim3o

// pb10 gpio1. tim2 channel 3.
// gahhh. annoying not channel 1.

// tim1 ch 1  pa8.   unused - but we didn't pull the pin out.
// tim1 ch 1. pe9     mux_inject_agnd_ctl.   OK. unused.   But would have to change existing code...
// tim3 ch 1  pc6    irange sw.

/*
  OK. I think we screwed up the adc. by not putting the P and N ref on a timer port. and with inverse.
  - in fact we almost certainly wanted a hardwhere inverse
    so we could just blink/alternate the refs.
  - but we can manually do it.
  - and we can probably appropriate io somewhere. and even add an inverter ic/fet common drain.
  - this is a big complicated.
  ---------

  see exti_rising_falling.c

  NO. NO.
    we just want to blip the corrective ref voltage. not add the voltage of the same direction.
    it's basically just a led on a timer.
    it is more two timers - for each direction - so can configure but enable/disable.
    or else

  should use nor gate - to construct not. perhaps.

  note our pwm example - where we respond on the interupt - because we change the led in the interrupt.

  mux_ifb_inv_ctl pe5   tim9 ch1.  <- can use easily.
  lets try to get interrupt working - done.

  now we want to blink a led - on output we can use...
  actually multimeter would do.

*/

#define ADC_MUX_PORT    GPIOD
#define ADC_MUX_P_CTL   GPIO1
#define ADC_MUX_N_CTL   GPIO2   // disconnected.
#define ADC_IN_CTL      GPIO3
#define ADC_RESET_CTL   GPIO4   // unused. jumper not fitted.


#define FALLING 0
#define RISING 1
static uint16_t exti_direction = FALLING;

static uint32_t period = 0;
static uint32_t oc_value = 0;

void slope_adc_setup(void)
{
  usart_printf("slope_adc setup\n\r");

  // rcc_periph_clock_enable(RCC_SYSCFG);  for interrupts. once in main.c

  // set up input and crossing interupt
  gpio_mode_setup(ADC_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, ADC_OUT);

  // use GPIO D0 so need EXTI0
  nvic_enable_irq(NVIC_EXTI0_IRQ);

  /* Configure the EXTI subsystem. */
  exti_select_source(EXTI0, GPIOD);
  exti_set_trigger(EXTI0, EXTI_TRIGGER_BOTH  /*EXTI_TRIGGER_FALLING */ );
  exti_enable_request(EXTI0);

  exti_direction = FALLING;

  usart_printf("slope_adc setup done\n\r");



  /////////////////////////////
  /////////////////////////////
  // stm32f4, need tim2 or tim5, for 32 bit timers

  rcc_periph_clock_enable(RCC_TIM2);

    /* Enable TIM2 interrupt. */
  nvic_enable_irq(NVIC_TIM2_IRQ);

  // pa0.
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO15 );
  gpio_set_af(GPIOA, GPIO_AF1, GPIO15 ); // PA0 AF1 == TIM2-CH1-ETR, PA0 AF2 == tim5-ch1 .
                                        // TIM5-CH1 / PA0 works.
                                        // TIM2-CH1-ETR/PA0 also *does* work.
                                        // TIM2-CH1 / PA15 also *does* work.
  gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, GPIO15); // 50is faster than 100? no. same speed


  rcc_periph_reset_pulse(RST_TIM2);     // reset

  timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

  // timer_set_repetition_counter(TIM2, 0);
  // timer_enable_break_main_output(TIM2);

  timer_set_prescaler(TIM2, 0 );      // 0 is twice as fast as 1.
  timer_disable_preload(TIM2);        // must be disable_preload()... else counter ignores period, and counts through to 32bits, 4billion
  timer_continuous_mode(TIM2);

  period = 1000000;
  timer_set_period(TIM2, period); // ok working

  timer_disable_oc_output(TIM2, TIM_OC1);
  timer_set_oc_mode(TIM2, TIM_OC1, TIM_OCM_PWM1);

  oc_value = 500000;
  timer_set_oc_value(TIM2, TIM_OC1, oc_value);   // eg. half the period for 50% duty
  timer_enable_oc_output(TIM2, TIM_OC1);


  timer_enable_irq(TIM2, TIM_DIER_CC1IE | TIM_DIER_UIE /*| TIM_DIER_TIE */ );   // counter update and capture compare
  timer_enable_counter(TIM2);

  /////////////////////////////
  /////////////////////////////
  // ports to mux voltages to integrator


  // u16
  const uint16_t all = ADC_MUX_P_CTL  | ADC_MUX_N_CTL | ADC_IN_CTL | ADC_RESET_CTL;

  gpio_clear(ADC_MUX_PORT, all);   // off for adg333 spdt
  gpio_mode_setup(ADC_MUX_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, all);

  // injecct +10V ref, which will integrate output to the negative rail
  gpio_set(ADC_MUX_PORT, ADC_MUX_P_CTL);

  usart_printf("slope_adc done timer done\n\r");

}

// we dont even need to switch input. because its constantly being integrated.
// so only actually require 2x of the 4x dpdt switches

// OR - the integrating period for the corrective voltage - should be long
// enough - that will always pass agng (to get interrupt). when first start?


// think we need an interupt - so if at the end of a cycle, we got no crossing interupt .
// we can extend the count in a direction.

/*
  OK. so using a DC sig-gen and manipulating voltage
  will generate several interupts.
  but think we can discriminate in software.
*/


void exti0_isr(void)
{
  // the crossing interrupt.

  uint32_t count = timer_get_counter(TIM2); // do as first thing

  exti_reset_request(EXTI0);



  // this rising falling is no good at all. it's just a toggle.

  // TODO - this state variable is terrible - we should be able to get the direction from the mcu flags

  if (exti_direction == FALLING) {

    // think this is rising.
    // usart_putc_from_isr('xu');

    usart_printf("x up\n");

    // gpio_set(GPIOE, GPIO0);
    exti_direction = RISING;
    exti_set_trigger(EXTI0, EXTI_TRIGGER_RISING);

  } else {

    usart_printf("x dwn\n");
    // usart_putc_from_isr('d');

    // integrating up... I think.

    // ahhh shouldn't call this from interupt. but it seems to work...
    // 265596

    int32_t diff   = count - oc_value;            // count should be greater than oc_value?
    int32_t remain  = period - count;

    // oc_value %u,
    usart_printf("  count %u\n\r", count);
    usart_printf("  diff %d  remain %d\n", diff, remain );
    usart_printf("  oc_value %u\n", oc_value  );

/*


  reason - not getting count as first thing. also probably need to adjust.
            

  its overshooting back and forward. regardless

  ----
  toggle a gpio pin.  on the crossing interupt. and hook up to a scope.
    to confirm we are not getting multiple interrupts on a cross.
    - is there another way to do this?
    - yes an interupt on new cycle.

    -----
    we can do print statements instead of a scope - if have the oc/period pwm interupt.


  also hook up to a scope - the injection ctl.
 crystal

  extreme could be a problem with interupt being hit multiple times.
  resulting in multiple error values adjusting. not stabilizing.

  or lagging because of print.

  or quantitization error. need to using a doulbe for oc_value

  oc_value is changing in huge amounts.... why?
  eg. adjut is 1. but value change 100?
*/

#if 0
    // this works.
    if(diff < remain) {
      oc_value += 50;
    } else {
      oc_value -= 50;
    }
#endif
    // oc_value += (remain - diff) * 0.1;
    /// oc_value += ((remain / (float)diff) - 1.0) * 100;

#if 1
    // appears to kind of work
    float err = (remain - diff) * 0.005;

    usart_printf("err %d\n\r", (int32_t)err);

    if(err > 50) err = 50;
    if(err < -50) err = -50;

    // usart_printf("clamped err %d\n\r", err);

    oc_value += err  ;   // it's weird that it oscillates/ and overshoots.
#endif


    timer_set_oc_value(TIM2, TIM_OC1, oc_value);


    /*
      it ought to be possible to calculate the desired value exactly...
    */

    // see also,  timer_set_oc_fast_mode()


    // gpio_clear(GPIOE, GPIO0);
    exti_direction = FALLING;
    exti_set_trigger(EXTI0, EXTI_TRIGGER_FALLING);
  }
}






void tim2_isr(void)
{
  // interupts for beginning of period (update), and output compare
  // EXTREME ---- if cannot get it someother way - then get the interupt by setting oc to 1 on second channel
  // SR=status register, IF= interupt flag.

  uint32_t count = timer_get_counter(TIM2);

  if (timer_get_flag(TIM2, TIM_SR_UIF)) {
    // ok. this seems to be the thing can catch the count at 20
    timer_clear_flag(TIM2, TIM_SR_UIF);

    usart_printf("-----\n");
    usart_printf("uif\n");
    usart_printf("  count %u\n", count );
  }



  if (timer_get_flag(TIM2, TIM_SR_CC1IF)) {
    /* Clear compare interrupt flag. */
    timer_clear_flag(TIM2, TIM_SR_CC1IF);   // TIM_DIER_CC1IE  ??   TIM_SR_CC1IF
    usart_printf("cc1if\n\r");
    usart_printf("  count %u\n", count );
  }

}


#if 0
void slope_adc_out_status_test_task(void *args __attribute((unused)))
{
  int tick = 0;
	for (;;) {

    if(interupt_hit) {
      usart_printf("slope_adc interupt\n\r");
      interupt_hit = 0;
    }

    // usart_printf("count %u\n\r", timer_get_counter(TIM5));

    usart_printf("slope_adc hi tick %d %d\n\r", tick++, gpio_get(ADC_PORT, ADC_OUT));
    task_sleep(1000); // 1Hz
	}
}
#endif





////////////////////////////

// this might be getting other interupts also... not sure.

// IMPORTANT how to discriminate ports if using high.
  // see, https://sourceforge.net/p/libopencm3/mailman/libopencm3-devel/thread/CAJ%3DSVavkRD3UwzptrAGG%2B-4DXexwncp_hOqqmFXhAXgEWjc8cw%40mail.gmail.com/#msg28508251
  // uint16_t port = gpio_port_read(GPIOE);
  // if(port & GPIO1) {
  // uint16_t EXTI_PR_ = EXTI_PR;
  // if(EXTI_PR_ & GPIO15) {

  // No. Think we do not have to filter,
  // see, exti15_10_isr example here,
  // https://github.com/geomatsi/stm32-tests/blob/master/boards/stm32f4-nucleo/apps/freertos-demo/button.c


  // really not quite sure what EXTI15_10 means 15 or 10?
  // see code example, https://sourceforge.net/p/libopencm3/mailman/message/28510519/
  // defn, libopencm3/include/libopencm3/stm32/f4/nvic.h


  /////////////////////////

  // etr is external trigger.  don't think its what we want..

  //rcc_periph_clock_enable(RCC_GPIOA);
  // rcc_periph_clock_enable(RCC_TIM1);

  // summary of timers, http://stm32f4-discovery.net/2014/05/stm32f4-stm32f429-discovery-pwm-tutorial/
  // OK. hang on. I think we want 32bit. not 16bit. resolution. with prescale of one. that ticks over pretty fast.
  // that means timer2 or timer5
  // timer_set_oc_value (uint32_t timer_peripheral, enum tim_oc_id oc_id, uint32_t value)   is 32 bit value.

  /////////////////////////////////////////
  // tim2-ch1  pa15   af1.
  // my god. so we can use the led status.
  // see this on pa15 confusion,
  // https://community.st.com/s/question/0D50X00009XkZNI/where-is-tim2ch1-on-the-stm32f4-chips
  // says has to have jtag disabled,
  // https://community.st.com/s/question/0D50X00009XkbxeSAB/can-tim2ch1-not-etr-be-remapped-to-pa15-or-anywhere
  // this has code and pretends to set PA15 but doesnt, just ignores.
  // http://www.micromouseonline.com/2013/02/16/quadrature-encoders-with-the-stm32f4/

  /////////////////////////////////////////

  // tim5 ch1 is pa0.   lp15v_fb. uggh.

/*
  maybe a conflict with swd/jtag output.
  eg. works for gpio. but not AF1.
*/



#if 0
void exti0_isr(void)
{
  exti_reset_request(EXTI0);

  // this works. quite nice
  // need to work out which is which...

  interupt_hit = 1;

  // this rising falling is no good at all. it's just a toggle.

  if (exti_direction == FALLING) {

    // think this is rising.
    usart_putc_from_isr('u');

    // ahhh shouldn't call this from interupt. but it seems to work...
    // 265596

    uint32_t count = timer_get_counter(TIM2);
    int32_t diff   = count - oc_value;    // count should be greater than oc_value?

    usart_printf("count %u diff %d\n\r", count, diff );

    timer_set_oc_value(TIM2, TIM_OC1, oc_value);   // eg. half the period for 50% duty

    // see also,  timer_set_oc_fast_mode()

    // ahhh is there an issue t
    // timer_set_oc_value(TIM2, TIM_OC1, 500000);
    // timer_get_oc_value(TIM2, TIM_OC1);

    // gpio_set(GPIOE, GPIO0);
    exti_direction = RISING;
    exti_set_trigger(EXTI0, EXTI_TRIGGER_RISING);
  } else {

    usart_putc_from_isr('d');
    // gpio_clear(GPIOE, GPIO0);
    exti_direction = FALLING;
    exti_set_trigger(EXTI0, EXTI_TRIGGER_FALLING);
  }
}

#endif

