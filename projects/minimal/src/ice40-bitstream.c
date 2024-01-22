


// #include <stdint.h>
// #include <stdio.h>
// #include <stdlib.h>
#include <assert.h>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>


#include <lib2/util.h>    // MAX
#include <lib2/stream-flash.h>

#include <peripheral/spi-port.h>
#include <peripheral/ice40-extra.h>



#include <ice40-bitstream.h>
#include <app.h>









static void spi_ice40_setup_manual(uint32_t spi)
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

}







int ice40_bitstream_test(app_t *app)
{

  // remove
  msleep(10, &app->system_millis);


  printf("ice40_bitstream_test\n");
  FILE *f = flash_open_file();
  assert(f);


  // read magic and length.

  uint32_t magic  = 0 ;
  uint32_t size = 0;

  fread(&magic, 1, 4, f);
  fread(&size, 1, 4, f);


  printf("magic %lx\n", magic );
  printf("size %lu\n", size );        // need to swap the byte order perhaps.

  // if(magic != 0xfe00fe00) {
  if(magic != 0xfe00fe00) {
    printf("bad magic!\n");
    fclose(f);
    return -1;
  }

  ////////////////////////////////////

  // see Fig. 13.2

  // use spi port with soft/manual control over cs1
  spi1_port_cs1_gpio_setup();

  // configure with soft/manual control over ss.
  spi_ice40_setup_manual(app->spi);


  // spi must be enabled in order to output clk cycles, regardless of state of SS.
  // note - because we use gpio, SS is not coupled, and doesn't wiggle when calling spi_enable/spi_disable funcs
  // possible that could use the spi_set_nss_low/high functions.
  spi_enable( app->spi );

  // start with creset enabled as well as cs hi. to make it easy to LA trigger on down transition
  ice40_port_extra_creset_enable();
  spi1_port_cs1_set();

  // wait
  msleep(1, &app->system_millis);



  //////////////////////////////////////////////
  // configure sequence

  // drive creset_b = 0  (pin lo).
  ice40_port_extra_creset_disable();

  // drive spi_ss = 0, spi_sck = 1
  spi1_port_cs1_clear();

  // wait minimum of 200ns
  msleep(1, &app->system_millis);

  // check cdone is lo
  assert(! ice40_port_extra_cdone_get() );

  // release creset (eg. pullup), or drive creset = 1
  // TODO change name enable to clear() or low()
  ice40_port_extra_creset_enable();

  // wait a minimum of of 1200u to clear internal config memory
  msleep(2, &app->system_millis);

  // set spi_ss = 1.
  spi1_port_cs1_set();

  // send 8 dummy clks
  spi_xfer( app->spi, 0x00 );

  // assert ss lo
  spi1_port_cs1_clear();


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
    //  print blob details - leading, trailing bytes
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
       spi_xfer( app->spi, buf[ i ] );


    remaining -= ret;
  }

  // printf("done remaining now %u\n", remaining);
  assert(remaining == 0);


  // spi-ss = high
  spi1_port_cs1_set();


  // wait - send up to 100 dummy clk cycles.
  unsigned i = 0;
  for(i = 0; i < 13  && !ice40_port_extra_cdone_get(); ++i)
     spi_xfer( app->spi, 0x00);

  printf("need %u dymmy bytes before cdone went hi\n", i );


  // check cdone really hi
  if(! ice40_port_extra_cdone_get() ) {
    printf("failed\n");

    spi_disable( app->spi );
    fclose(f);
    return -1;
  } else {

    printf("ok\n");
  }


  // send another 49 clk cycles, for gpio to become active
  for(i = 0; i < 7 ; ++i)
     spi_xfer( app->spi, 0x00);



  spi_disable( app->spi );
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


