

#include "FreeRTOS.h"
#include "task.h"

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>

#include <libopencm3/stm32/rcc.h>   // remove...
#include <libopencm3/stm32/timer.h>


// #include <math.h>
// #include <stdlib.h>

//////////////////////////////////////////


#include "led.h"
#include "sleep.h"
#include "serial.h"
#include "slope_adc.h"


#define ADC_PORT              GPIOD   // rename ADC_OUT_PORT... albeit this is just standard gpio.
#define ADC_OUT               GPIO0


#define ADC_MUX_PORT    GPIOD
#define ADC_MUX_P_CTL   GPIO1
#define ADC_MUX_N_CTL   GPIO2   // disconnected.
#define ADC_IN_CTL      GPIO3
#define ADC_RESET_CTL   GPIO4   // unused. jumper not fitted.




#define MCU_GPIO_PORT   GPIOB
#define MCU_GPIO1       GPIO10
#define MCU_GPIO2       GPIO11




#define FALLING 0
#define RISING 1
static uint16_t exti_direction = FALLING;

static uint32_t period = 0;



void slope_adc_setup(void)
{
  usart_printf("slope_adc setup\n\r");


  //////////
  //////////
  // crossing detect interupt 
  // must be high(est) priority, as timer counts are read in interrupt. 
  // rcc_periph_clock_enable(RCC_SYSCFG);  for interrupts. once in main.c

  // set up input and crossing interupt
  gpio_mode_setup(ADC_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, ADC_OUT);

  // use GPIO D0 so need EXTI0
  nvic_enable_irq(NVIC_EXTI0_IRQ);

  // 0 appears to be highest priority
  // set timer interupts to run at the highest priority
  nvic_set_priority(NVIC_EXTI0_IRQ,0); // priority

  /* Configure the EXTI subsystem. */
  exti_select_source(EXTI0, GPIOD);
  exti_set_trigger(EXTI0, EXTI_TRIGGER_BOTH  /*EXTI_TRIGGER_FALLING */ );
  exti_enable_request(EXTI0);

  exti_direction = FALLING;



  /////////////////////////////////
  /*
    countdown timer.
    because for updatable countdown periods simpler.
    since don't have to subtract desired time from the period of a count-up timer.
    stm32f4, want tim2 or tim5, for 32 bit timers
  */
  rcc_periph_clock_enable(RCC_TIM3);
  nvic_enable_irq(NVIC_TIM3_IRQ);         /* Enable TIM3 interrupt. */

  rcc_periph_reset_pulse(RST_TIM3);     // reset
  timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_DOWN);
  timer_enable_break_main_output(TIM3);
  timer_set_prescaler(TIM3, 10 );            // 0 is twice as fast as 1.
  period = 10000;
  timer_set_counter(TIM3, period );

  timer_enable_irq(TIM3, TIM_DIER_UIE);   // counter update
  timer_enable_counter(TIM3);               // start timer, IMPROTANT should be done last!!!.... or will miss.


  ////////////////////////////////
  // accumulation timers. 32 bit.
  // count up
  rcc_periph_clock_enable(RCC_TIM2);
  rcc_periph_reset_pulse(RST_TIM2);         // reset
  timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
  timer_enable_break_main_output(TIM2);
  timer_set_prescaler(TIM2, 0 );            // no prescale. 0 is twice as fast as 1.

  // move this
  timer_enable_counter(TIM2);

  
  rcc_periph_clock_enable(RCC_TIM5);
  rcc_periph_reset_pulse(RST_TIM5);         // reset
  timer_set_mode(TIM5, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
  timer_enable_break_main_output(TIM5);
  timer_set_prescaler(TIM5, 0 );            // no prescale. 0 is twice as fast as 1.

  // move this
  timer_enable_counter(TIM5);




  /////////////////////////////
  /////////////////////////////
  // ports to mux voltages to integrator

  //
  // u16
  const uint16_t all = ADC_MUX_P_CTL | ADC_MUX_N_CTL | ADC_IN_CTL | ADC_RESET_CTL;

  gpio_clear(ADC_MUX_PORT, all);   // off for adg333 spdt
  gpio_mode_setup(ADC_MUX_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, all);

  // injecct +10V ref, which will integrate output to the negative rail
  gpio_set(ADC_MUX_PORT, ADC_MUX_P_CTL);

  usart_printf("slope_adc done timer done\n\r");



  //////////////////////

  // run

  // injecct +10V ref, which will integrate output to the negative rail
  gpio_set(ADC_MUX_PORT, ADC_MUX_P_CTL);

  // -10V ref is injected by timer. pushes output up.
}

