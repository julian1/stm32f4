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


  // M1 high-Z. is 16 bit word.
  uint32_t out = ADC_M0  | ADC_M1 | ADC_M2 | ADC_RESET;
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


#if 0
static uint32_t spi_xfer_16(uint32_t spi, uint16_t val)
{
  spi_enable( spi );
  uint8_t a = spi_xfer(spi, (val >> 8 ) & 0xff );
  uint8_t b = spi_xfer(spi, val & 0xff);

  // spi_xfer(spi, 0 );  weird... this works...  but doesn't clear the spi clk error...
  spi_disable( spi);

  return (a << 8) + b;
}
#endif



static uint32_t spi_xfer_24_whoot(uint32_t spi, uint32_t val)
{
  uint8_t a = spi_xfer(spi, (val >> 16) & 0xff );
  uint8_t b = spi_xfer(spi, (val >> 8) & 0xff);
  uint8_t c = spi_xfer(spi, val & 0xff);

  return (a << 16) + (b << 8) + c;
}

static uint32_t spi_xfer_24(uint32_t spi, uint32_t val)
{
  spi_enable( spi );
  uint32_t ret = spi_xfer_24_whoot(spi, val);
  
  // spi_xfer(spi, 0 ); // dummy

  spi_disable( spi);
  return ret;
}


static uint16_t spi_xfer_24_16(uint32_t spi, uint16_t val)
{
  // encode 16 bit values in 24 bit word size, and return shifted value

  return spi_xfer_24( spi , val << 8) >> 8;
}


//////////////////



