
#include <stdio.h>
#include <assert.h>


#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/adc.h>

/*

  libopencm3-examples/examples/stm32//f0/stm32f0-discovery/adc/adc.c
  libopencm3-examples/examples/stm32/f4/cjmcu-407/adc-dac-printf/adc-dac-printf.c

../libopencm3/include/libopencm3/stm32/f4/adc.h:#define ADC_CHANNEL_TEMP_F40    16
../libopencm3/include/libopencm3/stm32/f4/adc.h:#define ADC_CHANNEL_TEMP_F42    18


  see converting to actual C.
  https://electronics.stackexchange.com/questions/324321/reading-internal-temperature-sensor-stm32


test temp
val 959
test temp

cold power on
val 942
convert 29.000000

warm .
test temp
val 964  convert 36.23C



*/

#include <lib2/stats.h>        // awful dependency, can we remove.


#include "mcu-temp.h"


#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

void adc_setup(void)
{

	// rcc_periph_clock_enable(RCC_ADC1);

	// gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO0);
	// gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO1);

	adc_power_off(ADC1);
	adc_disable_scan_mode(ADC1);
	adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_3CYC);

  adc_enable_temperature_sensor();

	adc_power_on(ADC1);

}



uint16_t adc_read_naiive(uint8_t channel)
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


#define TEMP_SENSOR_AVG_SLOPE_MV_PER_CELSIUS                        2.5f
#define TEMP_SENSOR_VOLTAGE_MV_AT_25                                760.0f
#define ADC_REFERENCE_VOLTAGE_MV                                    3300.0f
#define ADC_MAX_OUTPUT_VALUE                                        4095.0f
#define TEMP110_CAL_VALUE                                           ((uint16_t*)((uint32_t)0x1FFF7A2E))
#define TEMP30_CAL_VALUE                                            ((uint16_t*)((uint32_t)0x1FFF7A2C))
#define TEMP110                                                     110.0f
#define TEMP30                                                      30.0f


static double convert_temp( uint16_t sensorValue )
{
    // sensorValue = (float)HAL_ADC_GetValue(&hadc1);
    // HAL_ADC_Stop(&hadc1);
  return ((TEMP110 - TEMP30) / ((float)(*TEMP110_CAL_VALUE) - (float)(*TEMP30_CAL_VALUE)) * (sensorValue - (float)(*TEMP30_CAL_VALUE)) + TEMP30);
}





double adc_temp_read(void )
{
  uint16_t val = adc_read_naiive(  ADC_CHANNEL_TEMP_F40);
  // printf("val %u  convert %.2fC\n", val,  convert);
  return convert_temp( val );
}



double adc_temp_read10(void )
{
  // so much damn variance.

  float vals[10];
  assert( ARRAY_SIZE(vals) == 10);

  for(unsigned i = 0; i < ARRAY_SIZE(vals); ++i ) {
    vals[ i ] = adc_temp_read();
  }

  return mean(vals, ARRAY_SIZE(vals));
}



