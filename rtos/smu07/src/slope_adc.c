

#include "FreeRTOS.h"
#include "task.h"

#include <libopencm3/stm32/gpio.h>


//////////////////////////////////////////


#include "sleep.h"
#include "usart.h"
#include "slope_adc.h"


#define ADC_PORT              GPIOD

// TODO, fix should prefix all thse with MUX_ in the schematic.
// maybe differentiate from bootstrap mux. also.
#define ADC_OUT               GPIO0
#define ADC_REFP10V_CTL       GPIO1
#define ADC_REFN10V_CTL       GPIO2
#define ADC_IN_CTL            GPIO3
#define ADC_RESET_CTL         GPIO4
#define ADC_MUX_VFB_CTL       GPIO5
#define ADC_MUX_IFB_CTL       GPIO6
#define ADC_MUX_DAC_VMON_CTL  GPIO7
#define ADC_MUX_AGND_CTL      GPIO8



static  uint32_t all_ctl =
    ADC_REFP10V_CTL | ADC_REFN10V_CTL | ADC_IN_CTL | ADC_RESET_CTL
      | ADC_MUX_VFB_CTL | ADC_MUX_IFB_CTL | ADC_MUX_DAC_VMON_CTL | ADC_MUX_AGND_CTL;


void slope_adc_setup(void)
{
/*
  // active low.
  uint32_t all_ctl =
    ADC_REFP10V_CTL | ADC_REFN10V_CTL | ADC_IN_CTL | ADC_RESET_CTL
      | ADC_MUX_VFB_CTL | ADC_MUX_IFB_CTL | ADC_MUX_DAC_VMON_CTL | ADC_MUX_AGND_CTL;
*/

  uart_printf("slope_adc setup\n\r");

  gpio_set(ADC_PORT, all_ctl);

  gpio_clear(ADC_PORT, ADC_RESET_CTL);    // turn on - to short cap, and hold integrator at agnd.
                                          // it's the diode clamp that appears to oscillate ...
  // small amount of hysteriss could be useful - but don't think is the problem - unless its coupling?

  // ok all of these inputs will oscillate... with the cap shorted/reset by switch...
  // so there's no point - adding a smaller offset...
  gpio_clear(ADC_PORT, ADC_IN_CTL | ADC_MUX_AGND_CTL);   // put agnd on the input
  // gpio_clear(ADC_PORT, ADC_REFP10V_CTL);  // put 10V on the input
  // gpio_clear(ADC_PORT, ADC_REFN10V_CTL);  // put -10V on the input

  gpio_mode_setup(ADC_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, all_ctl);
  gpio_mode_setup(ADC_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, ADC_OUT);

  uart_printf("slope_adc setup done\n\r");
}



static void slope_adc_out_print(void)
{
  static uint32_t tick = 0;

  uart_printf("slope_adc hi tick %d %d\n\r",
      tick++,
      gpio_get(ADC_PORT, ADC_OUT)
    );
}


static void slope_adc_test(void)
{
  // uart_printf("adc test disable\n\r");
  // return;
  uart_printf("slope_adc test\n\r");

  // dac_write_register(0x01, (1 << 12) );  // select monitor dac0  // works
  // dac_write_register(0x01, (1 << 11) );     // ain, which is wired to vref65  // works
  // gpio_clear(ADC_PORT, ADC_MUX_DAC_VMON_CTL);

  // integrates up
  gpio_set(ADC_PORT, ADC_IN_CTL | ADC_MUX_AGND_CTL);      // turn off agnd in
  gpio_clear(ADC_PORT, ADC_REFN10V_CTL);                  // turn on N10V ref
  gpio_set(ADC_PORT, ADC_RESET_CTL);                    // start integrating
  task_sleep(3);

  // integrate down
  gpio_set(ADC_PORT, ADC_REFN10V_CTL);                  // turn off N10V ref
  gpio_clear(ADC_PORT, ADC_REFP10V_CTL);                  // turn on N10V ref
  task_sleep(3);

  // reset
  gpio_set(ADC_PORT, ADC_REFP10V_CTL);                  // turn off N10V ref
  gpio_clear(ADC_PORT, ADC_IN_CTL | ADC_MUX_AGND_CTL);  // turn on agnd in
  gpio_clear(ADC_PORT, ADC_RESET_CTL);                    // short the cap, to stop integrating

  uart_printf("slope_adc test done\n\r");
}



// IFB looks ok. goes in the correct direction, and looks nice and linear.

static void slope_adc_clear(void)
{
  gpio_set(ADC_PORT, all_ctl);
  gpio_clear(ADC_PORT, ADC_RESET_CTL);                    // short cap.  stop integrating
  gpio_clear(ADC_PORT, ADC_IN_CTL | ADC_MUX_AGND_CTL);   // put agnd on the input
}


static void slope_adc_ifb_test(void)
{
  uart_printf("slope_adc test\n\r");

  slope_adc_clear();
  task_sleep(3);

#if 1
  // integrate up
  gpio_set(ADC_PORT, ADC_IN_CTL | ADC_MUX_AGND_CTL);    // turn off agnd in
  gpio_clear(ADC_PORT, ADC_IN_CTL | ADC_MUX_IFB_CTL);    // turn on IFB ref
  gpio_set(ADC_PORT, ADC_RESET_CTL);                    // clear the short of the cap - start integrating
  task_sleep(5);

  // integrate down
  gpio_set(ADC_PORT, ADC_IN_CTL | ADC_MUX_IFB_CTL);    // turn off IFB ref
  gpio_clear(ADC_PORT, ADC_REFN10V_CTL);                  // turn on P10V ref
  task_sleep(3);

  // reset
  slope_adc_clear();
#endif
  uart_printf("slope_adc test done\n\r");
}




// VFB looks ok. it's hard to trigger on.
// but its a bit curved rather than linear - because not buffered.


static void slope_adc_vfb_test(void)
{
  uart_printf("slope_adc test\n\r");

  // VFB is 2.5V on the input. no problem. but not buffered...
  // gpio_set(ADC_PORT, ADC_IN_CTL | ADC_MUX_AGND_CTL);    // turn off agnd in
  // gpio_clear(ADC_PORT, ADC_IN_CTL | ADC_MUX_VFB_CTL);    // turn on VFB ref

  // IFB is 0.3VDC - no problem.
  // gpio_set(ADC_PORT, ADC_IN_CTL | ADC_MUX_AGND_CTL);    // turn off agnd in
  // gpio_clear(ADC_PORT, ADC_IN_CTL | ADC_MUX_IFB_CTL);    // turn on VFB ref

  slope_adc_clear();
  task_sleep(3);

  // integrate down
  gpio_set(ADC_PORT, ADC_IN_CTL | ADC_MUX_AGND_CTL);    // turn off agnd in
  gpio_clear(ADC_PORT, ADC_IN_CTL | ADC_MUX_VFB_CTL);    // turn on VFB ref
  gpio_set(ADC_PORT, ADC_RESET_CTL);                    // clear the short of the cap - start integrating
  task_sleep(3);

  // integrate up
  gpio_set(ADC_PORT, ADC_IN_CTL | ADC_MUX_VFB_CTL);    // turn off VFB ref
  gpio_clear(ADC_PORT, ADC_REFN10V_CTL);                  // turn on P10V ref
  task_sleep(3);

  // reset
  slope_adc_clear();

  uart_printf("slope_adc test done\n\r");
}