static uint32_t adc_send_code(uint32_t spi, /*uint16_t */ uint32_t val)
{
  spi_xfer_24_16( spi, val );
  val = spi_xfer_24_16( ADC_SPI, 0 );
  return val;
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

  uint32_t val = adc_send_code(spi, j << 8 );

  if(val >> 8 != j) {
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

  uint32_t ret =  adc_send_code(spi, j << 8 | val);

  // return value is address or'd with read bit, and written val
  if(ret != ((1u << 5 | r) << 8 | val)) {
    usart_printf("bad write acknowledgement address or value\n");
    // need something better....
    return -1;
  }

  return val & 0xff;
}



uint32_t sign_extend_24_32(uint32_t x) 
{
  // https://stackoverflow.com/questions/42534749/signed-extension-from-24-bit-to-32-bit-in-c
  const int bits = 24;
  uint32_t m = 1u << (bits - 1);
  return (x ^ m) - m;
}


static unsigned adc_reset( void )
{
  // change name. configure.

  usart_printf("------------------\n");

  // GND:Synchronousmastermode
  // IOVDD:Asynchronousinterruptmode
  gpio_set(ADC_SPI_PORT, ADC_M0);

  // SPI word transfersize
  // GND:24 bit
  // No connection:16 bit
  // appears to be controllable on reset. not just power up, as indicated in datasheet.
  gpio_clear(ADC_SPI_PORT, ADC_M1);

  // GND: Hamming code word validation off
  gpio_clear(ADC_SPI_PORT, ADC_M2);


  ////////////
  // reset
  usart_printf("assert reset\n");
  gpio_clear(ADC_SPI_PORT, ADC_RESET);
  task_sleep(20);
  usart_printf("drdy %d done %d\n", gpio_get(ADC_SPI_PORT, ADC_DRDY), gpio_get(ADC_SPI_PORT, ADC_DONE));
  gpio_set(ADC_SPI_PORT, ADC_RESET);


  uint32_t spi = ADC_SPI;


  // ok this is pretty positive get data ready flag.

  /////////////////////////////////
  // Monitor serial output for ready. 0xFF02 (ADS131A02) or 0xFF04 (ADS131A04)

  usart_printf("wait for ready\n");
  uint32_t val = 0;
  do {
    val = spi_xfer_24_16( spi, 0 );
    usart_printf("register %04x\n", val);
    task_sleep(20);
  }
  while(val != 0xff04) ;

  usart_printf("ok got ready\n");
  usart_printf("drdy %d\n", gpio_get(ADC_SPI_PORT, ADC_DRDY));


#define UNLOCK  0x0655
#define LOCK    0x0555
#define WAKEUP  0x0033


  /////////////////////////////////
  // unlock 0x0655


  val = adc_send_code(spi, UNLOCK);
  if(val != UNLOCK) {
    usart_printf("unlock failed %4x\n", val);
    return -1;
  } else {
    usart_printf("unlock ok\n");
  }


  task_sleep(20);
  usart_printf("register %04x\n", spi_xfer_24_16( spi, 0));

  task_sleep(20);
  usart_printf("register %04x\n", spi_xfer_24_16( spi, 0));


  /////////////////////////////////
  // write some registers

  usart_printf("--------\n");



// change prefix SR ?
#define STAT_1    0x02
#define STAT_P    0x03
#define STAT_N    0x04
#define STAT_S    0x05      // spi.

#define ERROR_CNT 0x06

#define A_SYS_CFG 0x0B
#define D_SYS_CFG 0x0C


#define ADC_ENA   0x0F

  // read a_sys_cfg
  uint8_t a_sys_cfg = adc_read_register(spi, A_SYS_CFG );
  // usart_printf("a_sys_cfg %2x\n", a_sys_cfg);
  usart_printf("a_sys_cfg %8b\n", a_sys_cfg);
  if(a_sys_cfg != 0x60) {
    usart_printf("a_sys_cfg not expected default\n");
    return -1;
  }

  // change
  adc_write_register(spi, A_SYS_CFG, a_sys_cfg | (1 << 3) );     // configure internal ref.

  usart_printf("a_sys_cfg now %02x\n", adc_read_register(spi, A_SYS_CFG ));




  // read d_sys_cfg
  uint8_t d_sys_cfg = adc_read_register(spi, D_SYS_CFG );
  // usart_printf("d_sys_cfg %02x\n", d_sys_cfg);
  usart_printf("d_sys_cfg %8b\n", d_sys_cfg);
  if(d_sys_cfg != 0x3c) {
    usart_printf("d_sys_cfg not expected default\n");
    return -1;
  }




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


// ok. stat_1 and stat_s is clear at this point...
#if 0
  usart_printf("here0\n");
  usart_printf("-------\nhere0\n");
  usart_printf("stat_1 %8b\n", adc_read_register(spi, STAT_1));
  usart_printf("stat_p %8b\n", adc_read_register(spi, STAT_P));
#endif


  /*
    as soon as we enable adc, then we get an error ...
    because it starts generating data - that we don't consume ... 
    that we didn't read a produced value properly, eg. because of encoding or not enough time. etc...
    not that there is a problem with our 
  */

  // adc_write_register(spi, ADC_ENA, 0x0 );     // no channel.
                                              // setting

  adc_write_register(spi, ADC_ENA, 0x01 );     // just one channel.
  // adc_write_register(spi, ADC_ENA, 0b1111 );     // just one channel.
  // adc_write_register(spi, ADC_ENA, 0x0f );     // all 4 channels

  /*
  // hang on.... 
  // DRDY - if we configure an interrupt - then that will consume data - while 
  // we are trying to do the register writing.
  // we may have to have manual disable interupt - when we are writing and reading values...

    VERY IMPORTANT NO. only configure the interupt to consume as last thing we do...
    then if want to unlock and change registers, we would disable. etc.
  */

  // just using a while() loop without NSS/CS will not consume data.

#if 0
  // while(true) {
  usart_printf("-------\nhere1\n");
  usart_printf("stat_1 %8b\n", adc_read_register(spi, STAT_1));
  usart_printf("stat_p %8b\n", adc_read_register(spi, STAT_P));
  usart_printf("stat_p %8b\n", adc_read_register(spi, STAT_P));
  usart_printf("stat_1 %8b\n", adc_read_register(spi, STAT_1));
  //}
#endif





  ////////////////////
  // wakeup
  val = adc_send_code(spi, WAKEUP);
  if(val != WAKEUP) {
    usart_printf("wakeup failed %4x\n", val);
    return -1;
  } else
  {
    usart_printf("wakeup ok\n", val);
  }



  ////////////////////
  // lock again
  val = adc_send_code(spi, LOCK);
  if(val != LOCK) {
    usart_printf("lock failed %4x\n", val);
    return -1;
  } else
  {
    usart_printf("lock ok\n", val);
  }



    

 
  // ok. think it's indicating that one of the F_ADCIN N or P bits is at fault.bits   (eg. high Z. comparator).


  usart_printf("error_cnt %d\n", adc_read_register(spi, ERROR_CNT));

  usart_printf("stat_1 %8b\n", adc_read_register(spi, STAT_1));


  usart_printf("stat_p %8b\n", adc_read_register(spi, STAT_P));
  usart_printf("stat_n %8b\n", adc_read_register(spi, STAT_N));

  usart_printf("stat_1 %8b\n", adc_read_register(spi, STAT_1)); // re-read

  usart_printf("stat_s %8b\n", adc_read_register(spi, STAT_S));   // this should clear the value?????
  usart_printf("stat_s %8b\n", adc_read_register(spi, STAT_S));   // this should clear the value?????
  // usart_printf("stat_1 %8b\n", adc_read_register(spi, STAT_1)); // re-read


  usart_printf("drdy %d\n", gpio_get(ADC_SPI_PORT, ADC_DRDY));
  usart_printf("-----------\n");


  /*

  SPI fault.This bit indicatesthat one of the statusbits in the STAT_Sregisteris set.Readthe STAT_Sregisterto clearthe bit
  */

  // status word is 0x2230
  // 0x22 == 00100010  ?
  // 0x30 is the value of stat_1

  // word is now 0x2220... after connecting up input voltage.  eg. no ovc set.
  // eg. only bit 5 error is set.

  // so may need to put it on an interrupt... to immediately pull the data out...

  // So. think we need to setup the interupt...
  // we can probably print from the interupt, if really want.
  // or enque the result. to print in a separate task.
  // and select.

  do
  {

    while(gpio_get(ADC_SPI_PORT, ADC_DRDY));   // wait for drdy to go lo
                                                // should already be set.

    spi_enable(spi);

    // get status code
    uint32_t code = spi_xfer_24_whoot(spi, 0) >> 8;
    // usart_printf("code %x\n", code);

    // negative value isn't working
    // get adc readings
    for(unsigned j = 0; j < 1; ++j)
    {
      int32_t x = spi_xfer_24_whoot(spi, 0);

      x = sign_extend_24_32(x );

      // usart_printf("%2x\n", val);
      usart_printf("%d\n", x);

    }
    spi_disable( spi );

    // stronger nss pullup?

    // perhaps adc_read_register()... itself is not providing enough cycles?
    // usart_printf("stat_1 %8b\n", adc_read_register(spi, STAT_1)); // re-read
    // usart_printf("stat_s %8b\n", adc_read_register(spi, STAT_S)); // F_FRAME,  not enough sclk values...
                                                                      // or maybe we just aren't reading fast enough...
                                                                      // IMPORTANT. happens before read data .
                                                                      // so must check when initializing.
                                                                      // but get 24bit word working first.
    // usart_printf("-\n");

  } while(false);




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

