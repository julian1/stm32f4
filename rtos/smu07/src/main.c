/*
  *********************
  EXTREME.

  should be

  xQueueSendFromISR() not xQueueSend()
  see p369.
    - fixed for usart.c
    - but do we have other ISRs using it?

  *********************

TODO
  libwwg uses its own FreeRTOSCOnfig.h
    which may differ from local.

  see top-level ../../Makefile
  actually no. think it comes from Makefile.incl.

  p357.
    must fix.
    by copying src to local

    from ../Makefile.inc

    maybe just move...
    mini_vprintf_cooked()
*/


/*
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
// #include <libopencm3/stm32/spi.h>
// #include <libopencm3/stm32/gpio.h>
// #include <libopencm3/stm32/usart.h>
// #include <libopencm3/cm3/nvic.h>
// #include <libopencm3/stm32/adc.h>


#include "sleep.h"
#include "usart.h"
#include "led.h"
#include "rails.h"
#include "ref.h"
#include "dac8734.h"
#include "mcu_adc.h"
#include "mux.h"
#include "slope_adc.h"





static void rails_wait_for_voltage(void)
{
  /*
  // move to rails?
  // potentially want a continuous running task to monitor, with queue events - not just a blocking call.
  // also want to pause for a couple of ticks... before unblock
  */
  int tick = 0;

  int good = 0; // how many ticks it's ok...

  while(1) {
    uint16_t pa0 = mcu_adc_read_native(0);   // LP15VP
    uint16_t pa1 = mcu_adc_read_native(1);   // LN15VN

		uart_printf("rails_wait_for_voltage, tick: %d: LP15VP=%u, LN15VN=%d\n", tick++, pa0, pa1);

    if(pa0 > 1000 && pa1 > 1000)
      ++good;
    else
      good = 0;

    if(good >= 3)
      break;

    task_sleep(500);
  }
}



/*
// 65535 increment
// 6.55V   ref. for ref * 2.
*/

static int voltageToDac( float x)
{
  // return x / (6.5769 * 2) * 65535;
  // eg.
  return x / 2.0 * 10000;  // need to add ptf56 60ohm, and then use dac trim registers.
}


static void test01(void *args __attribute((unused)))
{

  uart_printf("test01\n");

  dac_reset();
  refa_off();

  rails_wait_for_voltage();

  task_sleep(50);
  rails_negative_on();
  task_sleep(50);
  rails_positive_on();
  task_sleep(50);


  refa_on();

  dac_write_register(DAC_VSET_REGISTER, voltageToDac( 5.0 ));
  dac_write_register(DAC_ISET_REGISTER, voltageToDac( 4.0 ));


  task_sleep(50);
  rails_p30V_on();

#if 0
  // source_current_test();
  // adc_test();
  // adc_ifb_test();
  // adc_vfb_test();
#endif

  uart_printf("test01 done\n");


  // sleep forever
  for(;;) {
    task_sleep(1000);
  }
}






////////////////////////////



static void relay_setup(void)
{
  gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO9);
  gpio_clear(GPIOD, GPIO9);   // off.
}


static void relay_toggle_task(void *args __attribute((unused)))
{
  // change name test_task

	for (;;) {
		vTaskDelay(pdMS_TO_TICKS(5 * 1000)); // 1Hz
    gpio_toggle(GPIOD, GPIO9);
	}
}

/////////////////////////////

int main(void) {

  // main clocks
  // disable HSE
  // must edit, configCPU_CLOCK_HZ also
  // clocks
  // rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

  // led
  rcc_periph_clock_enable(RCC_GPIOA);

  // usart
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_USART1);


  // relay
  rcc_periph_clock_enable(RCC_GPIOD);

  // rails
  rcc_periph_clock_enable(RCC_GPIOE);

  // mcu adc1
	rcc_periph_clock_enable(RCC_ADC1);

  // spi1 / dac
  rcc_periph_clock_enable(RCC_SPI1);

  // dac / gpio
  rcc_periph_clock_enable(RCC_GPIOB);





  ///////////////
  // setup
  led_setup();
  usart_setup();
  uart_printf("------------------\n\r");
  uart_printf("starting\n\r");


  relay_setup();
  rails_setup();
  mcu_adc_setup();

  ref_setup();
  dac_setup_spi();



  ///////////////
  // setup

	xTaskCreate(led_blink_task,  "LED",100,NULL,configMAX_PRIORITIES-1,NULL);


  // IMPORTANT changing from 100 to 200, stops deadlock
  xTaskCreate(uart_task,        "UART",200,NULL,configMAX_PRIORITIES-1,NULL); /* Highest priority */
  xTaskCreate(usart_prompt_task,"PROMPT",200,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */



  // xTaskCreate(mcu_adc_print_task,"MCU_ADC",200,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */


  xTaskCreate(test01,        "DAC_TEST",200,NULL,configMAX_PRIORITIES-2,NULL); // Lower priority

	// xTaskCreate(relay_toggle_task,  "LED",100,NULL,configMAX_PRIORITIES-1,NULL);



	vTaskStartScheduler();

  // should never get here?
	for (;;);
}




#if 0

int main_old(void) {

  // main clocks
  // disable HSE
  // must edit, configCPU_CLOCK_HZ also
  // clocks
  // rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

  // led
  rcc_periph_clock_enable(RCC_GPIOE); // JA

  // Dac
  rcc_periph_clock_enable(RCC_GPIOB); // JA

  // usart
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_USART1);

  // spi1 / dac
  rcc_periph_clock_enable(RCC_SPI1);

  // mcu adc1
	rcc_periph_clock_enable(RCC_ADC1);

  // adc
  rcc_periph_clock_enable(RCC_GPIOD);



  ///////////////
  // setup
  // TODO maybe change names setup_led() setup_uart() ?
  led_setup();
  usart_setup();
  uart_printf("------------------\n\r");
  uart_printf("starting\n\r");

  // /////////////
  // EXTREME - gpio, clocks, and peripheral configuration ONLY.
  // no actual spi calls
  mcu_adc_setup();
  rails_setup();
  dac_setup_spi();
  // dac_setup_bitbash();
  ref_setup();
  mux_setup();
  slope_adc_setup();

  uart_printf("------------------\n\r");

  ///////////////
  // tasks
  // value is the stackdepth.
	xTaskCreate(led_blink_task,  "LED",100,NULL,configMAX_PRIORITIES-1,NULL);
  xTaskCreate(uart_task,        "UART",200,NULL,configMAX_PRIORITIES-1,NULL); /* Highest priority */

  // IMPORTANT changing from 100 to 200, stops deadlock
  xTaskCreate(usart_prompt_task,"PROMPT",200,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */

  xTaskCreate(test01,        "DAC_TEST",200,NULL,configMAX_PRIORITIES-2,NULL); // Lower priority

	vTaskStartScheduler();

  // should never get here?
	for (;;);
	return 0;
}

#endif


