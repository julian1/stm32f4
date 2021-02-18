/*

  Issue. any kind of interrupt (uart rx, gpio exti) causes freertos to hang

  Simplest example.
    blinking led task.
    and configure uart.
    crashes whenever on uart rx interrupt
*/


#include "FreeRTOS.h"
#include "task.h"


#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>

#include <libopencm3/cm3/scb.h>


#define LED_PORT  GPIOE
#define LED_OUT   GPIO15


static void critical_error_blink(void)
{
	for (;;) {
		gpio_toggle(LED_PORT,LED_OUT);
		for(uint32_t i = 0; i < 500000; ++i)
       __asm__("nop");
	}
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, char * pcTaskName )
{
  (void) xTask;
  (void) pcTaskName;
  critical_error_blink();

}

void vApplicationMallocFailedHook(void)
{
  critical_error_blink();
}

//////////////////



static void led_setup(void)
{
  gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_OUT);
}

static void led_task(void *args __attribute((unused)))
{
	for (;;) {
		gpio_toggle(LED_PORT,LED_OUT);
		vTaskDelay(pdMS_TO_TICKS(500)); // 1Hz
	}
}



static void usart_setup(void)
{

  nvic_enable_irq(NVIC_USART1_IRQ);
  nvic_set_priority(NVIC_USART1_IRQ,  5 );    // changing this has no effect

  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9  | GPIO10);

  gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO10);

  gpio_set_af(GPIOA, GPIO_AF7, GPIO9  | GPIO10);

  usart_set_baudrate(USART1, 115200);
  usart_set_databits(USART1, 8);
  usart_set_stopbits(USART1, USART_STOPBITS_1);
  usart_set_mode(USART1, USART_MODE_TX_RX);

  usart_set_mode(USART1, USART_MODE_TX_RX);
  usart_set_parity(USART1, USART_PARITY_NONE);
  usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);


  // enabling any kind of interrupt makes freertos hang on interrupt
#if 1
  /* Enable USART1 Receive interrupt. */
  usart_enable_rx_interrupt(USART1);
#endif


  usart_enable(USART1);
}


void usart1_isr(void)
{
  // do nothing, still hangs
  return ;
}




int main(void)
{

  // LED
  rcc_periph_clock_enable(RCC_GPIOE);

  // USART
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_USART1);


  // rcc_periph_clock_enable(RCC_SYSCFG); // should only be needed for external interupts.

 /*
  http://www.freertos.org/RTOS-Cortex-M3-M4.html
  Preempt priority and subpriority:
   If you are using an STM32 with the STM32 driver library then ensure all the
   priority bits are assigned to be preempt priority bits by calling
   NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 ); before the RTOS is started.
 */

  scb_set_priority_grouping(SCB_AIRCR_PRIGROUP_GROUP16_NOSUB);

  led_setup();
  usart_setup();


  xTaskCreate(led_task,  "LED",1000,NULL,configMAX_PRIORITIES-1,NULL);


	vTaskStartScheduler();

	for (;;);
	return 0;
}


