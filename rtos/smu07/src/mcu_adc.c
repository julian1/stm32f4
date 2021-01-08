
#include "FreeRTOS.h"
#include "task.h"

#include <libopencm3/stm32/gpio.h>

#include <libopencm3/stm32/adc.h>



#include "sleep.h"
#include "serial.h"
#include "mcu_adc.h"

/////////////////////////////
// this code should be where?




void mcu_adc_setup(void)
{
  uart_printf("mcu_adc setup\n\r");

	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO0);
	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO1);

	adc_power_off(ADC1);
	adc_disable_scan_mode(ADC1);
	adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_3CYC); // is this enough for all bits?

	adc_power_on(ADC1);

  uart_printf("mcu_adc setup done\n\r");
}


uint16_t mcu_adc_read_native(uint8_t channel)
{
  // set up the arry of channels to read.
	uint8_t channel_array[16];
	channel_array[0] = channel;
	adc_set_regular_sequence(ADC1, 1, channel_array); // 1 indicates number of channels to read. eg. 1

  // start the read
	adc_start_conversion_regular(ADC1);
	while (!adc_eoc(ADC1));
	uint16_t reg16 = adc_read_regular(ADC1);
	return reg16;
}


void mcu_adc_print_task(void *args __attribute((unused)))
{

  static int tick = 0;

	for (;;) {

#if 1
    // So we need to wait until supplies come up when initializing.
    // which means factorizing this code.
    // but where do we put it...
    // actually - monitoring supplies should almost be a separate task...

    // Note that we should be able to talk to the dac / gpio - even if do not have
    // rails or ref up.

		uint16_t pa0 = mcu_adc_read_native(0);   // LP15VP
		uint16_t pa1 = mcu_adc_read_native(1);   // LN15VN
		// uint16_t pa2 = mcu_adc_read_native(2);   // dacmon - need to cut trace
                                        // should test it works though

		// uart_printf("tick: %d: LP15VP=%u, LN15VN=%d, pa2=%d\n", tick++, pa0, pa1, pa2 );
		uart_printf("tick: %d: LP15VP=%u, LN15VN=%d\n", tick++, pa0, pa1 );

#endif

#if 0
    // at gnd 0-2, at 3.3V supply get 4095.  eg. 4096 = 12bit. good. but maybe resolution is off.
    /*
      ADC channel numbers
      http://libopencm3.org/docs/latest/stm32f4/html/group__adc__channel.html
    */

    // don't think this works...
		uint16_t vref = mcu_adc_read_native( ADC_CHANNEL_VREF);
		uint16_t vbat = mcu_adc_read_native( ADC_CHANNEL_VBAT);
		uint16_t temp = mcu_adc_read_native( ADC_CHANNEL_TEMP_F40 );

    /*
      OK
      VREF seems quite stable...  // 700 to 704
      VBAT also 745 to 750
      so maybe its working...
      TEMP_F40 went 770 to 850 with heat gun... pointed at it. and back again.
    */
		uart_printf("tick: %d: pa0=%u vbat=%d  vref=%d temp=%d\n", tick++, pa0, vbat, vref, temp);
#endif

    task_sleep(1000);
  }
}


