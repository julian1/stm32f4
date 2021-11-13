

#include <libopencm3/stm32/spi.h>

#include "ads131a04.h"

#include "bits.h"
// #include "format_bits.h"
#include "util.h"   // usart_printf
#include "str.h"   // format_bits()

#include "mux.h"
#include "reg.h"





void spi_adc_setup(uint32_t spi)
{
  // rcc_periph_clock_enable(RCC_SPI2);
  spi_init_master(
    spi,
    SPI_CR1_BAUDRATE_FPCLK_DIV_4,
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
    SPI_CR1_CPHA_CLK_TRANSITION_2,    // 2 == falling edge
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST
  );

  spi_disable_software_slave_management(spi);
  spi_enable_ss_output(spi);
}


static uint32_t spi_xfer_24(uint32_t spi, uint32_t val)
{
  uint8_t a = spi_xfer(spi, (val >> 16) & 0xff );
  uint8_t b = spi_xfer(spi, (val >> 8) & 0xff);
  uint8_t c = spi_xfer(spi, val & 0xff);

  return (a << 16) + (b << 8) + c;
}


static uint16_t spi_xfer_24_16(uint32_t spi, uint16_t val)
{
  // encode 16 bit values in 24 bit word size, and return similarly shifted value
  return spi_xfer_24(spi, val << 8) >> 8;
}


static uint16_t spi_xfer_24_16_cs(uint32_t spi, uint16_t val)
{
  spi_enable( spi );
  uint32_t ret = spi_xfer_24_16(spi, val);
  spi_disable( spi);
  return ret;
}


//////////////////



static uint32_t adc_send_code(uint32_t spi, /*uint16_t */ uint32_t val)
{
  spi_xfer_24_16_cs( spi, val );
  // val = spi_xfer_24_16_cs( ADC_SPI, 0 );   // JA... changed...
  val = spi_xfer_24_16_cs( spi, 0 );
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
  // TODO maybe move this somewhere general
  // https://stackoverflow.com/questions/42534749/signed-extension-from-24-bit-to-32-bit-in-c
  const int bits = 24;
  uint32_t m = 1u << (bits - 1);
  return (x ^ m) - m;
}



// TODO prefix SR_
#define UNLOCK  0x0655
#define LOCK    0x0555
#define WAKEUP  0x0033




// TODO add/change prefix SR_ ?
// check datasheet
#define STAT_1    0x02      // general?
#define STAT_P    0x03      // positive
#define STAT_N    0x04      // negative
#define STAT_S    0x05      // spi.
#define ERROR_CNT 0x06

#define A_SYS_CFG 0x0B
#define D_SYS_CFG 0x0C


#define CLK1      0x0D  // default 0x08
#define CLK2      0x0E  // default 0x86
#define ADC_ENA   0x0F




static void clear_status_registers(uint32_t spi)
{
  // which regs need to be read???

  adc_read_register(spi, STAT_1);

  adc_read_register(spi, STAT_P);
  adc_read_register(spi, STAT_N);
  adc_read_register(spi, STAT_S);   // this should clear the value?????
}



static void adc_print_status_registers(uint32_t spi)
{

  char buf[100];


  usart_printf(" stat_1 %s\n",  format_bits(buf, 8, adc_read_register(spi, STAT_1)));
  usart_printf(" stat_p %s\n", format_bits(buf, 8, adc_read_register(spi, STAT_P)));
  usart_printf(" stat_n %s\n", format_bits(buf, 8, adc_read_register(spi, STAT_N)));
  usart_printf(" stat_s %s\n", format_bits(buf, 8, adc_read_register(spi, STAT_S)));   // this should clear the value?????

  usart_printf(" error_cnt %d\n", adc_read_register(spi, ERROR_CNT));


  // usart_printf(" stat_1 %s\n", format_bits(buf, 8, adc_read_register(spi, STAT_1))); // re-read
  //usart_printf(" stat_s %s\n", format_bits(buf, 8, adc_read_register(spi, STAT_S)));   // this should clear the value?????
  // usart_printf("stat_1 %s\n", format_bits(buf, 8, adc_read_register(spi, STAT_1))); // re-read


/*
  usart_printf("drdy %d\n", gpio_get(ADC_GPIO_PORT, ADC_DRDY));
  usart_printf("-----------\n");
*/
}



  // keep latch low, and unused, unless chaining


