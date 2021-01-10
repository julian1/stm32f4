

#include "FreeRTOS.h"
#include "task.h"

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>



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

  see exti_rising_falling.c

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


#define FALLING 0
#define RISING 1
static uint16_t exti_direction = FALLING;


void slope_adc_setup(void)
{
  usart_printf("slope_adc setup\n\r");

  gpio_mode_setup(ADC_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, ADC_OUT);



  // really not quite sure what EXTI15_10 means 15 or 10?
  // see code example, https://sourceforge.net/p/libopencm3/mailman/message/28510519/
  // defn, libopencm3/include/libopencm3/stm32/f4/nvic.h

  // nvic_enable_irq(NVIC_EXTI0_IRQ);
  nvic_enable_irq(NVIC_EXTI15_10_IRQ);

  /* Configure the EXTI subsystem. */
  exti_select_source(EXTI15, GPIOD);
  exti_set_trigger(EXTI15, EXTI_TRIGGER_FALLING);
  exti_enable_request(EXTI15);


  exti_direction = FALLING;

  usart_printf("slope_adc setup done\n\r");
}



static int interupt_hit = 0;

void exti15_10_isr(void)
// void exti0_isr(void)
{
  exti_reset_request(EXTI15);

  // this might be getting other interupts also... not sure.

  // see, https://sourceforge.net/p/libopencm3/mailman/libopencm3-devel/thread/CAJ%3DSVavkRD3UwzptrAGG%2B-4DXexwncp_hOqqmFXhAXgEWjc8cw%40mail.gmail.com/#msg28508251
  // uint16_t port = gpio_port_read(GPIOE);
  // if(port & GPIO1) {
  // uint16_t EXTI_PR_ = EXTI_PR;
  // if(EXTI_PR_ & GPIO15) {

  // No. Think we do not have to filter,
  // see, exti15_10_isr example here,
  // https://github.com/geomatsi/stm32-tests/blob/master/boards/stm32f4-nucleo/apps/freertos-demo/button.c


  usart_putc_from_isr('a');  

    interupt_hit = 1;

  if (exti_direction == FALLING) {
    // gpio_set(GPIOE, GPIO0);
    exti_direction = RISING;
    exti_set_trigger(EXTI15, EXTI_TRIGGER_RISING);
  } else {
    // gpio_clear(GPIOE, GPIO0);
    exti_direction = FALLING;
    exti_set_trigger(EXTI15, EXTI_TRIGGER_FALLING);
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

    usart_printf("slope_adc hi tick %d %d\n\r", tick++, gpio_get(ADC_PORT, ADC_OUT));
    task_sleep(1000); // 1Hz
	}
}
