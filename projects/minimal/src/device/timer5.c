
/*
  apr. 2026.
  old timer code
  keep for the moment
  used to drive dc/dc.

*/



#include <stdio.h>    // printf, scanf
#include <assert.h>
#include <string.h>



#include <libopencm3/stm32/rcc.h>   // mcu clock initialization
#include <libopencm3/stm32/gpio.h>    // required to initialize critical_error_led blink.  context.
#include <libopencm3/stm32/timer.h>



// this all looks ok. to me.
// is something else trying

static void timer_port_setup(void )
{
  assert(0);

  printf("timer port setup\n");

  // PA0.   TIM5/CH1


  //  port set up for alt function.
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO0);
  gpio_set_af(GPIOA, GPIO_AF2, GPIO0 );
  gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO0);



/*
  // ok. this code works. to set, pin value as gpio.
  gpio_mode_setup( GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO0);
  gpio_set_output_options( GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO0);
  gpio_write_val(GPIOA, GPIO0 , 0 );

*/
}




static void timer_setup( uint32_t timer )
{
  // timer counter is peripheral.

  // rcc_periph_clock_enable(RCC_TIM5);

  printf("timer setup\n");

  // uint32_t timer = TIM5;
  assert( timer == TIM5 );

  if(timer == TIM5)
    rcc_periph_reset_pulse( RST_TIM5 );   // is this needed?
  else
    assert(0);

  timer_set_prescaler(timer, 0 );  // No prescaler = 42Mhz.

  timer_set_mode(timer, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_CENTER_1, TIM_CR1_DIR_UP);  // alternating up/down

  // timer_enable_counter(timer);
}



static void timer_set_frequency( uint32_t timer, uint32_t freq /*, uint32_t deadtime */ )
{
  /* output channel is device.
    - but cannot share easily,
      although oc channel only needs halfperiod, the timer itselfs needs period.
      so cannot hang another simple square wave off the same timer.
      - so should probably treat everything as a device.
  */

  // assert(deadtime >= 1 /*&& deadtime <= 50 */);
  // assert(freq >= 40000 && freq <= 500000);
  // assert(freq >= 10000 && freq <= 500000);
  assert(freq >= 10000 && freq <= 100000);
  assert( timer == TIM5 );

  timer_disable_counter(timer);

  // uint32_t freq = 200 * 1000;               // in Hz.
  // double clk_period = 2 / 84000000.f ;

  uint32_t period = (84000000.f / freq) / 2; // calculated.
  uint32_t half_period = period / 2;

  printf("------\n");
  printf("freq          %.1f kHz\n", freq / 1000.f );
  printf("clk period    %lu\n", period );

  // timer_enable_break_main_output(timer);
  timer_set_period(timer, period );    // 42kHz

  // 1
  timer_enable_oc_output(timer, TIM_OC1 );
  timer_set_oc_mode(timer, TIM_OC1 , TIM_OCM_PWM1);    // Output is active (high) when counter is less than output compare value
  timer_set_oc_value(timer, TIM_OC1, half_period );


  timer_set_counter( timer, 0 );    // make sure timer count does  not escape when shortening period

  timer_enable_counter(timer);  // seems to need this

}


