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

static void report_timer_task(void *args __attribute__((unused))) {

  // Ahhhh not having a buffer... means
  for (;;) {
    // uart_printf("tim4 %u   tim5 %d\n\r", timer_get_counter( TIM4 ), timer_get_counter( TIM5 ));
    uart_printf("tim3 %d\n\r", timer_get_counter( TIM3 ));

    // uart_printf("val %u\n\r", TIM_CR2_MMS_UPDATE);
    // uart_printf("val %u\n\r", 0x20 );
    // uart_printf("val %u\n\r", TIM_CR2_MMS_UPDATE == 0x20 );
  }
}


// maybe in slave mode it cannot do interupts...
// or the trigger is no good?



static QueueHandle_t rotary_txq;

static void rotary_setup_interupt(void)
{

  rotary_txq = xQueueCreate(256,sizeof(char));

  // Ahhh interupt - cannot be set 
  nvic_enable_irq(NVIC_TIM3_IRQ);

  // timer_enable_irq(TIM3, TIM_DIER_CC1IE);
  timer_enable_irq(TIM3, TIM_DIER_CC1IE | TIM_DIER_CC2IE | TIM_DIER_CC3IE | TIM_DIER_CC4IE); 
  
}

// CC2 does something as well. sometimes. 


// someone else is using TIM_SR_CC2IF
 
void tim3_isr(void)
{
  char ch = 'x';

   // timer_clear_flag(TIM3, TIM_SR_CC2IF);
  gpio_toggle(GPIOE,GPIO0);

  xQueueSend(rotary_txq, &ch, portMAX_DELAY); // blocks when queue is full
  // IMPORTANT -- could just send a report.
  // uart_printf("tim3 interrupt %d\n\r", timer_get_counter( TIM3 ));
  //uart_printf("i");
   //timer_clear_flag(TIM3, TIM_SR_CC2IF | );  // not clearing the interrupt will freeze it.
                                          // why CC2IF and not CC2IF?

   timer_clear_flag(TIM3, TIM_DIER_CC1IE | TIM_DIER_CC2IE | TIM_DIER_CC3IE | TIM_DIER_CC4IE );  // not clearing the interrupt will freeze it.
}



static void rotary_task(void *args __attribute__((unused))) {
  char ch;

  for (;;) {
    if ( xQueueReceive(rotary_txq,&ch,500) == pdPASS ) {
      uart_printf("x");
      // uart_printf("x\n\r");
    }
    //taskYIELD();  
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

  // nvic_enable_irq(NVIC_TIM3_IRQ);
  rotary_setup(TIM3, GPIOA, GPIO6, GPIO_AF2, GPIOA, GPIO7, GPIO_AF2) ;
  rotary_setup_interupt(); // uggly... because fixed 


  ///////////////
  // tasks
  // value is the stackdepth.
	xTaskCreate(led_blink_task, "LED",100,NULL,configMAX_PRIORITIES-1,NULL);
  xTaskCreate(uart_task,      "UART",200,NULL,configMAX_PRIORITIES-1,NULL); /* Highest priority */
  xTaskCreate(prompt_task,    "PROMPT",100,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */
  xTaskCreate(rotary_task,    "ROTARY",100,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */


  xTaskCreate( report_timer_task,  "REPORT",200,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */

	vTaskStartScheduler();

	for (;;);
	return 0;
}

// End
