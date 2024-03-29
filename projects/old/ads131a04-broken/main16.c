/*
  ads131 using 16 bit word size for spi.
  this is not really useful, because no way to obtain the 24 bit value. 
  everything gets truncated.
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

  usart1_printf("adc setup spi\n");

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

  // spi_xfer(spi, 0 );  weird... this works...  but doesn't clear the spi clk error...
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
    usart1_printf("bad acknowledgement address\n");
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
    usart1_printf("bad write acknowledgement address or value\n");
    // need something better....
    return -1;
  }

  return val & 0xff;
}





static unsigned adc_reset( void )
{

  usart1_printf("------------------\n");

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

  usart1_printf("assert reset\n");
  gpio_clear(ADC_SPI_PORT, ADC_RESET);
  task_sleep(20);
  usart1_printf("drdy %d done %d\n", gpio_get(ADC_SPI_PORT, ADC_DRDY), gpio_get(ADC_SPI_PORT, ADC_DONE));
  gpio_set(ADC_SPI_PORT, ADC_RESET);


  // ok this is pretty positive get data ready flag.

  /////////////////////////////////
  // Monitor serial output for ready. 0xFF02 (ADS131A02) or 0xFF04 (ADS131A04)

  usart1_printf("wait for ready\n");
  uint32_t val =  0;
  do {
    val = spi_xfer_16( ADC_SPI, 0 );
    usart1_printf("register %04x\n", val);
    task_sleep(20);
  }
  while(val != 0xff04) ;

  usart1_printf("ok got ready\n");
  usart1_printf("drdy %d\n", gpio_get(ADC_SPI_PORT, ADC_DRDY));




  /////////////////////////////////
  // unlock 0x0655

  spi_xfer_16( ADC_SPI, 0x0655);
  //while(gpio_get(ADC_SPI_PORT, ADC_DRDY));
  val = spi_xfer_16( ADC_SPI, 0 );
  // usart1_printf("x here %04x\n", val);

  if(val != 0x0655) {
    usart1_printf("unlock failed %4x\n", val);
    return -1;
  }


  task_sleep(20);
  usart1_printf("register %04x\n", spi_xfer_16( ADC_SPI, 0));

  task_sleep(20);
  usart1_printf("register %04x\n", spi_xfer_16( ADC_SPI, 0));


  /////////////////////////////////
  // write some registers

  usart1_printf("--------\n");

#define A_SYS_CFG   0x0B


  usart1_printf("val %02x\n", adc_read_register(ADC_SPI, A_SYS_CFG ));

  adc_write_register(ADC_SPI, A_SYS_CFG, 0x60 | (1 << 3) );     // configure internal ref.

  usart1_printf("val now %02x\n", adc_read_register(ADC_SPI, A_SYS_CFG ));

  // shouldn't really be lo yet?.
  usart1_printf("drdy %d\n", gpio_get(ADC_SPI_PORT, ADC_DRDY));


/*
  In fixed-framemode,thereare alwayssix devicewordsfor eachdataframefor the
ADS131A04.The first devicewordis reservedfor the statusword,the next four
devicewordsare reservedfor the conversiondatafor eachofthe four channels,and
the last wordis reservedfor the cyclicredundancycheck(CRC)dataword


0 : Device words per data frame depends on whether the CRC and ADCs are enabled(default)
0 : CRC disabled(default)



The commandwordis the first devicewordon everyDIN dataframe.Thisframeis
reservedfor sendingusercommandsto writeor readfromregisters(seetheSPI
CommandDefinitionssection).The commandsare stand-alone,16-bitwordsthat appearin
the 16 mostsignificantbits (MSBs)of the first devicewordof the DIN
dataframe.Writezeroesto the remainingunusedleastsignificantbits
(LSBs)whenoperatingin either24-bitor 32-bitwordsize modes.

The contentsof the statuswordare always16 bits in lengthwith the
remainingLSBsset to zeroesdependingon the devicewordlength;see Table7

*/

