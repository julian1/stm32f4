

#include "FreeRTOS.h"
#include "task.h"

#include <libopencm3/stm32/gpio.h>


//////////////////////////////////////////


#include "sleep.h"
#include "serial.h"
#include "slope_adc.h"


#define ADC_PORT              GPIOD
#define ADC_OUT               GPIO0


// OK. first lets just report the status

void slope_adc_setup(void)
{
  usart_printf("slope_adc setup\n\r");

  gpio_mode_setup(ADC_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, ADC_OUT);

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
