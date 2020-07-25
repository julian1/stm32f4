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


#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>


// #include <libopencm3/stm32/gpio.h>
// #include <libopencm3/stm32/usart.h>
// #include <libopencm3/cm3/nvic.h>


#include "miniprintf.h"
#include "util.h"








static void stepper_timer_setup(void)
{

/*
	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

  rcc_periph_clock_enable(RCC_GPIOD);
  rcc_periph_clock_enable(RCC_TIM4);
*/

  gpio_mode_setup(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO12 | GPIO13 | GPIO14 | GPIO15);
  gpio_set_af(GPIOD, GPIO_AF2, GPIO12 | GPIO13 | GPIO14 | GPIO15);
  gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, GPIO12 | GPIO13 | GPIO14 | GPIO15);


  rcc_periph_reset_pulse(RST_TIM4);   // good practice
	timer_set_prescaler(TIM4, 65535 );  // blinks 1/s.

  // timer_disable_preload(TIM4);
  // timer_continuous_mode(TIM4);

  // timer is up counting.
  timer_set_mode(TIM4, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
  // timer_set_mode(TIM4, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_CENTER_1, TIM_CR1_DIR_UP);
  timer_enable_preload(TIM4);
  timer_enable_break_main_output(TIM4); // what does this do
  timer_set_period(TIM4, 2000);

  ////////
  // configure the channnel outputs to toggle on the oc (output compare) value

  // channel 1
  timer_set_oc_mode(TIM4, TIM_OC1, TIM_OCM_TOGGLE);
  timer_enable_oc_output(TIM4, TIM_OC1);
  timer_set_oc_value(TIM4, TIM_OC1, 1000);

  // channel 2
  timer_set_oc_mode(TIM4, TIM_OC2, TIM_OCM_TOGGLE); // OK. this inverts from PWM1. eg. its the bottom.
  timer_enable_oc_output(TIM4, TIM_OC2);
  timer_set_oc_value(TIM4, TIM_OC2, 1); // 1 not zero, to catch on the upward count...

  // chan 3, same as 1 except flip polarity
  timer_set_oc_mode(TIM4, TIM_OC3, TIM_OCM_TOGGLE);
  timer_enable_oc_output(TIM4, TIM_OC3);
  timer_set_oc_value(TIM4, TIM_OC3, 1000);
  timer_set_oc_polarity_low(TIM4, TIM_OC3); // flip

  // chan4, same as 3 except flip polarity
  timer_set_oc_mode(TIM4, TIM_OC4, TIM_OCM_TOGGLE);
  timer_enable_oc_output(TIM4, TIM_OC4);
  timer_set_oc_value(TIM4, TIM_OC4, 1);
  timer_set_oc_polarity_low(TIM4, TIM_OC4); // flip


  timer_enable_counter(TIM4);

}



// ok want a task to print the timer value

static void report_timer_task(void *args __attribute__((unused))) {

  for (;;) {
    uart_printf("tim4 %u\n\r", timer_get_counter( TIM4 ));
  }
}



int main(void) {

  ///////////////
  // clocks
  rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

  // led
  rcc_periph_clock_enable(RCC_GPIOE); // JA

  // usart
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_USART1);

  // stepper
  rcc_periph_clock_enable(RCC_GPIOD);
  rcc_periph_clock_enable(RCC_TIM4);


  ///////////////
  // setup
  led_setup();
  usart_setup();


  stepper_timer_setup();

  ///////////////
  // tasks

	xTaskCreate(led_blink_task, "LED",100,NULL,configMAX_PRIORITIES-1,NULL);
  xTaskCreate(uart_task,      "UART",200,NULL,configMAX_PRIORITIES-1,NULL); /* Highest priority */
  xTaskCreate(prompt_task,    "PROMPT",100,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */


  xTaskCreate( report_timer_task,  "REPORT",100,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */

	vTaskStartScheduler();

	for (;;);
	return 0;
}

// End
