


// #include <stdint.h>
// #include <stdio.h>
// #include <stdlib.h>
#include <assert.h>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>


#include <lib2/util.h>    // MAX
#include <lib2/stream-flash.h>

// #include <peripheral/spi-port.h>
#include <peripheral/ice40-extra.h>



#include <ice40-bitstream.h>
#include <app.h>







/*
  need to pass the spi to use.

  we need to be able to msleep, and api.
*/

/*
      ---------------
  p29.

  Send the entire configuration image, without interruption, serially to the iCE40’s SPI_SI input on the falling
  edge of the SPI_SCK clock input. Once the AP sends the 0x7EAA997E synchronization pattern, the generated SPI_SCK
  clock frequency must be within the range specified in the data sheet while sending the configuration image.
  Send each byte of the configuration image with most-significant bit (msb) first. The AP sends data to the iCE40 FPGA on
  the falling edge of the SPI_SCK clock. The iCE40 FPGA internally captures each incoming SPI_SI data bit on the rising
  edge of the SPI_SCK clock. The SPI_SO output pin in the iCE40 is not used during SPI slave mode but must connect to
  the AP if the AP also programs the NVCM of the iCE40 device.


  After sending the entire image, the iCE40 FPGA releases the CDONE output allowing it to float High via the external
  pull-up resistor to AP_VCC. If the CDONE pin remains Low, then an error occurred during configuration and the AP
  should handle the error accordingly for the application.

  After the CDONE output pin goes High, send at least 49 additional dummy bits, effectively 49 additional SPI_SCK clock
  cycles measured from rising-edge to rising-edge.
  After the additional SPI_CLK cycles, the SPI interface pins then become available to the user-application loaded in
  FPGA. In the iCE40-1KLP SWG16 package, the CDONE pin can be used as a user output.

*/


int ice40_bitstream_test(app_t *app)
{


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

  // think we may want manual configuration. jjjjjj
  // see Fig. 13.2


  // drive creset_b = 0  (pin lo).
  ice40_port_extra_creset_disable();


  // drive spi_ss = 0, spi_sck = 1
  spi_enable(app->spi);

  // wait minimum of 200ns
  msleep(1, &app->system_millis);

  // check cdone is lo
  assert(! ice40_port_extra_cdone_get() );


  // release creset (eg. pullup), or drive creset = 1
  ice40_port_extra_creset_enable();


  // wait a minimum of of 1200u to clear internal config memory
  msleep(10, &app->system_millis);


  // set spi_ss = 1.      (THIS IS SURPRISING)  polarity appears inverted
  // check this works.
  // may be hardware issue - if try to send with spi-disabled.
  spi_disable(app->spi);

  // send 8 dummy clks
  spi_xfer( app->spi, 0x00 );

  // enable spi again
  spi_enable(app->spi);



  // send image.

  // IMPORTANT - note the 4 bytes at the start before the sequence.  these may need to be dropped.



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
    ///  print deails of blob.
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

  printf("done remaining now %u\n", remaining);
  assert(remaining == 0);


  // send 100 more clock cycles.
  for(unsigned i = 0; i < 100; ++i)
     spi_xfer( app->spi, 0x00);

  // check cdone is hi
  assert( ice40_port_extra_cdone_get() );




  // send 49 more clock cycles. for to gpio to function..





  // wait
  msleep(10, &app->system_millis);

  // check cdone is hi
  assert(! ice40_port_extra_cdone_get() );


  // when do we relase spi select.   and creset?.

  // need another 49 clk cycles.



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