/*
  https://stackoverflow.com/questions/19412780/stand-alone-portable-snprintf-independent-of-the-standard-library

  portable snprintf.

  const char * ftos(float x, char *buf, size_t n)
  { 
    int intpart, fracpart;
    intpart = (int)x;
    fracpart = (int)((x- intpart) * 10000000);  // controls the digits

    // handle negatives
    if(fracpart <0 )
      fracpart *= -1;

    // snprintf(buf, 11, "%4d.%04d", intpart, fracpart);
    snprintf(buf, n, "%d.%d", intpart, fracpart);
    return buf;
  }


  int main()
  { 
    char buf [ 1000];

    printf("%s\n",  ftos( 99.12345678, buf, 1000 ));
  }

  TODO
    1. set the interrupt priority - even though shouldn't matter (was does freertos use).
      crossing interupt *does* have to be precise.
    2. move miniprintf to ./lib.  and delink the ww library.
*/


void exti0_isr(void)
{
  // crossing interrupt.   ie. agnd comparator.

  // uint32_t count = timer_get_counter(TIM3); // do as first thing
  // count -= 21;                              // approx time for interupt and call to get value

  exti_reset_request(EXTI0);

  if (exti_direction == FALLING) {
    // slope direction rising.

    // set count to zero for both timers for start of integration... 
  
    ////////////////////////////
    // full cycle
    // get counter values, from last cycle
    int32_t x = timer_get_counter(TIM2);
    int32_t y = timer_get_counter(TIM5);

    // clear counter values for next cycle
    timer_set_counter(TIM2, 0);
    timer_set_counter(TIM5, 0);
  
    // compute 
    // use atos fton
    float result = ((float)x) / y ; 
    usart_printf("full cycle %d\n", (int32_t) (result * 1000000));

    // static char buf[100];
    // gcvt(result, 5, buf);

    exti_direction = RISING;
    exti_set_trigger(EXTI0, EXTI_TRIGGER_RISING);

  } else {
    // slope direction falling

    /*
    // get timer count...
    int x = timer_get_counter(TIM2);
    int y = timer_get_counter(TIM5);

    float result = ((float)x) / y ; 
    usart_printf("done %d\n", (int32_t) (result * 1000000));
    */


    exti_direction = FALLING;
    exti_set_trigger(EXTI0, EXTI_TRIGGER_FALLING);
  }

  // set run-down timer to run again.
  timer_set_counter(TIM3, period );
}

/*
  note that on first zero-cross we have to start the timer
  EXTREME
  NO. we just set both counters to zero... and whether it is counting of not is correct.

  likewise for zero cross at end.
*/

void tim3_isr(void)
{
  // timer interrupt, we've hit a apex or bottom of integration

  if (timer_get_flag(TIM3, TIM_SR_UIF)) {

    timer_clear_flag(TIM3, TIM_SR_UIF);

    // branch timing with if statement is unequal - but doesn't matter

    if(exti_direction) {

      // these are the two actions that should be run adjacent in time..
      timer_disable_counter(TIM5);
      gpio_clear(ADC_MUX_PORT, ADC_MUX_N_CTL);

      // usart_printf("reached top tim2 %u  tim5 %u \n", timer_get_counter(TIM2), timer_get_counter(TIM5) );
      // period is set in zero cross.
    }
    else {

      timer_enable_counter(TIM5);
      gpio_set(ADC_MUX_PORT, ADC_MUX_N_CTL); // start injecting negative current to rise

      // usart_printf("reached bottom\n" );
    }
  }
}





/*
  /////////////////////////////
  /////////////////////////////

  // port for test signals for scope

  gpio_mode_setup(MCU_GPIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, MCU_GPIO1 | MCU_GPIO2);
  gpio_set_output_options(MCU_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, MCU_GPIO1 | MCU_GPIO2);

  gpio_set(MCU_GPIO_PORT, MCU_GPIO1);
  // gpio_clear(MCU_GPIO_PORT, MCU_GPIO1);
  // speed...
*/


#if 0
  /////////////////////////////
  /////////////////////////////
  // GPIOA PA15 - led out...  need to resolder old connections back
  gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_OUT);
  gpio_set_output_options(LED_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, LED_OUT);
  gpio_set(LED_PORT, LED_OUT);

#endif



////////////////////////////


// stop/start a 32 bit counter. would be easy way to accumulate injected voltages...

// OK wait... our up/down timer precision is not critical.   So use a non-32bit timer with prescalar.
// that leaves us with two 32 bit timers - for the counting of injected voltages / references.
// by just starting/stopping with enable and disable - would make very easy.

    // usart_printf("  c fall\n");
    // timer_set_counter(TIM2, period );
    // timer_enable_counter(TIM2);



    // usart_printf("  c rise\n");
    // timer_set_counter(TIM2, period );
    // timer_enable_counter(TIM2);



  // uint32_t count = timer_get_counter(TIM2);
  // count -= 21; // approx time for interupt and call to get value

  // we use count, to updated the injected/integrated voltage/current.
  // injected_voltage += count;

  /*
      have another timer counter - that just keeps aggregating the total injected current.
      rather than doing += values.
      - not sure. think we will need a reference count anyway.

      OR. keep the counter running below 0...  and then we can use the value from the other side
  */



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

