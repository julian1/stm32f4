

#include "FreeRTOS.h"
#include "task.h"

#include <libopencm3/stm32/gpio.h>


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

// pb10 gpio1. we could use. tim2 channel 3.
// gahhh. that's annoying.

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

  NO. NO. 
    we just want to blip the corrective ref voltage. not add the voltage of the same direction. 
    it's basically just a led on a timer. 
    it is more two timers - for each direction - so can configure but enable/disable.
    or else 

  should use nor gate - to construct not. perhaps.

  note our pwm example - where we respond on the interupt - because we change the led in the interrupt.

  mux_ifb_inv_ctl pe5   tim9 ch1.  <- can use easily.


  lets try to get interrupt working.
  
*/

// OK. first lets just report the status

void slope_adc_setup(void)
{
  usart_printf("slope_adc setup\n\r");

  gpio_mode_setup(ADC_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, ADC_OUT);





  exti_set_trigger(EXTI16, EXTI_TRIGGER_RISING);      // other code uses rising...
  // exti_set_trigger(EXTI16, EXTI_TRIGGER_FALLING);   // think we want falling.
                                                    // pwr_voltage_high() eg. goes from high to lo.
  exti_enable_request(EXTI16);

  // defined 1 for line 16.
  // #define NVIC_PVD_IRQ 1
  nvic_enable_irq( NVIC_PVD_IRQ );



  usart_printf("slope_adc setup done\n\r");




}



void slope_adc_out_status_test_task(void *args __attribute((unused)))
{
  int tick = 0;
	for (;;) {

    usart_printf("slope_adc hi tick %d %d\n\r", tick++, gpio_get(ADC_PORT, ADC_OUT));

    task_sleep(1000); // 1Hz
	}
}
