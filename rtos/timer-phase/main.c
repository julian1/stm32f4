/* Simple LED task demo, using timed delays:
 *
 * The LED on PC13 is toggled in task1.


  see, for
  ~/devel/stm32/FreeRTOSv10.3.1/FreeRTOS/Demo/CORTEX_M4F_STM32F407ZG-SK/FreeRTOSConfig.h

  doc,
  https://www.freertos.org/FreeRTOS-for-STM32F4xx-Cortex-M4F-IAR.html

  so, i think 'proper' usart will use dma.

 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


#include <libopencm3/stm32/rcc.h>
// #include <libopencm3/stm32/gpio.h>
// #include <libopencm3/stm32/usart.h>
// #include <libopencm3/cm3/nvic.h>


#include "miniprintf.h"
#include "util.h"




int main(void) {

  ///////////////
  // clocks
  rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

  // led
  rcc_periph_clock_enable(RCC_GPIOE); // JA

  // usart
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_USART1);


  ///////////////
  // setup
  led_setup();
  usart_setup();

  ///////////////
  // tasks

	xTaskCreate(task1,    "LED",100,NULL,configMAX_PRIORITIES-1,NULL);

  xTaskCreate(uart_task,"UART",200,NULL,configMAX_PRIORITIES-1,NULL); /* Highest priority */
  xTaskCreate(demo_task,"DEMO",100,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */



	vTaskStartScheduler();

	for (;;);
	return 0;
}

// End
