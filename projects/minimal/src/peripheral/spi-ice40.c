
/*
  ice40 spi is just one of multiple targets, like adc,dac,w25q etc.
  spi_ice40_
  ------

  should be able to be shared for 24 bit register system. and 8 bit register system. just suffix.
  ---


  see,

     #include <byteswap.h>

     uint16_t bswap_16(uint16_t x);
     uint32_t bswap_32(uint32_t x);
     uint64_t bswap_64(uint64_t x);


*/

#include <assert.h>


#include <libopencm3/stm32/spi.h>


/*
  everything is typed on spi_t not spi_ice40_t
*/
#include <peripheral/spi-ice40.h>




static uint32_t spi_xfer_32(uint32_t spi, uint32_t val)
{
  uint8_t a = spi_xfer( spi, (val >> 24) & 0xff );  // correct reg should be the first bit that is sent.
  uint8_t b = spi_xfer( spi, (val >> 16) & 0xff );
  uint8_t c = spi_xfer( spi, (val >> 8)  & 0xff  );
  uint8_t d = spi_xfer( spi,  val        & 0xff  );

  // fixed this.
  // + or |
  return (a << 24) + (b << 16) + (c << 8) + d;        // this is better. needs no on reading value .
}


/*
static uint32_t spi_reg_xfer_24(uint32_t spi, uint8_t reg, uint32_t val)
{
  // for write, or transfer
  return spi_xfer_32(spi, reg << 24 | val);
}
*/




uint32_t spi_ice40_reg_write32( spi_t *spi, uint8_t reg, uint32_t val)
{
  spi_cs_assert(spi);

  // write single byte, for the reg we are interested in, with read bit cleared.
  spi_xfer( spi->spi, reg );
  // return the data
  uint32_t ret = spi_xfer_32(spi->spi, val );

  spi_cs_deassert(spi);

  return ret;
}




uint32_t spi_ice40_reg_read32(  spi_t *spi, uint8_t reg)
{
  // call write with, with read bit set, and passing dummy value.
  return spi_ice40_reg_write32( spi, reg | (1 << 7), 0);
}


uint32_t spi_ice40_reg_write_n( spi_t *spi, uint8_t reg, const void *s, size_t n )
{
  // helper function when passing structs.
  // for cast.
  assert(n == 4); // only 32bit supported atm.


  return spi_ice40_reg_write32(spi, reg, *(uint32_t *)s );
}