int adc_reset( uint32_t spi, uint8_t reg)
{
  /* in a fault condition. to avoid interupt generating supriou
    only callable after 
  */

  mux_fpga(spi);


  ////////////
  // reset
  usart_printf("adc assert reset\n");
  reg_clear(spi, reg, ADC_RST);

  usart_printf("adc before msleep\n");
  msleep(20);
  usart_printf("adc after msleep\n");

  return 0;
}





int adc_init( uint32_t spi, uint8_t reg)
{

  usart_printf("------------------\n");
  usart_printf("ads131a04 init\n");


  mux_fpga(spi);

  // GND:Synchronousmastermode
  // IOVDD:Asynchronousinterruptmode
  // gpio_set(ADC_GPIO_PORT, ADC_M0);
  reg_set(spi, reg, ADC_M0);

  // SPI word transfersize
  // GND:24 bit
  // No connection:16 bit
  // appears to be controllable on reset. not just power up, as indicated in datasheet.
  // gpio_clear(ADC_GPIO_PORT, ADC_M1);
  reg_clear(spi, reg, ADC_M1);

  // GND: Hamming code word validation off
  // gpio_clear(ADC_GPIO_PORT, ADC_M2);
  reg_clear(spi, reg, ADC_M2);


  ////////////
  // reset
  usart_printf("assert reset\n");
  // gpio_clear(ADC_GPIO_PORT, ADC_RESET);
  reg_clear(spi, reg, ADC_RST);

  usart_printf("before msleep\n");
  msleep(20);
  usart_printf("after msleep\n");


  // usart_printf("drdy %d done %d\n", gpio_get(ADC_GPIO_PORT, ADC_DRDY), gpio_get(ADC_GPIO_PORT, ADC_DONE));
  // gpio_set(ADC_GPIO_PORT, ADC_RESET);
  reg_set(spi, reg, ADC_RST);
  msleep(20);


  //////////////////////////////////
  // everything below here. is spi.

  // ok this is pretty positive get data ready flag.

  /////////////////////////////////
  // Monitor serial output for ready. 0xFF02 (ADS131A02) or 0xFF04 (ADS131A04)

  usart_printf("wait for ready\n");
  // return 0;

  mux_adc(spi);

  uint32_t val = 0;
  do {
    val = spi_xfer_24_16_cs( spi, 0 );
    usart_printf("register %04x\n", val);
    msleep(20);
    // adc is returning 0000. with only digital power.
    // but doesn't need xtal, to respond to spi.
  }
  while(val != 0xff04) ;

  usart_printf("ok got ready %04x\n", val);
  // usart_printf("drdy %d\n", gpio_get(ADC_GPIO_PORT, ADC_DRDY));



  /////////////////////////////////
  // unlock 0x0655


  val = adc_send_code(spi, UNLOCK);
  if(val != UNLOCK) {
    usart_printf("unlock failed %4x\n", val);
    return -1;
  } else {
    usart_printf("unlock ok\n");
  }


  // msleep(20);
  // usart_printf("register %04x\n", spi_xfer_24_16_cs( spi, 0));

  msleep(20);
  usart_printf("register %04x\n", spi_xfer_24_16_cs( spi, 0));


  /////////////////////////////////
  // write some registers

  // usart_printf("--------\n");

  char buf[100];

  //////////////////////
  // read a_sys_cfg
  uint8_t a_sys_cfg = adc_read_register(spi, A_SYS_CFG );
  // usart_printf("a_sys_cfg %2x\n", a_sys_cfg);
  usart_printf("a_sys_cfg %s\n", format_bits(buf, 8, a_sys_cfg));
  if(a_sys_cfg != 0x60) {
    usart_printf("a_sys_cfg not expected default\n");
    return -1;
  } else {
    usart_printf("a_sys_cfg val ok\n");
  }



  // change
  adc_write_register(spi, A_SYS_CFG, a_sys_cfg | (1 << 3) );     // configure internal ref.

  usart_printf("a_sys_cfg now %02x\n", adc_read_register(spi, A_SYS_CFG ));


#if 1


  //////////////////////
  // read d_sys_cfg
  uint8_t d_sys_cfg = adc_read_register(spi, D_SYS_CFG );
  // usart_printf("d_sys_cfg %02x\n", d_sys_cfg);
  usart_printf("d_sys_cfg %s\n", format_bits(buf, 8, d_sys_cfg));
  if(d_sys_cfg != 0x3c) {
    usart_printf("d_sys_cfg not expected default\n");
    return -1;
  }


/*
  NPLC 1 or greater. then we reject power-line noise.
  less than 1 and we do not.
    https://www.youtube.com/watch?v=ld3ju6EOdZA

  --------------
  // eg. with 16.384 MHz xtal
  // 16384000 / 4096 / 8 /8 =  62.5
  // 16384000 / 4096 / 10 /8 = 50Hz

  // with ice40 divider
  // min on-die divider is 2
  // eg. 2 * 2 * 4096 * 3000 = 49152000
  float u = 49152000  / 4096 / ia[i] / ia[j] / m ;        // works i=2 j=2  m=60 f=50.000000,   i=2 j=2  m=50 f=60.000000
  // float u = 24576000/ 4096 / ia[i] / ia[j] / m ;       // works i=2 j=2  m=30 f=50.000000,   i=2 j=2  m=25 f=60.000000
  ie.  4.9152MHz

  // using just the ads dividers
  // (50 * 60  * 4096 * 2) =  24576000   = 24.576MHz
  // float u = (50 * 60  * 4096 * 2)  / 4096 / ia[i] / ia[j];    // works for both 50 and 60hz.
    10 10 60.000000
    10 12 50.000000


*/


  //////////////////////
  // clk1
  uint8_t clk1 = adc_read_register(spi, CLK1);
  usart_printf("clk1 %s\n", format_bits(buf, 8, clk1));
  if(clk1 != 0x08) {
    usart_printf("clk1 not expected default\n");
    return -1;
  }

#define CLK_DIV_WIDTH  3
#define CLK_DIV_OFF    1
// #define CLK_DIV_VAL 4      // 0b100 // default== 4 // 100 : fICLK = fCLKIN / 8 (default)
#define CLK_DIV_VAL    5      // 101 : fICLK = fCLKIN / 10

  // usart_printf("clk_div val %u\n", GETFIELD(clk1, CLK_DIV_WIDTH, CLK_DIV_OFF));
  clk1 = SETFIELD(clk1, CLK_DIV_WIDTH, CLK_DIV_OFF, CLK_DIV_VAL);

  adc_write_register(spi, CLK1, clk1);


  //////////////////////
  // clk2
  uint8_t clk2 = adc_read_register(spi, CLK2);
  // usart_printf("clk2 %2x\n", clk2);
  usart_printf("clk2 %s\n", format_bits(buf, 8, clk2)); // 10000110
  if(clk2 != 0x86) {
    usart_printf("clk2 not expected default\n");
    return -1;
  }


#define OSR_WIDTH     4
#define OSR_OFF       0
#define OSR_VAL       0     // max oversampling rate 4096
  clk2 = SETFIELD(clk2, OSR_WIDTH, OSR_OFF, OSR_VAL);

#define ICLK_DIV_WIDTH  3
#define ICLK_DIV_OFF    5
#define ICLK_DIV_VAL    4 // 0b100  == 4. // table 29. default  100 : fMOD = fICLK / 8 (default)
                          // 101 : fMOD = fICLK / 10
  clk2 = SETFIELD(clk2, ICLK_DIV_WIDTH, ICLK_DIV_OFF, ICLK_DIV_VAL);

  adc_write_register(spi, CLK2, clk2);

  clk2 = adc_read_register(spi, CLK2);
  usart_printf("clk2 now %s\n", format_bits(buf, 8, clk2));   //  10000000


// OK.  can we OR together multiple bits????.... we can with nesting...
// but that's ugly.
  ///////////////////////////////



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
  usart_printf("stat_1 %s\n", format_bits(buf, 8, adc_read_register(spi, STAT_1)));
  usart_printf("stat_p %s\n", format_bits(buf, 8, adc_read_register(spi, STAT_P)));
#endif


  /*
    as soon as we enable adc, then we get an error ...
    because it starts generating data - that we don't consume ...
    that we didn't read a produced value properly, eg. because of encoding or not enough time. etc...
    not that there is a problem with our
  */

  // adc_write_register(spi, ADC_ENA, 0x0 );     // no channel.
                                              // setting

  // adc_write_register(spi, ADC_ENA, 0x01 );     // just one channel.
  // adc_write_register(spi, ADC_ENA, 0b1111 );
  // adc_write_register(spi, ADC_ENA, 0b1000 );     // vmon, chan4.  only.  now disconnected.
  // adc_write_register(spi, ADC_ENA, 0b0100 );   // (agnd - avss), chan3.  now disconnected.
  // adc_write_register(spi, ADC_ENA, 0x0f );     // all 4 channels

  // adc_write_register(spi, ADC_ENA, 0b0001 );      // ifb. chan1  10V in 1.65V measured.   10/2k = 1.666
  // adc_write_register(spi, ADC_ENA, 0b0010 );      // vfb.  chan2   10V in 1.65V measured.   10/2k = 1.666


  adc_write_register(spi, ADC_ENA, 0b0011 );     // vfb and ifb



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
  adc_print_status_registers(spi);



  usart_printf("ads131a04 ok\n");

#endif
  return 0;
}