#define ADC_ENA  0x0F

  // adc_write_register(ADC_SPI, ADC_ENA, 0x0 );     // no channel.
  adc_write_register(ADC_SPI, ADC_ENA, 0x01 );     // just one channel.
  // adc_write_register(ADC_SPI, ADC_ENA, 0b1111 );     // just one channel.
  // adc_write_register(ADC_SPI, ADC_ENA, 0x0f );     // all 4 channels



  ////////////////////
  // wakeup

  spi_xfer_16( ADC_SPI, 0x0033);
  //while(gpio_get(ADC_SPI_PORT, ADC_DRDY));
  val = spi_xfer_16( ADC_SPI, 0 );
  // usart1_printf("x here %04x\n", val);

  if(val != 0x0033) {
    usart1_printf("wakeup failed %4x\n", val);
    return -1;
  } else
  {
    usart1_printf("wakeup ok\n", val);
  }




#define STAT_1  0x02
#define STAT_P  0x03
#define STAT_N  0x04
#define STAT_S  0x05      // spi.

#define ERROR_CNT 0x06

  // ok. think it's indicating that one of the F_ADCIN N or P bits is at fault.bits   (eg. high Z. comparator).


  usart1_printf("error_cnt %d\n", adc_read_register(ADC_SPI, ERROR_CNT));

  usart1_printf("stat_1 %8b\n", adc_read_register(ADC_SPI, STAT_1));


  usart1_printf("stat_p %8b\n", adc_read_register(ADC_SPI, STAT_P));
  usart1_printf("stat_n %8b\n", adc_read_register(ADC_SPI, STAT_N));

  usart1_printf("stat_1 %8b\n", adc_read_register(ADC_SPI, STAT_1)); // re-read

  usart1_printf("stat_s %8b\n", adc_read_register(ADC_SPI, STAT_S));   // this should clear the value?????
  usart1_printf("stat_1 %8b\n", adc_read_register(ADC_SPI, STAT_1)); // re-read


  usart1_printf("drdy %d\n", gpio_get(ADC_SPI_PORT, ADC_DRDY));
  usart1_printf("-----------\n");


  /*

  SPI fault.This bit indicatesthat one of the statusbits in the STAT_Sregisteris set.Readthe STAT_Sregisterto clearthe bit
  */

  // status word is 0x2230
  // 0x22 == 00100010  ?
  // 0x30 is the value of stat_1

  // word is now 0x2220... after connecting up input voltage.  eg. no ovc set.
  // eg. only bit 5 error is set.


  // so may need to put it on an interrupt... to immediately pull the data out...

  uint32_t spi = ADC_SPI;
  do
  {

    while(gpio_get(ADC_SPI_PORT, ADC_DRDY));   // wait for drdy to go lo
    spi_enable( spi );
    for(unsigned j = 0; j < 4; ++j)
    {
      uint8_t a = spi_xfer(spi, 0);
      usart1_printf("%x\n", a);
      //usart1_printf("%8b\n", a);
    }
    spi_disable( spi );


  // perhaps adc_read_register()... itself is not providing enough cycles?

    // usart1_printf("stat_1 %8b\n", adc_read_register(ADC_SPI, STAT_1)); // re-read
    usart1_printf("stat_s %8b\n", adc_read_register(ADC_SPI, STAT_S)); // F_FRAME,  not enough sclk values...
                                                                      // or maybe we just aren't reading fast enough...
                                                                      // IMPORTANT. happens before read data .
                                                                      // so must check when initializing.
                                                                      // but get 24bit word working first.
    usart1_printf("-\n");
    // break;
  } while(false);




  return 0;
}







static void test01(void *args __attribute((unused)))
{

  usart1_printf("test01\n");
  usart1_printf("adc reset\n");

  adc_reset();

#if 0
  usart1_printf("adc reset done\n");

  usart1_printf("mcu drdy %d done %d\n", gpio_get(ADC_SPI_PORT, ADC_DRDY), gpio_get(ADC_SPI_PORT, ADC_DONE));

  uint32_t x = spi_xfer_24();  // this is stalling?    // not enough stack?
//  usart1_printf("x %d\n", x );

  usart1_printf("register %d\n", spi_xfer_24());
  usart1_printf("----\n" );
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
  usart1_setup();

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

