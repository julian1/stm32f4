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

/*
  p9 pin functions
  p16  asynchoroous interupt mode timing.
  p37 power up discussion
  p55 wakeup
  p55 unlock
  p82 for power on flowchart.


  problem...
    This pin must be set to one of three states at power-up.The pin stateis
    latchedat power-upand changingthe pin stateafterpower-uphas no effect

    The M0 pin settings(listedin Table12) are latchedon power-upto set the interface.
    not at reset?
  --------------

    NEEDS WAKEUP....?
    NEEDS TO BE unlocked.



  //////////////

  OK. crystal only comes up when digital and applied power...
  cannot see any current draw on bench supply.

  The reference source is selected by the INT_REFEN bit in the A_SYS_CFGregister.
  By default,the external reference is selected(INT_REFEN= 0).

  /////////////////
  So we should try to read.
  Or try to see if we are getting an interrupt. 

*/

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
  // spi_enable(ADC_SPI);



  uint32_t out = ADC_M0 | ADC_M1 | ADC_M2 | ADC_RESET;
  uint32_t in =  ADC_DRDY | ADC_DONE ;

  // should setup a reasonable state first...

  gpio_mode_setup(ADC_SPI_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, out );
  gpio_mode_setup(ADC_SPI_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, in );
}



/*
  - don't think it ever makes sense to do spi_read(), because hardware implies some value must
    be asserted on mosi while clocking and reading miso/din. even if high-Z.
  - similarly for write, there is always a value present on din/miso - even if high-Z/unconnected..
  so always use xfer. not spi_write() or spi_read()
*/

static uint32_t spi_xfer_24(uint32_t spi)
{
  spi_enable( spi );
  uint8_t a = spi_xfer(spi, 0);
  uint8_t b = spi_xfer(spi, 0);
  uint8_t c = spi_xfer(spi, 0);
  spi_disable( spi);

  return (a << 16) + (b << 8) + c;  // msb first. reading 3 registers gives us ff0400   eg. 
  // return (c << 16) + (b << 8) + a;
  // return  (b << 8) + a;
  // return  (a << 8) + b;
    // register 65284
    // == ff04
    // which is the value we are looking for.
}


static uint32_t spi_xfer_16(uint32_t spi)
{
  spi_enable( spi );
  uint8_t a = spi_xfer(spi, 0);
  uint8_t b = spi_xfer(spi, 0);
  spi_disable( spi);

  return  (a << 8) + b;
} 




static void adc_reset( void )
{

  gpio_set(ADC_SPI_PORT, ADC_M0);     // GND:Synchronousmastermode
                                        // IOVDD:Asynchronousinterruptmode
  
  gpio_clear(ADC_SPI_PORT, ADC_M1);     // SPI word transfersize GND:24 bit 
                                        // setting this may not work... after powerup. before reset.

  gpio_clear(ADC_SPI_PORT, ADC_M2);     // GND: Hamming code word validation off



  // reset
  gpio_clear(ADC_SPI_PORT, ADC_RESET);
  task_sleep(20);

  usart_printf("in    reset mcu drdy %d done %d\n", gpio_get(ADC_SPI_PORT, ADC_DRDY), gpio_get(ADC_SPI_PORT, ADC_DONE));
  
  gpio_set(ADC_SPI_PORT, ADC_RESET);
/*
  usart_printf("after reset mcu drdy %d done %d\n", gpio_get(ADC_SPI_PORT, ADC_DRDY), gpio_get(ADC_SPI_PORT, ADC_DONE));
  task_sleep(20);
  usart_printf("after sleep mcu drdy %d done %d\n", gpio_get(ADC_SPI_PORT, ADC_DRDY), gpio_get(ADC_SPI_PORT, ADC_DONE));
*/

  // ok this is pretty positive get data ready flag.

  // Monitor serial output for 0xFF02 (ADS131A02) or 0xFF04
  // (ADS131A04) 
  // wait for ready.
  uint32_t val =  0;
  do { 
    val = spi_xfer_16( ADC_SPI ); 
    usart_printf("register %x\r\n", val);
  }
  while(val != 0xff04 ) ;

  usart_printf("ok got ready\n");
}







static void test01(void *args __attribute((unused)))
{

  usart_printf("test01\n");
  usart_printf("adc reset\n");

  adc_reset();

#if 0
  usart_printf("adc reset done\n");

  usart_printf("mcu drdy %d done %d\n", gpio_get(ADC_SPI_PORT, ADC_DRDY), gpio_get(ADC_SPI_PORT, ADC_DONE));

  uint32_t x = spi_xfer_24();  // this is stalling?    // not enough stack?
//  usart_printf("x %d\n", x );

  usart_printf("register %d\r\n", spi_xfer_24());
  usart_printf("----\n" );
#endif

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


  xTaskCreate(test01,        "TEST01",1500,NULL,configMAX_PRIORITIES-2,NULL); // Lower priority

	vTaskStartScheduler();

	for (;;);
	return 0;
}

