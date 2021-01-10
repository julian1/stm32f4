

#include "FreeRTOS.h"
#include "task.h"

#include <libopencm3/stm32/gpio.h>


//////////////////////////////////////////


#include "sleep.h"
#include "serial.h"
#include "slope_adc.h"


#define ADC_PORT              GPIOD
#define ADC_OUT               GPIO0


void slope_adc_setup(void)
{
  usart_printf("slope_adc setup\n\r");

  gpio_mode_setup(ADC_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, ADC_OUT);

  usart_printf("slope_adc setup done\n\r");
}



static void slope_adc_out_print(void)
{
  static uint32_t tick = 0;

  usart_printf("slope_adc hi tick %d %d\n\r",
      tick++,
      gpio_get(ADC_PORT, ADC_OUT)
    );
}


