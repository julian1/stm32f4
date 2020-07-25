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
  timer_set_period(TIM4, 1000);

  ////////
  // configure the channnel outputs to toggle on the oc (output compare) value

  // channel 1
  timer_set_oc_mode(TIM4, TIM_OC1, TIM_OCM_TOGGLE);
  timer_enable_oc_output(TIM4, TIM_OC1);
  timer_set_oc_value(TIM4, TIM_OC1, 500);

  // channel 2
  timer_set_oc_mode(TIM4, TIM_OC2, TIM_OCM_TOGGLE); // OK. this inverts from PWM1. eg. its the bottom.
  timer_enable_oc_output(TIM4, TIM_OC2);
  timer_set_oc_value(TIM4, TIM_OC2, 1 /*999*/); // 1 not zero, to catch on the upward count...

  // chan 3, same as 1 except flip polarity
  timer_set_oc_mode(TIM4, TIM_OC3, TIM_OCM_TOGGLE);
  timer_enable_oc_output(TIM4, TIM_OC3);
  timer_set_oc_value(TIM4, TIM_OC3, 500);
  timer_set_oc_polarity_low(TIM4, TIM_OC3); // flip

  // chan4, same as 3 except flip polarity
  timer_set_oc_mode(TIM4, TIM_OC4, TIM_OCM_TOGGLE);
  timer_enable_oc_output(TIM4, TIM_OC4);
  timer_set_oc_value(TIM4, TIM_OC4, 1 /*999*/);
  timer_set_oc_polarity_low(TIM4, TIM_OC4); // flip


  timer_enable_counter(TIM4);
}


static void timer2_setup(void)
{
  // tim5 really looks to be 32 bit.
  // note that we can treat it as signed - to get reverse position.

  timer_set_master_mode(TIM4,  TIM_CR2_MMS_UPDATE /* 0x20 */);  // set TIM4 as master
  // timer_set_master_mode(TIM4,  TIM_CR2_MMS_COMPARE_PULSE );
  // seems - really only want a single OC ...

  // its a number, not a bitfield unfortunately. but we can select an individual oc
  // timer_set_master_mode(TIM4,  TIM_CR2_MMS_COMPARE_OC2REF  );  // will update on 500.

  /*
  OK, incredibly neat. we can select the output channel to decide when to let it tick over.
  its a bitfield not a numbber, so cannot trigger on all channels combined.
  tim4 499   tim5 3
  tim4 501   tim5 4

  -- VERY IMPORTANT.
  - OK - maybe we just need to think of the position. as 1 - position.
  */
  rcc_periph_reset_pulse(RST_TIM5);   // good practice


  // timer is up counting.
  // timer_set_mode(TIM5, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_DOWN); // useful for reversing?
  // timer_enable_break_main_output(TIM5); // what does this do?

  // timer_set_period(TIM5, 5 );  // works - can be set for detents... but probably better to use mod %

  // set external trigger
  timer_slave_set_mode( TIM5, TIM_SMCR_SMS_ECM1);

  // set to follow TIM4, see, links https://blog.csdn.net/zwlforever/article/details/89021249
  timer_slave_set_trigger(TIM5, TIM_SMCR_TS_ITR2);

  // rising or falling doesn't make a difference.
  timer_slave_set_polarity(TIM5, TIM_ET_RISING /*TIM_ET_FALLING */ );   // may be useful for reversing
  // timer_slave_set_polarity(TIM5, TIM_ET_FALLING  );   // may be useful for reversing


  // just enabling the timer will turn it on
  // important need to turn on both timers at same time.
  timer_enable_counter(TIM5);
}




// ok do we want interrupts. ideally yes - for isolating, changing parameters.
// ok, probably want to actually get the motor hooked up, and it generating correctly...
// the counting to one on the first pulse isn't so neat. but maybe its ok - because it will be modulo?
// falling

// OK. we want to connect the motor up. and check that it really does/can work. before do anything else.


static void report_timer_task(void *args __attribute__((unused))) {

  // Ahhhh not having a buffer... means
  for (;;) {
    uart_printf("tim4 %u   tim5 %d\n\r", timer_get_counter( TIM4 ), timer_get_counter( TIM5 ));

    // uart_printf("val %u\n\r", TIM_CR2_MMS_UPDATE);
    // uart_printf("val %u\n\r", 0x20 );
    // uart_printf("val %u\n\r", TIM_CR2_MMS_UPDATE == 0x20 );
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

  rcc_periph_clock_enable(RCC_TIM5);  // WHY NOT MOVE TO WHERE USED?

  ///////////////
  // setup
  led_setup();
  usart_setup();


  stepper_timer_setup();
  timer2_setup();


  ///////////////
  // tasks
  // value is the stackdepth.
	xTaskCreate(led_blink_task, "LED",100,NULL,configMAX_PRIORITIES-1,NULL);
  xTaskCreate(uart_task,      "UART",200,NULL,configMAX_PRIORITIES-1,NULL); /* Highest priority */
  xTaskCreate(prompt_task,    "PROMPT",100,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */


  xTaskCreate( report_timer_task,  "REPORT",200,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */

	vTaskStartScheduler();

	for (;;);
	return 0;
}

// End
