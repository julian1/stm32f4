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
  https://www.ti.com/lit/ds/symlink/ads131a04.pdf

  p9  pin table functions
  p16 asynchoroous interupt mode timing.
  p31 data frame
  p37 power up discussion
  p55 wakeup
  p55 unlock
  p56 UNLOCKfromPORor RESET
  p82 flowchart.
  ---

  p56 RREG:Reada SingleRegister
  p57 WREG:WriteSingleRegister
  p65  A_SYS_CFGRegister

  problem...
    This pin must be set to one of three states at power-up.The pin stateis
    latchedat power-upand changingthe pin stateafterpower-uphas no effect

    The M0 pin settings(listedin Table12) are latchedon power-upto set the interface.
    not at reset?

  ------
  // Chip select(CS) is an active-lowinput that selects the device for SPI
    communication and controls the beginningand end of a dataframein
    asynchronousinterruptmode.

    The devicelatchesdataon DIN on the SCLKfallingedge  (MOSI)
    Data on DOUTare shiftedout on the SCLKrisingedge.    (MISO)


  --------------


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

  // rcc_periph_clock_enable(RCC_SPI2);
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


  // M1 high-Z. 16 bit.

  uint32_t out = ADC_M0 /* | ADC_M1*/ | ADC_M2 | ADC_RESET;
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


static uint32_t spi_xfer_16(uint32_t spi, uint16_t val)
{
  spi_enable( spi );
  uint8_t a = spi_xfer(spi, (val >> 8 ) & 0xff );
  uint8_t b = spi_xfer(spi, val & 0xff);
  spi_disable( spi);

  return (a << 8) + b;
}


static uint8_t adc_read_register(uint32_t spi, uint8_t r )
{
  /*
    Firstbyte:001a aaaa, where a aaaa is the register address
    Secondbyte:00h

    The response contains an 8-bit acknowledgment byte with the register
    address and an 8-bit data byte with the register content
  */
  uint32_t j = 1 << 5 | r;   // set bit 5 for read

  spi_xfer_16(spi, j << 8);
  uint32_t val = spi_xfer_16(spi, 0);

  if(val >> 8 != j  ) {
    usart_printf("bad acknowledgement address\n");
    // need something better....
    return -1;
  }

  return val & 0xff;
}



static uint8_t adc_write_register(uint32_t spi, uint8_t r, uint8_t val )
{
  /*
    Firstbyte: 010a aaaa, where a aaaa is the register address.
    Secondbyte: dddddddd, where dddddddd is the data to write to the address.

    The resulting command status response is a register read back from the
    updated register.
  */
  uint32_t j = 1 << 6 | r;   // set bit 6 to write

  spi_xfer_16(spi, j << 8 | val);
  uint32_t ret = spi_xfer_16(spi, 0);


  // return value is address or'd with read bit, and written val
  if(ret != ((1u << 5 | r) << 8 | val)) {
    usart_printf("bad write acknowledgement address or value\n");
    // need something better....
    return -1;
  }

  return val & 0xff;
}





static unsigned adc_reset( void )
{

  usart_printf("------------------\n");

  // GND:Synchronousmastermode
  // IOVDD:Asynchronousinterruptmode
  gpio_set(ADC_SPI_PORT, ADC_M0);

  // SPI word transfersize
  // GND:24 bit
  // No connection:16 bit
  // appears to be controllable on reset. not just power up, as indicated in datasheet.
  // gpio_set(ADC_SPI_PORT, ADC_M1);

  // GND: Hamming code word validation off
  gpio_clear(ADC_SPI_PORT, ADC_M2);



  // reset

  usart_printf("assert reset\n");
  gpio_clear(ADC_SPI_PORT, ADC_RESET);
  task_sleep(20);
  usart_printf("drdy %d done %d\n", gpio_get(ADC_SPI_PORT, ADC_DRDY), gpio_get(ADC_SPI_PORT, ADC_DONE));
  gpio_set(ADC_SPI_PORT, ADC_RESET);


  // ok this is pretty positive get data ready flag.

  /////////////////////////////////
  // Monitor serial output for ready. 0xFF02 (ADS131A02) or 0xFF04 (ADS131A04)

  usart_printf("wait for ready\n");
  uint32_t val =  0;
  do {
    val = spi_xfer_16( ADC_SPI, 0 );
    usart_printf("register %04x\n", val);
    task_sleep(20);
  }
  while(val != 0xff04) ;

  usart_printf("ok got ready\n");
  usart_printf("drdy %d\n", gpio_get(ADC_SPI_PORT, ADC_DRDY));




  /////////////////////////////////
  // unlock 0x0655

  spi_xfer_16( ADC_SPI, 0x0655);
  //while(gpio_get(ADC_SPI_PORT, ADC_DRDY));
  val = spi_xfer_16( ADC_SPI, 0 );
  // usart_printf("x here %04x\n", val);

  if(val != 0x0655) {
    usart_printf("unlock failed %4x\n", val);
    return -1;
  }


  task_sleep(20);
  usart_printf("register %04x\n", spi_xfer_16( ADC_SPI, 0));

  task_sleep(20);
  usart_printf("register %04x\n", spi_xfer_16( ADC_SPI, 0));


  /////////////////////////////////
  // write some registers

  usart_printf("--------\n");

#define A_SYS_CFG   0x0B


  usart_printf("val %02x\n", adc_read_register(ADC_SPI, A_SYS_CFG ));

  adc_write_register(ADC_SPI, A_SYS_CFG, 0x60 | (1 << 3) );     // configure internal ref.

  usart_printf("val now %02x\n", adc_read_register(ADC_SPI, A_SYS_CFG ));

  // shouldn't really be lo yet?.
  usart_printf("drdy %d\n", gpio_get(ADC_SPI_PORT, ADC_DRDY));


/*
  In fixed-framemode,thereare alwayssix devicewordsfor eachdataframefor the
ADS131A04.The first devicewordis reservedfor the statusword,the next four
devicewordsare reservedfor the conversiondatafor eachofthe four channels,and
the last wordis reservedfor the cyclicredundancycheck(CRC)dataword


0 : Devicew ords per data frame depends on whether the CRC and ADCs are enabled(default)
0 : CRC disabled(default)

*/

#define ADC_ENA  0x0F

  //adc_write_register(ADC_SPI, ADC_ENA, 0x01 );     // just one channel.
  adc_write_register(ADC_SPI, ADC_ENA, 0b1111 );     // just one channel.



  ////////////////////
  // wakeup

  spi_xfer_16( ADC_SPI, 0x0033);
  //while(gpio_get(ADC_SPI_PORT, ADC_DRDY));
  val = spi_xfer_16( ADC_SPI, 0 );
  // usart_printf("x here %04x\n", val);

  if(val != 0x0033) {
    usart_printf("wakeup failed %4x\n", val);
    return -1;
  } else
  {
    usart_printf("wakeup ok\n", val);
  }


  usart_printf("drdy %d\n", gpio_get(ADC_SPI_PORT, ADC_DRDY));


  while(true )
  {

    usart_printf("-----------\n");

    while(! gpio_get(ADC_SPI_PORT, ADC_DRDY)) 
    {
      // need 8 bytes?
      val = spi_xfer_16( ADC_SPI, 0 );
      usart_printf("x %d\n", val);
    }
  }

  


  return 0;
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

  usart_printf("register %d\n", spi_xfer_24());
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

