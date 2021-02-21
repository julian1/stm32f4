/*

  EXTREME
    we can use ina154 with bjt in loop.  just use feedback/ref pin.

  ---------
  simple led blink task,
  and answer on uart.

*/


#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"   // JA


#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>

#include <libopencm3/stm32/timer.h>


#include <libopencm3/cm3/scb.h>


#include <math.h>


//////////

#include "sleep.h"
#include "usart.h"
#include "serial.h"



#define LED_PORT  GPIOE
#define LED_OUT   GPIO15



void vApplicationTickHook( void )
{
}

void vApplicationIdleHook( void )
{
}



void vApplicationStackOverflowHook( TaskHandle_t xTask, char * pcTaskName )
{
  (void) xTask;
  (void) pcTaskName;

  while(true)
  {
		gpio_toggle(LED_PORT,LED_OUT);

    for(unsigned i = 0; i < 2 * 1000000; ++i) {
      __asm__("nop");
    }
  }
}

void vApplicationMallocFailedHook(void)
{
  while(true)
  {
		gpio_toggle(LED_PORT,LED_OUT);

    for(unsigned i = 0; i < 2 * 1000000; ++i) {
      __asm__("nop");
    }
  }


}





static void adc_exti_setup(void);


static void led_task(void *args __attribute((unused))) {

	for (;;) {
		gpio_toggle(LED_PORT,LED_OUT);
		vTaskDelay(pdMS_TO_TICKS(500)); // 1Hz
	}

/*
  //
  TickType_t xTimeNow;
  xTimeNow = xTaskGetTickCount();
*/
}


// prvTimerCallback


static void led_setup(void)
{
  gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_OUT); // JA - move to function led_setup.
}



////////////////
///  ads131a04
#define ADC_GPIO_PORT GPIOB    // change name ADC_GPIO_PORT
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
  gpio_mode_setup(ADC_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, all);

  // OK.. THIS MADE SPI WORK AGAIN....
  // note, need harder edges for signal integrity. or else different speed just helps suppress parasitic components
  // see, https://www.eevblog.com/forum/microcontrollers/libopencm3-stm32l100rc-discovery-and-spi-issues/
  gpio_set_output_options(ADC_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, all);

  // af 5 for spi2 on PB
  gpio_set_af(ADC_GPIO_PORT, GPIO_AF5, all);

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

  gpio_mode_setup(ADC_GPIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, out );
  gpio_mode_setup(ADC_GPIO_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, in );
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



static uint32_t sign_extend_24_32(uint32_t x)
{
  // https://stackoverflow.com/questions/42534749/signed-extension-from-24-bit-to-32-bit-in-c
  const int bits = 24;
  uint32_t m = 1u << (bits - 1);
  return (x ^ m) - m;
}


static unsigned adc_reset( void )
{
  // rename. adc_configure.  actually not sure...

  usart_printf("------------------\n");

  // GND:Synchronousmastermode
  // IOVDD:Asynchronousinterruptmode
  gpio_set(ADC_GPIO_PORT, ADC_M0);

  // SPI word transfersize
  // GND:24 bit
  // No connection:16 bit
  // appears to be controllable on reset. not just power up, as indicated in datasheet.
  gpio_clear(ADC_GPIO_PORT, ADC_M1);

  // GND: Hamming code word validation off
  gpio_clear(ADC_GPIO_PORT, ADC_M2);


  ////////////
  // reset
  usart_printf("assert reset\n");
  gpio_clear(ADC_GPIO_PORT, ADC_RESET);
  task_sleep(20);
  usart_printf("drdy %d done %d\n", gpio_get(ADC_GPIO_PORT, ADC_DRDY), gpio_get(ADC_GPIO_PORT, ADC_DONE));
  gpio_set(ADC_GPIO_PORT, ADC_RESET);


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
  usart_printf("drdy %d\n", gpio_get(ADC_GPIO_PORT, ADC_DRDY));


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


#define CLK2      0x0E
#define ADC_ENA   0x0F




  //////////////////////
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




  //////////////////////
  // read d_sys_cfg
  uint8_t d_sys_cfg = adc_read_register(spi, D_SYS_CFG );
  // usart_printf("d_sys_cfg %02x\n", d_sys_cfg);
  usart_printf("d_sys_cfg %8b\n", d_sys_cfg);
  if(d_sys_cfg != 0x3c) {
    usart_printf("d_sys_cfg not expected default\n");
    return -1;
  }


  //////////////////////
  // clk2 
  uint8_t clk2 = adc_read_register(spi, CLK2 );
  // usart_printf("clk2 %2x\n", clk2);
  usart_printf("clk2 %8b\n", clk2); // 10000110
  if(clk2 != 0x86) {
    usart_printf("clk2 not expected default\n");
    return -1;
  }

  // set OSR to max
  adc_write_register(spi, CLK2, clk2 & (0b1111 << 4)  );    // clear lower 4 bits, for max OSR
                                                            // better way to do this?

  usart_printf("clk2 now %8b\n", adc_read_register(spi, CLK2 ));


 



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
  // lock
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


  usart_printf("drdy %d\n", gpio_get(ADC_GPIO_PORT, ADC_DRDY));
  usart_printf("-----------\n");


  /*

  SPI fault.This bit indicatesthat one of the statusbits in the STAT_Sregisteris set.Readthe STAT_Sregisterto clearthe bit

  blocking uart/serial - means we miss frames?

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

#if 0
  do
  {

    while(gpio_get(ADC_GPIO_PORT, ADC_DRDY));   // wait for drdy to go lo
                                                // should already be set.

    spi_enable(spi);

    // get status code
    uint32_t code = spi_xfer_24_whoot(spi, 0) >> 8;
    (void) code;
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
#endif


/*
  OK. doing the exti setup causes everything to stall...
*/

#if 1
  // set up interrupt
  adc_exti_setup();
#endif
  return 0;
}


