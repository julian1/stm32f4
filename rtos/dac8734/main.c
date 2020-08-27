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


#include "usart.h"
#include "blink.h"






//
#define DAC_SPI       1

// use spi1/ port A alternate function
#define DAC_CS        GPIO4
#define DAC_CLK       GPIO5
#define DAC_MOSI      GPIO6
#define DAC_MISO      GPIO7

//  GPIOE
#define DAC_PORT      GPIOE

// THESE ARE WRONG...
#define DAC_LDAC      GPIO2
#define DAC_RST       GPIO3
#define DAC_GPIO0     GPIO4  // gpio pe4   dac pin 8
#define DAC_GPIO1     GPIO5
#define DAC_UNIBIPA   GPIO6



static int last = 0;

static void led_blink_task2(void *args __attribute((unused))) {

	for (;;) {

		gpio_toggle(GPIOE,GPIO0); // OK. the toggle isn't working...


  /// doesn't look right.... values 4 and 5 match the port numbers...
     int u0 = gpio_get(DAC_PORT, DAC_GPIO0 ); // why is this numerical 4?
     int u1 = gpio_get(DAC_PORT, DAC_GPIO1 ); // why is this numerical 4?

    uart_printf("hi %d %d %d\n\r", last++, u0, u1 );
    // uart_printf("gpio0 %d \n\r", u0 );
    // uart_printf("gpio1 %d \n\r", u1 );

  // whether we use pu/pd changes value but only for gpio1

		vTaskDelay(pdMS_TO_TICKS(  500  )); // 1Hz
		// vTaskDelay(pdMS_TO_TICKS(  100  )); // 10Hz
  }
}


// gpio0 is always 4.  bit strange.

static void dac_setup( void )
{

  // spi alternate function 5
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, DAC_CS | DAC_CLK | DAC_MOSI | DAC_MISO );

  gpio_set_af(GPIOA, GPIO_AF5, DAC_CS | DAC_CLK | DAC_MOSI | DAC_MISO );

  rcc_periph_clock_enable(RCC_SPI1);
  spi_init_master(DAC_SPI, SPI_CR1_BAUDRATE_FPCLK_DIV_4,
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
    SPI_CR1_CPHA_CLK_TRANSITION_1,
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST);              // check.
  spi_enable_ss_output(DAC_SPI);
  spi_enable(DAC_SPI);

  /////
  // other outputs
  gpio_mode_setup(DAC_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, DAC_LDAC | DAC_RST | DAC_UNIBIPA);


  // OK. pull ups work, read 0 if use pull down.
  gpio_mode_setup(DAC_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, DAC_GPIO0 | DAC_GPIO1 );

}

int main(void) {

  ////
  // clocks
  rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

  // led
  rcc_periph_clock_enable(RCC_GPIOE); // JA

  // usart
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_USART1);

  // spi1
  rcc_periph_clock_enable(RCC_SPI1);



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

	vTaskStartScheduler();

	for (;;);
	return 0;
}



/*


static void report_timer_task(void *args __attribute__((unused))) {

  for (;;) {
      uart_printf("hi %d\n\r", last++);

  }
}



  xTaskCreate( report_timer_task,  "REPORT",200,NULL,configMAX_PRIORITIES-2,NULL);


*/



