

#include <assert.h>
#include <stdio.h>

#include <libopencm3/stm32/spi.h>

#include <peripheral/spi-ad5446.h>


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
{
  assert(spi);

  spi_cs_assert( spi);

  spi_xfer_16(spi->spi, val );

  spi_cs_deassert( spi);
}




