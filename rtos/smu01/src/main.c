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

#include <libopencm3/stm32/adc.h>


#include "sleep.h"
#include "usart.h"
#include "led.h"
#include "rails.h"
#include "ref.h"
#include "dac8734.h"


static uint16_t read_adc_native(uint8_t channel);

static void led_blink_task2(void *args __attribute((unused)))
{
  static int tick = 0;

	for (;;) {

    // led_toggle
		led_toggle();

    // ping
    // uart_printf("ping %d\n\r", tick++);

/*
    uart_printf("hi %d %d %d\n\r",
      last++,
      gpio_get(DAC_PORT, DAC_GPIO0),
      gpio_get(DAC_PORT, DAC_GPIO1 )

    );
*/
    // at gnd 0-2, at 3.3V supply get 4095.  eg. 4096 = 12bit. good. but maybe resolution is off.
    /*
      ADC channel numbers
      http://libopencm3.org/docs/latest/stm32f4/html/group__adc__channel.html
    */

		uint16_t input_adc0 = read_adc_native(0);   // PA0.

    // don't think this works...
		// uint16_t input_adc1 = read_adc_native( ADC_CHANNEL_VREF);
		uint16_t input_adc1 = read_adc_native( ADC_CHANNEL_VBAT);
		// uint16_t input_adc1 = read_adc_native( ADC_CHANNEL_TEMP_F40 );

    /*
      OK
      VREF seems quite stable...  // 700 to 704
      VBAT also 745 to 750
      so maybe its working...
      TEMP_F40 went 770 to 850 with heat gun... pointed at it. and back again.
    */

		uart_printf("tick: %d: adc0=%u adc1=%d\n", tick++, input_adc0, input_adc1);

    task_sleep(500);
  }
}






static void dac_test(void)
{

  dac_reset();

  uart_printf("dac test\n\r");

  /*
  34,the digital supplies (DVDD and IOVDD) and logic inputs (UNI/BIP-x) must be
  applied before AVSS and AVDD. Additionally, AVSS must be applied before AVDD
  unless both can ramp up at the same time. REF-x should be applied after AVDD
  comes up in order to make sure the ESD protection circuitry does not turn on.
  */
  task_sleep(50);
  rails_negative_on();
  task_sleep(50);
  rails_positive_on();
  task_sleep(50);
  ref_on();
  task_sleep(50);

  uart_printf("dac writing dac registers\n\r");

  dac_write_register(0x04, 50000 );  // Iout == 10V, need macro for DAC_OUT0
  dac_write_register(0x05, 25000 );  // Vout == 5V

  /*
    ok for v reference of 6.5536V
    rails need to be 6.5536 * 2 + 1 == 14.1V.
  */
#if 0
  dac_write_register1( 0b00000110 << 16 | 0x7f7f ); // write dac 2
  dac_write_register1( 0b00000111 << 16 | 0x7f7f ); // write dac 3
  task_sleep(1);  // must wait for update - before we read
#endif


  // select ain auxillary monitor.
  // 11 is ain. 13 is dac1.
  uart_printf("dac write mon register for ain\n\r");
  // dac_write_register1( 0b00000001 << 16 | (1 << 11) ); // select AIN.
  // dac_write_register1( 0b00000001 << 16 | (1 << 13) ); // select dac 1.

  dac_write_register(0x01, (1 << 13) ); // select monitor dac1

  uart_printf("dac test finished\n\r");
}



static void mux_setup(void)
{
  uart_printf("mux setup\n\r");
  // call *before* bringing up rails
  gpio_set(GPIOE, GPIO1 | GPIO2 |  GPIO3 | GPIO4);   // active low.
  gpio_mode_setup(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1 | GPIO2 |  GPIO3 | GPIO4);
  uart_printf("mux setup done\n\r");
}


static void mux_test(void)
{
  // U1
  uart_printf("mux test \n\r");

  // gpio_clear(GPIOE, GPIO1 );   // top-left      VFB
   gpio_clear(GPIOE, GPIO2 );   // bottom-right  VSET  ... gives us +9V on TP4 / VERR
                                  // but top-right shows 9V on both sides why - even though off. why?
                                  // Yes. So it flows through the resistor network. about 3x resistors.
                                  // Hmmm...

  //gpio_clear(GPIOE, GPIO3 );   // top-right     VSET  --- something weird. drawing 20mA. no output.
                                  // looks like it shorts. a bit
  // gpio_clear(GPIOE, GPIO4);       // bottom-left   VFB

  uart_printf("mux test finished\n\r");
}



static void dac_test1(void *args __attribute((unused)))
{
  dac_test();

  mux_test();


  // sleep forever
  for(;;) {
    task_sleep(1000);
  }
}




static void adc_setup(void)
{
	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO0);
	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO1);

	adc_power_off(ADC1);
	adc_disable_scan_mode(ADC1);
	adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_3CYC); // is this enough for all bits?

	adc_power_on(ADC1);

}


static uint16_t read_adc_native(uint8_t channel)
{
  // set up the arry of channels to read.
	uint8_t channel_array[16];
	channel_array[0] = channel;
	adc_set_regular_sequence(ADC1, 1, channel_array); // 1 indicates number of channels to read. eg. 1

  // start the read
	adc_start_conversion_regular(ADC1);
	while (!adc_eoc(ADC1));
	uint16_t reg16 = adc_read_regular(ADC1);
	return reg16;
}

int main(void) {

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

  // spi1
  rcc_periph_clock_enable(RCC_SPI1);

  // adc1
	rcc_periph_clock_enable(RCC_ADC1);

  ///////////////
  // setup
  // TODO maybe change names setup_led() setup_uart() ?
  led_setup();
  usart_setup();
  uart_printf("------------------\n\r");
  uart_printf("starting\n\r");

#if 0
  // peripheral setup
  rails_setup();
  ref_setup();
  // dac_setup_bitbash();
  dac_setup_spi();
  mux_setup();
#endif
  adc_setup();


  ///////////////
  // tasks
  // value is the stackdepth.
	xTaskCreate(led_blink_task2,  "LED",100,NULL,configMAX_PRIORITIES-1,NULL);
  xTaskCreate(uart_task,        "UART",200,NULL,configMAX_PRIORITIES-1,NULL); /* Highest priority */

  // IMPORTANT changing from 100 to 200, stops deadlock
  xTaskCreate(usart_prompt_task,"PROMPT",200,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */

  xTaskCreate(dac_test1,        "DAC_TEST",200,NULL,configMAX_PRIORITIES-2,NULL); // Lower priority

	vTaskStartScheduler();

  // should never get here?
	for (;;);
	return 0;
}


