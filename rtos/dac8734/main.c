/*
 *
  see, for
  ~/devel/stm32/FreeRTOSv10.3.1/FreeRTOS/Demo/CORTEX_M4F_STM32F407ZG-SK/FreeRTOSConfig.h

  doc,
  https://www.freertos.org/FreeRTOS-for-STM32F4xx-Cortex-M4F-IAR.html

  so, i think 'proper' usart will use dma.

  // OK. so we want to move the dac code.

 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/*
  - ok. move back to actual spi port - see if can still bit bash.
  - see if peripheral spi works.
  - mon - to ADC - resistor divider? won't work - for negative signals.

  - we want
*/

// #include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
// #include <libopencm3/stm32/timer.h>
// #include <libopencm3/stm32/spi.h>
// #include <libopencm3/stm32/gpio.h>
// #include <libopencm3/stm32/usart.h>
// #include <libopencm3/cm3/nvic.h>


#include "usart.h"
#include "led.h"
#include "rails.h"
#include "dac8734.h"
#include "utility.h" // msleep


/*
  OK. our sequencing *is* no good.
    - because we are pumping in the reference before the rails are up.
    - we need to fix this.

*/

static int last = 0;

static void led_blink_task2(void *args __attribute((unused))) {

	for (;;) {

    // led_toggle
		led_toggle();

    // ping
    uart_printf("ping %d\n\r", last++);


    // dac_write_register(0x05, 50000  - last);  // Vout == 10V

/*
    uart_printf("hi %d %d %d\n\r",
      last++,
      gpio_get(DAC_PORT, DAC_GPIO0),
      gpio_get(DAC_PORT, DAC_GPIO1 )

    );
*/
    msleep(500);
  }
}



int main(void) {

  ////
  // Disable HSE
  // clocks
  // rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

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


  uart_printf("------------------\n\r");

/*
  This organisation is really bad...
  There should be a single linear task that does all the configuration.
  Rather than doing half in the main thread, and half in the task thread.
*/


  rails_setup();

/*
  // dac_setup_bitbash();
  dac_setup_spi();
*/

  ///////////////
  // tasks
  // value is the stackdepth.
	xTaskCreate(led_blink_task2, "LED",100,NULL,configMAX_PRIORITIES-1,NULL);
  xTaskCreate(uart_task,      "UART",200,NULL,configMAX_PRIORITIES-1,NULL); /* Highest priority */

  // IMPORTANT setting from 100 to 200, stops deadlock
  xTaskCreate(usart_prompt_task,    "PROMPT",200,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */

/*
  // ok....
  xTaskCreate(dac_test,    "DAC_TEST",200,NULL,configMAX_PRIORITIES-2,NULL); // Lower priority
*/

	vTaskStartScheduler();

	for (;;);
	return 0;
}


