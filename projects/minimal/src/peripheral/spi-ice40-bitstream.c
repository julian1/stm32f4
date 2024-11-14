


#include <assert.h>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>


#include <lib2/util.h>    // MAX
//#include <lib2/stream-flash.h>



//#include <peripheral/spi-port.h>
#include <peripheral/spi-ice40.h>


// #include <peripheral/ice40-extra.h>
#include <peripheral/spi-ice40-bitstream.h>







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




int spi_ice40_bitstream_send( spi_ice40_t *spi , FILE *f, size_t size , volatile uint32_t *system_millis)
{
  printf("spi_ice40_bitstream_send\n");

  assert(f);



  // uint32_t size = 104090 ;      // UP5K

  assert(size == 104090 /* || size == hx8k_size */ );

  // read magic and length.
  uint32_t magic  = 0 ;
  // uint32_t magic2  = 0 ;


  fread(&magic, 1, 4, f);
  fseek(f, 0, SEEK_SET );    // seek start again.



  printf("magic %lx\n", magic );

  /*
  Once the AP sends the 0x7EAA997E synchronization pattern, the generated SPI_SCK clock frequency must
  be within the range specified in the data sheet while sending the configuration image.
      https://blog.aleksander.kaweczynski.pl/wp-content/uploads/2024/07/iCE40_Programming_Configuration_2022.pdf

   me@flow:~/devel/ice40-fpga/projects/minimal$ hexdump -C build/main.bin | head
  00000000  ff 00 00 ff 7e aa 99 7e  51 00 01 05 92 00 20 62  |....~..~Q..... b|

  should just this as the magic number. and then hardcode the size.
  rather than prepend a separate header.

  */

  // if(magic != 0xfe00fe00) {
  if(magic != 0xff0000ff ) {
    printf("bad magic!\n");
    return -1;
  } else {

    printf("magic ok!\n");
  }






  ////////////////////////////////////

  // see Fig. 13.2

  // use spi port with soft/manual control over cs1


  // configure with soft/manual control over cs.
  spi_ice40_bitstream_setup(spi->spi);


  // must have spi enabled to output clk cycles, regardless of state of SS.

#if 0
  // start with creset enabled as well as cs hi. to make it easy to LA trigger on down transition
  ice40_port_extra_creset_enable();       // reset lo. start reset.
#endif

  // enable == lo.
  // disable == hi.

  // spi_port_cs1_disable( spi);             // cs1 hi.
  spi->cs(spi, 1);

  // spi_port_cs2_disable( spi);             // cs2 hi.   but it isn't unconditional.
  spi->rst(spi, 1);




  // wait
  msleep(1, system_millis);



  //////////////////////////////////////////////
  // configure sequence

#if 0
  // drive creset_b = 0  (pin lo).
  ice40_port_extra_creset_disable();      // creset/ clear / lo. inverse.
#endif

  // spi_port_cs2_enable(spi);                 // cs2 lo.
  // drive spi_ss = 0, spi_sck = 1
  // spi_port_cs1_enable(spi);                 // cs1 lo.   - now in reset.

  spi->cs(spi, 0);
  spi->rst(spi, 0);       // now in reset


  // wait minimum of 200ns
  msleep(1, system_millis);

  // check cdone is lo
  // assert(! cdone-spi_port_cdone_get() );
  assert( ! spi->cdone(spi));

  printf("here0\n");
#if 0
  // release creset (eg. pullup), or drive creset = 1
  // TODO change name enable to clear() or set()
  ice40_port_extra_creset_enable();         // creset set/ hi. inverse.  (Can do this by releasing cs2 ).
#endif

  // spi_port_cs2_disable(spi);          // cs2 hi.  out of reset.
  spi->rst(spi, 1);                       // out of reset

  // wait a minimum of of 1200u to clear internal config memory
  msleep(2, system_millis);

  // set spi_ss hi = 1.
  // spi_port_cs1_disable(spi);                // cs1 hi.
  spi->cs(spi, 1);

  // send 8 dummy clks
  spi_xfer( spi->spi, 0x00 );

  // assert ss lo = 0
  // spi_port_cs1_enable(spi);
  spi->cs(spi, 0);


  // cs(spi, 0);
  // or dev_spi_cs


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
       spi_xfer( spi->spi, buf[ i ] );


    remaining -= ret;
  }

  // printf("done remaining now %u\n", remaining);
  assert(remaining == 0);




  // spi-ss = high
  // spi_port_cs1_disable(spi);
  spi->cs(spi, 1);


  // wait - send up to 100 dummy clk cycles.
  unsigned i = 0;
  // for(i = 0; i < 13  && !   spi_port_cdone_get(); ++i)
  for(i = 0; i < 13  && !  spi->cdone(spi); ++i)
     spi_xfer( spi->spi, 0x00);




  // check cdone really hi
  if(! spi->cdone(spi) ) {
    printf("failed\n");


    // set cs. ports lo again..  so if fpga gets powered up. it will succeed.
    // TODO re-enable
    //spi->cs(spi, 0);
    // spi->rst(spi, 0);

    return -1;
  } else {

    printf("needed %u dummy bytes before cdone went hi\n", i );
    printf("ok\n");
  }


  // send another 49 clk cycles, for gpio to become active
  for(i = 0; i < 7 ; ++i)
     spi_xfer( spi->spi, 0x00);


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