/////////////////////////////////////////

/*
  
*/

static void adc_exti_setup(void)
{
  // return;
  nvic_enable_irq(NVIC_EXTI15_10_IRQ);

  nvic_set_priority(NVIC_EXTI15_10_IRQ, 5 );


  exti_select_source(EXTI10, ADC_GPIO_PORT);
  exti_set_trigger(EXTI10, EXTI_TRIGGER_FALLING);
  exti_enable_request(EXTI10);
}

/*
// should try external ref.
// should try battery powered 5V ref - when have opamp input. maybe.
// should try 50Hz interval

*/


// because freertos - uses the interrupt also?

// Do, we WE HAVE TO DISAMBIGUATE THE CALLER???? 

void exti15_10_isr(void)
{

  exti_reset_request(EXTI10);
  return;

  ////////////////
  uint32_t spi = ADC_SPI;
  static uint32_t count = 0;

  int32_t x = 0;
  double y;

  // do read...
  spi_enable(spi);
  // get status code
  uint32_t code = spi_xfer_24_whoot(spi, 0) >> 8;     // this is just the 24_16 call... except without
                                                      // the cs.   Should be able to refactor to avoid needing
                                                      // separate extra function.
  (void)code;
  // get values
  for(unsigned j = 0; j < 1; ++j)
  {
    x = spi_xfer_24_whoot(spi, 0);
    x = sign_extend_24_32(x );
                   // -3454153
    y = ((double )x) / 3451766.f;
  }
  spi_disable( spi );


  if(count++ % 100 == 0) {

    // nat
    // usart_printf("%d\n", x);

    /*
      EXTREME....
      HERE
      OK.
      trying to do a print statement here... 
      is what kills it.
    */ 
    // usart_printf("%f\n",  y   );

#if 0
    if(adc_read_register(spi, STAT_S) != 0) {
      usart_printf("!");
    }
#endif

#if 0
    usart_printf("code   %8b\n", code );
    usart_printf("stat_1 %8b\n", adc_read_register(spi, STAT_1)); // re-read
    usart_printf("stat_s %8b\n", adc_read_register(spi, STAT_S)); // this should clear the value?????
#endif
  }

}

/////////////////////////////////////





static void test01_task(void *args __attribute((unused)))
{
#if 0
  static int count = 0; 

  while(true) {
    usart_printf("test01_task %u %f %f\n", count, (float)count / 100., sqrtf( count ) );
    ++count;
  }
#endif

#if 1
  usart_printf("test01_task\n");
  usart_printf("adc reset\n");

  adc_reset();

  usart_printf("adc reset done %f\n", sqrtf(123.456) );

#if 0
  usart_printf("adc reset done\n");

  usart_printf("mcu drdy %d done %d\n", gpio_get(ADC_GPIO_PORT, ADC_DRDY), gpio_get(ADC_GPIO_PORT, ADC_DONE));

  uint32_t x = spi_xfer_24();  // this is stalling?    // not enough stack?
//  usart_printf("x %d\n", x );

  usart_printf("register %d\n", spi_xfer_24());
  usart_printf("----\n" );
#endif

#endif
  // sleep forever
  for(;;) {
    usart_printf(".");
    task_sleep(1000);

  }
}







#if 1
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

  rcc_periph_clock_enable(RCC_SYSCFG); // needed for external interupts.



#if 1
 /*
  http://www.freertos.org/RTOS-Cortex-M3-M4.html
  Preempt priority and subpriority:
   If you are using an STM32 with the STM32 driver library then ensure all the
   priority bits are assigned to be preempt priority bits by calling
   NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 ); before the RTOS is started.
 */

  scb_set_priority_grouping(SCB_AIRCR_PRIGROUP_GROUP16_NOSUB);
#endif



  ///////////////
  led_setup();
  usart_setup();

  //timer_setup();  // software timer 

  ///////////////

  adc_setup_spi();



  // OK. but it's stalling....
  // led is ok.
  // uart can print some stuff
  // try to type into terminal and it freezes.
  
  // xPSR: 0x21000035 pc: 0x080019d4 msp: 0x2001ffb8

  xTaskCreate(led_task,  "LED",1000,NULL,configMAX_PRIORITIES-1,NULL);

  // IMPORTANT changing from 100 to 200, stops deadlock
  xTaskCreate(usart_task,        "UART",1000,NULL,configMAX_PRIORITIES-1,NULL); /* Highest priority */

  xTaskCreate(serial_prompt_task,"SERIAL2",1000,NULL,configMAX_PRIORITIES-2,NULL); /* Lower priority */


  xTaskCreate(test01_task,        "TEST01",3500,NULL,configMAX_PRIORITIES-2,NULL); // Lower priority

	vTaskStartScheduler();

	for (;;);
	return 0;
}
#endif
