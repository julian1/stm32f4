/*
  simple led blink task,
  and answer on uart.

*/


#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>


//////////

#include "sleep.h"
#include "usart.h"
#include "serial.h"



#define LED_PORT  GPIOE
#define LED_OUT   GPIO15





static void task1(void *args __attribute((unused))) {

	for (;;) {
		gpio_toggle(LED_PORT,LED_OUT);
		vTaskDelay(pdMS_TO_TICKS(500)); // 1Hz
	}
}




static void led_setup(void)
{
  gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_OUT); // JA - move to function led_setup.
}



////////////////
///  ads131a04
#define ADC_SPI_PORT GPIOB
#define ADC_SPI     SPI2

#define ADC_M0      GPIO6
#define ADC_M1      GPIO7
#define ADC_M2      GPIO8
#define ADC_DONE    GPIO9
#define ADC_DRDY    GPIO10
#define ADC_RESET   GPIO11
#define ADC_CS      GPIO12      // SPI2_NSS
#define ADC_SCLK    GPIO13      // SPI2_SCK
#define ADC_MISO    GPIO14      // SPI2_MISO
#define ADC_MOSI    GPIO15      // SPI2_MISO



static void adc_setup_spi( void )
{
  uint32_t all = ADC_CS | ADC_SCLK | ADC_MISO | ADC_MOSI;

  usart_printf("adc setup spi\n");

  // spi alternate function
  gpio_mode_setup(ADC_SPI_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, all);

  // OK.. THIS MADE SPI WORK AGAIN....
  // note, need harder edges for signal integrity. or else different speed just helps suppress parasitic components
  // see, https://www.eevblog.com/forum/microcontrollers/libopencm3-stm32l100rc-discovery-and-spi-issues/
  gpio_set_output_options(ADC_SPI_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, all);

  // af 5 for spi2 on PB
  gpio_set_af(ADC_SPI_PORT, GPIO_AF5, all);

  // rcc_periph_clock_enable(RCC_SPI1);
  spi_init_master(
    ADC_SPI,
    SPI_CR1_BAUDRATE_FPCLK_DIV_4,     // SPI_CR1_BAUDRATE_FPCLK_DIV_256,
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,  // SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE ,
    SPI_CR1_CPHA_CLK_TRANSITION_2,    // 2 == falling edge (from dac8734 doc.
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST                  // SPI_CR1_LSBFIRST
  );

  spi_disable_software_slave_management( ADC_SPI);
  spi_enable_ss_output(ADC_SPI);
  spi_enable(ADC_SPI);



  uint32_t out = ADC_M0 | ADC_M1 | ADC_M2 | ADC_RESET;
  uint32_t in =  ADC_DONE | ADC_DRDY;

  // should setup a reasonable state first...

  gpio_mode_setup(ADC_SPI_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, out );
  gpio_mode_setup(ADC_SPI_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, in );
}


static void adc_reset( void )
{

#if 0
  // reset
  gpio_clear(DAC_PORT, DAC_RST);
  task_sleep(20);
  gpio_set(DAC_PORT, DAC_RST);
  task_sleep(20);
#endif

}




static void test01(void *args __attribute((unused)))
{

  usart_printf("test01\n");

  adc_reset();

  usart_printf("test01 done\n");


  // sleep forever
  for(;;) {
    task_sleep(1000);
  }
}

int main(void) {

  // ONLY WORKS if fit crystal.
  // rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

  // LED
  rcc_periph_clock_enable(RCC_GPIOE); // LED_PORT JA

  // USART
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_USART1);

  // adc02 gpio
  rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_SPI2);


  ///////////////
  led_setup();
  usart_setup();

  ///////////////

  adc_setup_spi();





  xTaskCreate(task1,  "LED",100,NULL,configMAX_PRIORITIES-1,NULL);

  // IMPORTANT changing from 100 to 200, stops deadlock
  xTaskCreate(usart_task,        "UART",200,NULL,configMAX_PRIORITIES-1,NULL); /* Highest priority */

  xTaskCreate(serial_prompt_task,"SERIAL2",200,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */


  xTaskCreate(test01,        "TEST01",200,NULL,configMAX_PRIORITIES-2,NULL); // Lower priority

	vTaskStartScheduler();

	for (;;);
	return 0;
}

