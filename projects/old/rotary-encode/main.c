/*
 *
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


// #include "miniprintf.h"
#include "usart.h"
#include "stepper.h"
#include "rotary.h"
#include "blink.h"



// So we probably also want an interrupt...
// setup on rotary change.

// could move this to rotary... as some test code

static int last = 0;

static void report_timer_task(void *args __attribute__((unused))) {

  // Ahhhh not having a buffer... means
  for (;;) {
    // uart_printf("tim4 %u   tim5 %d\n\r", timer_get_counter( TIM4 ), timer_get_counter( TIM5 ));
    int x = timer_get_counter( TIM3 );
    if(x != last) {
      uart_printf("tim3 %d\n\r", timer_get_counter( TIM3 ));
      last = x;
    }

    // uart_printf("val %u\n\r", TIM_CR2_MMS_UPDATE);
    // uart_printf("val %u\n\r", 0x20 );
    // uart_printf("val %u\n\r", TIM_CR2_MMS_UPDATE == 0x20 );
  }
}


// maybe in slave mode it cannot do interupts...
// or the trigger is no good?




static void led_blink_task2(void *args __attribute((unused))) {

	for (;;) {

    int x = timer_get_counter( TIM3 );
    if(x > 50) {
      timer_set_counter(TIM3, 50);
      x = 50;
    }

		gpio_toggle(GPIOE,GPIO0);


      uart_printf("tim3 %d\n\r", x );
		vTaskDelay(pdMS_TO_TICKS(  500 - (x * 10) )); // 1Hz
	}
}


// if cannot get to work. try the pwm pulse  timer.c example


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
  // stepper counter
  rcc_periph_clock_enable(RCC_TIM5);  // WHY NOT MOVE TO WHERE USED?


  // rotary
  // rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_TIM3);



  ///////////////
  // setup
  led_setup();
  usart_setup();

  // stepper
  stepper_timer_setup();
  stepper_timer_counter_setup();

  // rotary
  // TIM3, PA6 and PA7
  rotary_setup(TIM3, GPIOA, GPIO6, GPIO_AF2, GPIOA, GPIO7, GPIO_AF2) ;


  ///////////////
  // tasks
  // value is the stackdepth.
	xTaskCreate(led_blink_task2, "LED",100,NULL,configMAX_PRIORITIES-1,NULL);
  xTaskCreate(uart_task,      "UART",200,NULL,configMAX_PRIORITIES-1,NULL); /* Highest priority */

  // IMPORTANT setting from 100 to 200, stops deadlock
  xTaskCreate(prompt_task,    "PROMPT",200,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */

  // VERY IMPORTANT...
  // possible that the echo - from uart ends up deadlocked.
  //xTaskCreate( report_timer_task,  "REPORT",200,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */

	vTaskStartScheduler();

	for (;;);
	return 0;
}

// End
