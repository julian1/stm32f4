



#include <assert.h>

#include <libopencm3/stm32/spi.h>

#include <peripheral/spi-4094.h>
#include <peripheral/spi-ice40.h>



/*
// 4094 output is transparent on strobe-hi,  and latched on strobe negative edge..  normally park lo.
// OK. there is issue that the clock parks high. when strobe goes lo. so there's an extra clk edge.

// WHICH is probably caused by our fpga code change...
// the clock is returning to high. (eg. another edge). before the strobe negative edge, that latches everything.has finished.
*/



/*




  ------
  "The data in the shift register is transferred to the storage register when the STR input is HIGH"

  - so if strobe is high then storage register and output will be transparent. and the output will not change as atomic update.
      which is generally what we want.

  - so we only want to assert strobe high briefly - after we have clocked the correct data in.
  - the shift register always gets/holds the last mosi data. regardless of the cs/strobe
  ----------


  - there is no way to read the register contents, because the value being clocked out, is not the value that was latched in by the strobe.
    instead it is just whatever data was present from the last use of the spi lines - perhaps intended for another spi peripheral device.
  --------

  - this is in addition to the MISO/ output which will not go high-Z when used, meaning cannot share spi lines for other spi peripherals.
  - and we don't have the right timing characteristics with the strobe if wanted to use tri-state buffer
  - and we would need to write a dummy value, while reading.
  - to avoid would need two wire control for read/strobe. but won't work - again because the shift value last contents are not the latch contents.


*/






static void assert_strobe( spi_t *spi)
{
  /*
    4094 output is transparent on strobe-hi,  and latched on strobe negative edge..  normally park lo.
    OK. there is issue that the clock parks high. before strobe goes lo. creating an extra positive clk edge.
    which shifts the data.
    --
    note, this happens even if configure mcu clock to park lo. because afterwards it will still shift clkk to hi.
  */


  // assert 4094 strobe.
  // fpga inverts active lo.

  spi_cs(spi, 0 );


  for(uint32_t i = 0; i < 10; ++i)   // 100count == 5us.
     __asm__("nop");

  // normal state is lo
  spi_cs(spi, 1 );

}



// think passing a unsigned char *s. is cleaner than void *.
// can then call with &value.

uint32_t spi_4094_write_n( spi_t *spi, const unsigned char *s, size_t n)
{
  uint32_t ret = 0;


  /*
    for(unsigned i = 0; i < n; ++i) {
      ret = spi_xfer(spi, v);
      v >>= 8;
      ret <<= 8;  // check
    }
  */

  // we want to push the last byte first. but avoid addressing.


  for(signed i = n - 1; i >= 0; --i) {
    ret = spi_xfer(spi->spi, s[i] );

    ret <<= 8;  // check
  }

  assert_strobe(spi);

  return ret;
}









#if 0

uint8_t spi_4094_write( spi_t *spi, uint8_t v)
{
  assert( 0);
  // expect port is already configured with gpio for cs etc.


  // TODO maybe remove the enable.  not required by 4094. maybe required by stm32 spi hardware.
  /*
    EXTR
      - without the spi_enable()/ spi_disable() the spi_xfer() will hang, because waiting for spi peripheral status registers.
      - But if configure the peripheral without hardware toggling of cs, perhaps it is ok.

  */
  uint8_t ret = spi_xfer(spi->spi, v);

  assert_strobe( spi);

  return ret;
}
#endif






// TODO rename spi_mux_4094().  because first arg is spi

#if 0
void spi_mux_4094(uint32_t spi )
{
  UNUSED(spi);
  assert(0);

  /*
      0. mux the fpga.
      1. write the fpga mux register, for the spi device which is 4094 system
      2. then setup the port on mcu side to write 4094.
  */

  // EXTR. setup on the ice40 side.
  // printf("mux 4094\n");

  assert( SPI_MUX_4094 == 1); // june 2023. for dmm03.

  assert(0);
  // spi_mux_ice40( spi);

  // default state - should always be to *not* to propagate spi on 4094 lines.  to avoid emi
  assert( spi_ice40_reg_read32(spi, REG_SPI_MUX ) == SPI_MUX_NONE);

  // set ice40 to mux spi to 4094 peripheral
  spi_ice40_reg_write32(spi, REG_SPI_MUX,  SPI_MUX_4094 );


  // ensure gpio cs2 is disabled before switching from mcu AF to gpio control.
  // may not be needed, if gpio cs2 is not used in any other context


  spi_port_cs1_disable( spi );  // disable, actually hi, but fpga will invert for 4094 active hi strobe.
  spi_port_cs2_disable( spi);


   spi_init_master(
    spi,
    SPI_CR1_BAUDRATE_FPCLK_DIV_4,
    // SPI_CR1_BAUDRATE_FPCLK_DIV_16,
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,      // park to 0/lo == positive clok edge. park to 1 == negative clk edge.
    SPI_CR1_CPHA_CLK_TRANSITION_1,    // 1 == leading edge,  2 == falling edge
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST
  );

  spi_enable( spi );
}
#endif













#if 0

  /* EXTR.   sep 18, 2023.
      finish 4094 strobe - before we let the stm32 know we have finished with spi.
      otherwise mcu/fpga may generate spurious extra clk or data  parking pulses.
      that get read by 4094 while strobe is active, and 4094 is transparent.

  */
  // spi_disable( spi );


/*
  spi_port_cs2_gpio_setup();
  spi_setup(spi);
*/

static void spi_setup(uint32_t spi)
{
  // not clear this needs to be exposed.
  // EXTR. setup on the mcu side.

  spi_reset( spi );

  spi_init_master(
    spi,
    SPI_CR1_BAUDRATE_FPCLK_DIV_4,
    // SPI_CR1_BAUDRATE_FPCLK_DIV_16,
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,      // park to 0/lo == positive clok edge. park to 1 == negative clk edge.
    SPI_CR1_CPHA_CLK_TRANSITION_1,    // 1 == leading edge,  2 == falling edge
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST
  );

  // TODO.  set the SS slave manage.
  // and can probably use spi_set_nss_low (uint32_t spi)
}

#endif

