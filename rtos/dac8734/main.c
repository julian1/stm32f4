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
#include "led.h"


#define DAC_PORT_CS   GPIOA
#define DAC_CS        GPIO4

//
#define DAC_SPI       SPI1

// use spi1/ port A alternate function
#define DAC_CLK       GPIO5
#define DAC_MOSI      GPIO6
#define DAC_MISO      GPIO7

//  GPIOE
#define DAC_PORT      GPIOE   // GPIO1 is led.
#define DAC_LDAC      GPIO2
#define DAC_RST       GPIO3
#define DAC_GPIO0     GPIO4  // gpio pe4   dac pin 8
#define DAC_GPIO1     GPIO5
#define DAC_UNIBIPA   GPIO6






static void msleep(uint32_t x)
{
  // only works in a task thread... not main initialization thread
  vTaskDelay(pdMS_TO_TICKS(  x  )); // 1Hz
}




static int last = 0;

static void led_blink_task2(void *args __attribute((unused))) {

	for (;;) {

		gpio_toggle(LED_PORT, LED_OUT);

    uart_printf("hi %d %d %d\n\r",
      last++,
      gpio_get(DAC_PORT, DAC_GPIO0),
      gpio_get(DAC_PORT, DAC_GPIO1 )
    );

    msleep(500);
		// vTaskDelay(pdMS_TO_TICKS(  500  )); // 1Hz
		// vTaskDelay(pdMS_TO_TICKS(  100  )); // 10Hz
  }
}


// static void msleep(int

static void dac_test(void *args __attribute((unused))) {

  int i;
  for( i = 0; i < 10; ++i ) {
		gpio_toggle(LED_PORT, LED_OUT);
    msleep(50);
  }

  /*
  Reset input (active low). Logic low on this pin resets the input registers
  and DACs to the values RST67I defined by the UNI/BIP pins, and sets the Gain
  Register and Zero Register to defaultvalues.
  */
  gpio_clear(DAC_PORT, DAC_RST);
  msleep(50);
  gpio_set(DAC_PORT, DAC_RST);

  gpio_clear(DAC_PORT, DAC_LDAC);   // keep latch low, and unused, unless chaining

  gpio_clear(DAC_PORT_CS, DAC_CS);  // CS active low

  msleep(1);
  //gpio_clear(DAC_PORT, DAC_LDAC);   if we fall through... then we never
  // msleep(1);

  /*
  Writing a '1' to the GPIO-0 bit puts the GPIO-1 pin into a Hi-Zstate(default).
  DB8 GPIO-01 Writing a '0' to the GPIO-0 bit forces the GPIO-1 pin low

  p22 After a power-on reset or any forced hardware or software reset, all GPIO-n
  bits are set to '1', and the GPIO-n pin goes to a high-impedancestate.
  */

  spi_xfer(DAC_SPI, 0);
  spi_xfer(DAC_SPI, 0);
  spi_xfer(DAC_SPI, 0);
  // spi_xfer(DAC_SPI, 0xff );       // OK. working pa7, pin 4 dac
  // spi_xfer(DAC_SPI, 0b01010101 );

  msleep(1);

  gpio_set(DAC_PORT_CS, DAC_CS); // if ldac is low, then latch will latch on deselect cs.

  // gpio_clear(DAC_PORT, DAC_LDAC);

  // sleep forever
  // exiting a task thread isn't very good...
  for(;;) {
    msleep(1000);
  }
}


/*
  OK. check with scope that spi is emitting stuff.
  check RST works
  check that CS is
  check that latch gate is working as gpio
  check mosi to SDI
*/


static void dac_setup( void )
{

  // spi alternate function 5
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE,  DAC_CLK | DAC_MOSI | DAC_MISO );

  gpio_set_af(GPIOA, GPIO_AF5,  DAC_CLK | DAC_MOSI | DAC_MISO );

  rcc_periph_clock_enable(RCC_SPI1);
  spi_init_master(DAC_SPI,
    SPI_CR1_BAUDRATE_FPCLK_DIV_4,
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
    SPI_CR1_CPHA_CLK_TRANSITION_1,
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST);
  spi_enable_ss_output(DAC_SPI);
  spi_enable(DAC_SPI);


  // CS same pin as alternate function, but use as io.
  gpio_mode_setup(DAC_PORT_CS, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, DAC_CS);

  /////
  // other outputs
  gpio_mode_setup(DAC_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, DAC_LDAC | DAC_RST | DAC_UNIBIPA);


  // inputs, pullups work
  gpio_mode_setup(DAC_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, DAC_GPIO0 | DAC_GPIO1 );
}



// OK. do we have a sleep function for bit bashing...?

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


  // ok....
  xTaskCreate(dac_test,    "DAC_TEST",200,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */

	vTaskStartScheduler();

	for (;;);
	return 0;
}



