

#include <assert.h>
#include <stdio.h>

#include <libopencm3/stm32/spi.h>

#include <peripheral/spi-ad5446.h>

/*
  issue is that the config is gpio setup.

  ---------------
  EXTR. make the configure a free standing function.
        rather than create it with the device.

      do this for ice40 also.
      if
    
     - i think the port configure should be free-standing. otherwise it forces us to put stuff in device

      void spi_ad5446_port_configure( spi_ad5446_t *spi)
  -------------------

*/

#if 1

// void spi_port_configure_ad5446( spi_ad5446_t *spi)
void spi_ad5446_port_configure( uint32_t spi)
{

  // ensure cs disabled
  // spi_port_cs1_disable( spi );  // disable, acvei lo
  // spi_port_cs2_disable( spi);

  // dac8811  data is clked in on clk leading rising edge.
  // ad5446 on falling edge.
  spi_init_master(
    spi,
    SPI_CR1_BAUDRATE_FPCLK_DIV_4,       // actually works over 50cm. idc cable.
    // SPI_CR1_BAUDRATE_FPCLK_DIV_16,
    // SPI_CR1_BAUDRATE_FPCLK_DIV_32,
    SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,      // ad5446 reads on neg edge. ONLY DIFFERENCE.   park to 0/lo == positive clok edge. park to 1 == negative clk edge.
    SPI_CR1_CPHA_CLK_TRANSITION_1,    // 1 == leading edge,  2 == falling edge
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST
  );

  spi_enable( spi );
}

#endif


// Need to create the device.  with approapriate config().


/* value writing code is the same for dac8811 and ad5446
  but keep separate. could enable bounds check for 14 bit etc.
  ad5446, requires top two bits kept low to maintain spi polarity.

  14bit. range 0 - 0x3fff
*/


static uint16_t spi_xfer_16( uint32_t spi, uint16_t val)
{
  uint8_t a = spi_xfer( spi, (val >> 8) & 0xff );  // correct reg should be the first bit that is sent.
  uint8_t b = spi_xfer( spi, val & 0xff );

  return (a << 8) + b;
}


void spi_ad5446_write16( spi_t *spi, uint16_t val)
// void spi_ad5446_write16(  spi_ad5446_t *spi, uint16_t val)
{
  // spi_port_cs2_enable(spi);
  spi->cs( spi, 0 ); 

  spi_xfer_16(spi->spi, val );

  // spi_port_cs2_disable(spi);
  spi->cs( spi, 1 ); 
}