/*
analog init ok
adc, bad code 2230
adc val -9999.
adc, bad code 2232
adc val -9999.

*/


// #define SWAP(a, b) do { typeof(a) temp = a; a = b; b = temp; } while (0)
// #define SWAP(TYPE,x,y)  TYPE a = (x), x=(y), y=(a);

static void swapf(float *x, float *y)
{
  // hideous
  float tmp = *x;
  *x = *y;
  *y = tmp;
}


int32_t spi_adc_do_read( uint32_t spi, float *ar, size_t n)
{
  UNUSED(n);
  // assert( n >= 2)

  mux_adc(spi);
  spi_enable(spi);



  // read status code
  uint32_t code = spi_xfer_24_16(spi, 0);     // this is just the 24_16 call... except without

  // read/consume values regardless of error
  // MIN(n, 1);
  for(unsigned j = 0; j < 2; ++j)
  {
    int32_t x = spi_xfer_24(spi, 0);
    x = sign_extend_24_32(x );
    // ar[j] = ((double )x) / 3451403.f;
    // ar[j] = ((double )x) / 345140.3f;   // weird.   for 10k/3k.

    ar[j] =  ((double)x) / 299410;    // 10k/2.5k
  }


  /*
    we need to distinguish ovp ovn  condition from other possible
    errors. eg. ovp/ovn we still want to return the value. to enable ranging to a better range.
  */
  int ret = 0;

  // log any errors
  if(code != 0x2220) {

#if 0
    usart_printf("adc, bad code %4x\n",  code);
#endif
    // must registers to clear for next time
    // adc_print_status_registers(spi);
    clear_status_registers(spi);
    // usart_printf("x");

    ret = -123;
  }

  // don't return early
  spi_disable( spi );

  return ret;
}


