

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


#define ADC_PORT              GPIOD
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

#define ADC_MUX_P_CTL         GPIO1
#define ADC_MUX_N_CTL         GPIO2

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



#define FALLING 0
#define RISING 1
static uint16_t exti_direction = FALLING;


void slope_adc_setup(void)
{
  usart_printf("slope_adc setup\n\r");

  // rcc_periph_clock_enable(RCC_SYSCFG);  for interrupts. once in main.c

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
  // perhaps the non ETR eg. PA15 doesn't work?

  rcc_periph_clock_enable(RCC_TIM2);

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
  timer_disable_preload(TIM2);        // must be disable_preload... else counter ignores period, and counts to 32bits, 4billion
  timer_continuous_mode(TIM2);
  timer_set_period(TIM2, 1000000); // ok working

  timer_disable_oc_output(TIM2, TIM_OC1);
  timer_set_oc_mode(TIM2, TIM_OC1, TIM_OCM_PWM1);
  timer_set_oc_value(TIM2, TIM_OC1, 500000);   // eg. half the period for 50% duty
  timer_enable_oc_output(TIM2, TIM_OC1);

  timer_enable_counter(TIM2);

  usart_printf("slope_adc done timer done\n\r");

}



static int interupt_hit = 0;

// void exti15_10_isr(void)
void exti0_isr(void)
{
  exti_reset_request(EXTI0);

  // this works. quite nice
  usart_putc_from_isr('a');

  interupt_hit = 1;

  if (exti_direction == FALLING) {
    // gpio_set(GPIOE, GPIO0);
    exti_direction = RISING;
    exti_set_trigger(EXTI0, EXTI_TRIGGER_RISING);
  } else {
    // gpio_clear(GPIOE, GPIO0);
    exti_direction = FALLING;
    exti_set_trigger(EXTI0, EXTI_TRIGGER_FALLING);
  }
}








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

