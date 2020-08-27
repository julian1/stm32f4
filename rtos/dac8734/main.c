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




// #include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
// #include <libopencm3/stm32/timer.h>

#include <libopencm3/stm32/spi.h> 

// #include <libopencm3/stm32/gpio.h>
// #include <libopencm3/stm32/usart.h>
// #include <libopencm3/cm3/nvic.h>


// #include "miniprintf.h"
#include "usart.h"
#include "blink.h"



// So we probably also want an interrupt...
// setup on rotary change.

// could move this to rotary... as some test code

/*
static int last = 0;


static void report_timer_task(void *args __attribute__((unused))) {

  for (;;) {
      uart_printf("hi %d\n\r", last++);

  }
}
*/



static void led_blink_task2(void *args __attribute((unused))) {

	for (;;) {

		gpio_toggle(GPIOE,GPIO0);

		vTaskDelay(pdMS_TO_TICKS(  500  )); // 1Hz
		// vTaskDelay(pdMS_TO_TICKS(  100  )); // 10Hz
	}
}



// 
#define DAC_SPI       1 

// GPIOA
#define DAC_CS        4
#define DAC_CLK       5
#define DAC_MOSI      6
#define DAC_MISO      7


static void dac_setup( void )
{

  // spi alternate function
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, DAC_CS | DAC_CLK |  DAC_MOSI | DAC_MISO );

  gpio_set_af(GPIOA, GPIO_AF5, DAC_CS | DAC_CLK |  DAC_MOSI | DAC_MISO );


  rcc_periph_clock_enable(RCC_SPI1);
  spi_init_master(DAC_SPI, SPI_CR1_BAUDRATE_FPCLK_DIV_4,
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
    SPI_CR1_CPHA_CLK_TRANSITION_1,
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST);              // check.
  spi_enable_ss_output(DAC_SPI);
  spi_enable(DAC_SPI);

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

  // rcc_periph_clock_enable(RCC_SPI1);



  ///////////////
  // setup
  led_setup();
  usart_setup();

  dac_setup();

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
