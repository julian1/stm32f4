/* Simple LED task demo, using timed delays:
 *
 * The LED on PC13 is toggled in task1.


  see, for
  ~/devel/stm32/FreeRTOSv10.3.1/FreeRTOS/Demo/CORTEX_M4F_STM32F407ZG-SK/FreeRTOSConfig.h

  doc,
  https://www.freertos.org/FreeRTOS-for-STM32F4xx-Cortex-M4F-IAR.html

  so, i think 'proper' usart will use dma.

 */

#if 0 
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>

//#include <stdarg.h>
// #include <stdio.h>

// using the ww library...
//#include "uartlib.h"
#include "miniprintf.h"

#endif

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

// ideally, nothing too mcu complicated here,
// #include <libopencm3/stm32/timer.h>
// #include <libopencm3/stm32/spi.h>
// #include <libopencm3/stm32/gpio.h>
// #include <libopencm3/stm32/usart.h>
// #include <libopencm3/cm3/nvic.h>
// #include <libopencm3/stm32/adc.h>
// #include <libopencm3/stm32/exti.h>


#include <string.h>   // strcmp


//////////

#include "sleep.h"
#include "usart.h"
#include "serial.h"




#define LED_PORT  GPIOE
#define LED_OUT   GPIO15





static void task1(void *args __attribute((unused))) {

	for (;;) {
		gpio_toggle(LED_PORT,LED_OUT);
		vTaskDelay(pdMS_TO_TICKS(500)); // 1Hz
	}
}




#if 0
static char buf[100];

static void demo_task(void *args __attribute__((unused))) {

  // buf size of 10 - seems ok
  // buf size of 50 - ok.
  // OK. buf size of 100. fails and stack exception condition caught - led blinks fast.
  // so we have a margin of somewhere between 50 - 100 bytes or so...
  // ok - at 70 - short input strings are ok - but will stack overflow on longer input strings..
  // char buf[70];

  for (;;) {
    // uart_printf is cooked ... so it should already be giving us stuff...
    usart_printf("\n\r> ");
    uart_gets( buf, 100 ); // ie. should block...
    uart_printf("\n\ruuu you said '%s'", buf );   // there looks like a bug in the formatting...
                                              // no it's just returning the \n but not the \r...
  }
}
#endif



static void led_setup(void) 
{
  gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_OUT); // JA - move to function led_setup.
}



int main(void) {

  // ONLY WORKS if fit crystal.
  // rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

  // LED
  rcc_periph_clock_enable(RCC_GPIOE); // LED_PORT JA

  // USART
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_USART1);


  ///////////////
  led_setup();
  usart_setup();

  ///////////////



  xTaskCreate(task1,  "LED",100,NULL,configMAX_PRIORITIES-1,NULL);


  // IMPORTANT changing from 100 to 200, stops deadlock
  xTaskCreate(usart_task,        "UART",200,NULL,configMAX_PRIORITIES-1,NULL); /* Highest priority */

  // xTaskCreate(serial_prompt_task,"SERIAL",200,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */
  xTaskCreate(serial_prompt_task,"SERIAL2",200,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */



	vTaskStartScheduler();

	for (;;);
	return 0;
}

// End
