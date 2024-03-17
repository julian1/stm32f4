


#include <assert.h>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>


#include <lib2/util.h>    // MAX
#include <lib2/stream-flash.h>



#include <peripheral/spi-port.h>
#include <peripheral/ice40-extra.h>
#include <peripheral/spi-ice40-bitstream.h>


// last 128 . on 512k.
// this be declared in /periphal  perhaps. it's an arch/build dependency
#define FLASH_SECT_ADDR   0x08060000






static void spi_ice40_bitstream_setup(uint32_t spi)
{

  /*
    - assert mosi on neg edge.  ice40 reads on pos edge.

    the 49 additional clock cycles is useful-  since it clearly demarcates where configuration ends, and when spi becomes active with user comms.
    actually cs is high already by this time. so it doesn't matter too much.

    - mini-grabbers on soic is a problem, because they end up touching eash other, giving wrong signals.
  */


  spi_reset( spi );

  spi_init_master(
    spi,
    // SPI_CR1_BAUDRATE_FPCLK_DIV_2,  // div2 seems to work with iso, but not adum. actually misses a few bits with iso.
//    SPI_CR1_BAUDRATE_FPCLK_DIV_4,
    SPI_CR1_BAUDRATE_FPCLK_DIV_16,
    // SPI_CR1_BAUDRATE_FPCLK_DIV_32,
    // SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,  // park to 0/lo == positive clok edge. park to 1 == negative clk edge.
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,  // park to 0/lo == positive clok edge. park to 1 == negative clk edge.
    SPI_CR1_CPHA_CLK_TRANSITION_1,    // 1 == leading edge,  2 == falling edge
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST
  );

  spi_enable( spi );

}


// catch errors
#define spi_enable(x) WHOOT(x)
#define spi_disable(x) WHOOT(x)






int spi_ice40_bitstream_send(uint32_t spi,  volatile uint32_t *system_millis)
{
  printf("spi_ice40_bitstream_send\n");

  FILE *f = flash_open_file( FLASH_SECT_ADDR );
  assert(f);


  // read magic and length.
  uint32_t magic  = 0 ;
  uint32_t size = 0;

  fread(&magic, 1, 4, f);
  fread(&size, 1, 4, f);

  printf("magic %lx\n", magic );
  printf("size %lu\n", size );        // need to swap the byte order perhaps.

  if(magic != 0xfe00fe00) {
    printf("bad magic!\n");
    fclose(f);
    return -1;
  }

  ////////////////////////////////////

  // see Fig. 13.2

  // use spi port with soft/manual control over cs1


  // configure with soft/manual control over cs.
  spi_ice40_bitstream_setup(spi);


  // must have spi enabled to output clk cycles, regardless of state of SS.

  // start with creset enabled as well as cs hi. to make it easy to LA trigger on down transition
  ice40_port_extra_creset_enable();


  spi_port_cs1_disable( spi);
  spi_port_cs2_disable( spi);


  // wait
  msleep(1, system_millis);



  //////////////////////////////////////////////
  // configure sequence

  // drive creset_b = 0  (pin lo).
  ice40_port_extra_creset_disable();

  // drive spi_ss = 0, spi_sck = 1
  spi_port_cs1_enable(spi);

  // wait minimum of 200ns
  msleep(1, system_millis);

  // check cdone is lo
  assert(! ice40_port_extra_cdone_get() );

  // release creset (eg. pullup), or drive creset = 1
  // TODO change name enable to clear() or low()
  ice40_port_extra_creset_enable();

  // wait a minimum of of 1200u to clear internal config memory
  msleep(2, system_millis);

  // set spi_ss = 1.
  spi_port_cs1_disable(spi);

  // send 8 dummy clks
  spi_xfer( spi, 0x00 );

  // assert ss lo
  spi_port_cs1_enable(spi);


  /* send image.
    note the 4 byte start sequence of ff0000ff, before magic token 7eaa997e. seems ok

    size 135100
    10000   ff  0  0 ff 7e aa 99 7e  ..  0  0  0  0  0  1  0  0
    ...
    5100    0  0  0  0  0  0  0  0  ..  0  0 22 46 f6  1  6  0
  */

  size_t remaining = size;
  while(remaining > 0) {

    char buf[10000];
    size_t ret = 0;

    if(remaining >= 10000) {
      ret =  fread(buf, 1, 10000, f);
      assert(ret);
    }
    else {
      ret = fread(buf, 1, remaining , f);
      assert(ret);
    }

    //////////////////////
    // print some blob details - leading, trailing bytes
    printf("%u   ", ret);

    for(unsigned i = 0; i < 8; ++i)
      printf("%2x ", buf[i]);

    printf(" .. ");
    // not sure if the max deals with signedness here...
    for(unsigned i = MAX( (signed)ret - 8, 0); i < ret; ++i)
      printf("%2x ", buf[i]);
    printf("\n");
    /////////////////////////////

    // send data
    for(unsigned i = 0; i < ret; ++i)
       spi_xfer( spi, buf[ i ] );


    remaining -= ret;
  }

  // printf("done remaining now %u\n", remaining);
  assert(remaining == 0);


  // spi-ss = high
  spi_port_cs1_disable(spi);


  // wait - send up to 100 dummy clk cycles.
  unsigned i = 0;
  for(i = 0; i < 13  && !ice40_port_extra_cdone_get(); ++i)
     spi_xfer( spi, 0x00);

  printf("needed %u dummy bytes before cdone went hi\n", i );


  // check cdone really hi
  if(! ice40_port_extra_cdone_get() ) {
    printf("failed\n");

    fclose(f);
    return -1;
  } else {

    printf("ok\n");
  }


  // send another 49 clk cycles, for gpio to become active
  for(i = 0; i < 7 ; ++i)
     spi_xfer( spi, 0x00);



  fclose(f);
  return 0;
}




#if 0

// should be passing an argument to check.

int flash_raw_test(void)
{
  printf("flash raw test\n");
  FILE *f = flash_open_file();
  assert(f);
  // print first 100 chars.
  char buf[ 100 ] ;
  size_t ret = fread(buf, 1, 100, f);
  if(!ret ) {
      printf("flash read returned nothing\n");
  } else {

    for(unsigned i = 0; i < ret; ++i ) {
      putchar( buf[ i] );
    }
    printf("\n");
    for(unsigned i = 0; i < ret; ++i ) {
      printf("%2x ", buf[ i] );
    }

    printf("\n");
  }

  fclose(f);
  return 0;
}

#endif