/*

  timer_generate_event()
    Force generate a timer event.

    The UG event is useful to cause shadow registers to be preloaded before the
    timer is started to avoid uncertainties in the first cycle in case an update
    event may never be generated.


  TIM_EGR_UG   (1 << 0)
    Update generation.
  --------

  simple as,

   timer_generate_event(TIM3, TIM_EGR_UG);
   timer_enable_counter(TIM3);

    may need to set value...

    https://github.com/jsphuebner/stm32-test/blob/master/stm32_test.c


  eg.
  void pwm_start(void)
  {
   timer_generate_event(TIM4, TIM_EGR_UG);
    timer_enable_counter(TIM4);
  }
    https://github.com/marcorussi/stm32f4_demo/blob/master/pwm.c
  --------

  EXTREME.
  Or it doesn't require anything at all.
  eg. we are hittinig the period / to generate our update.

    so we just change th counter value - in the update - tfor our new period.

    interupt on update.
      so set the counter for the next period.

  EXTREME.
  count down timer. from a starting position might be cleaner.
    eg. in the upate. we set the countdown point.


*/

/*
  There are two separate concerns.
    - 1. centering. trying to keep integration from bouncing up and down - or hitting the rails.
    - 2. getting reading - which is the total injected voltage / crossings span.

    1. centering approach by moving oc. might actually be wrong. because it can be at any height
    with the same oc value. but oc can be used to, change timing to shift it
      hence why it stays at bottom for ages, then moves very quicky.
      - we might also need fractional...

    1. centering is not a transfer function. instead. oc_value needs has to
      increase for small period to centre . then restore to previous.  so any kind of
      error should not be accumulative.
      - but then oc_value is used as the injection current - and to balance ...

    might be easier to just name vars  xr. cross rising count?
                                      oc_value.

    2. we don't actually need to compute injected current for a single cycle.
      although it would be nice
    -----------

    maybe should try to program it - so that rising intercept occurs at the timer period .

  ---------
    could always short circuit the integrator - to centre. eg. every second cycle. perhaps.
  --------
    think we need to try integrating an arbitrary voltage. and test.
      or - just set starting oc - at some varying point.
  --------
  EXTREME
    the portionate approach at the end (count -oc, remain - oc). might be easier - as can almost exactly
      calculute (rather than integrate error). what the correct/new oc value should be.

      (think we really need to go back to this).
  ---------------------
  EXTREME
    OK. actually think we might need three intercept crossings - to calculate. while holding oc_value constant.

*/


/*
  it ought to be possible to calculate the desired value exactly...

  // this rising falling is no good at all. it's just a toggle.
  // TODO - this state variable is terrible - we should be able to discern direction from the mcu flags
  //


*/

// IMPORTANT.
// see function, timer_set_oc_fast_mode()


// OK. the issue is that it doesn't start the integrationn ... at 0...
// which is our assumption.
// that's why it overshoots and bounces around.



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

#if 0
static float signed_square( float x )
{
  float h = x * x;
  if(x < 0) h *= -1;
  return h;
}


static uint32_t crise = 0;

static uint32_t mylog2 (uint32_t val)
{
  if (val == 0) return UINT32_MAX;
  if (val == 1) return 0;
  uint32_t ret = 0;
  while (val > 1) {
      val >>= 1;
      ret++;
  }
  return ret;
}



static int32_t signed_mylog2 (int32_t val)
{
  if(val > 0)
    return mylog2( val);
  else
    return -mylog2(-val);

}

#endif

// i think we need some kind of expression for height. only then can optimize / integrade/ feedback for it.
// don't try to centre the integration around 0V - instead when it hits zero - we're finished.
// or do 'enhanced dual slope', need the agnd on a cou
// don't have channel for reset
// actually we wouldn't necessarily use a timer. or we kind of could.
// the timer sequences stuff.  but we calculate everything on the interrupt of the cross.
// it just needs three more switches - much more complicated (agnd, reset, suspension of input).

// can do dual slope - just with interrupts - and get_timer . would be easier with timer control - for agnd/reset..

//////////////////
// or have extra comparators (or ADC) at +10V and -10V - so can always just bounce it.
// eg. when gets to value - we get interupt - so reverse... can set oc value in interrupt also - if want.


// ADC with interrupts. and in the interrupt - we can just set the oc for a small time in the future .
// ------ issue of loading of the output - ???


// gahh. it shouldn't be so hard. - we know roughly where the voltage is - due to our zero crossing.

// or do we need a slope compensation type thing... that applies the oc voltage early.


// maybe we just need to keep the thing at less than 50% cycle ????



/*
EXTREME.
  OK. get rid of fixed time.

  after up crossing - continue running up for a bit, and fixed interval start tundown.
  after down crossing - same.

  That should be centred. And does not rely on any other comparators to bounce the signal.

  It may make sense to use a timer to get our reverse point.
  just do reset... if possible.
  only one channel to do both.

  set the period to really long. so will never reach.
  then just use the OC value each time.

  use preload. to start counting from number other than 0.

*/
