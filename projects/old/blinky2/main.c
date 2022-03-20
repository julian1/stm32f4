/*
  simple led blink task,
  and answer on uart.

*/


#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>


//////////

#include "sleep.h"
#include "usart.h"
#include "serial.h"



#define LED_PORT  GPIOE
#define LED_OUT   GPIO15


void vApplicationStackOverflowHook(void)
{

}



static void task1(void *args __attribute((unused))) {

	for (;;) {
		gpio_toggle(LED_PORT,LED_OUT);
		vTaskDelay(pdMS_TO_TICKS(500)); // 1Hz
	}
}




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
  usart1_setup();

  ///////////////


  xTaskCreate(task1,  "LED",100,NULL,configMAX_PRIORITIES-1,NULL);

  // IMPORTANT changing from 100 to 200, stops deadlock
  xTaskCreate(usart_task,        "UART",200,NULL,configMAX_PRIORITIES-1,NULL); /* Highest priority */

  xTaskCreate(serial_prompt_task,"SERIAL2",200,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */



	vTaskStartScheduler();

	for (;;);
	return 0;
}

