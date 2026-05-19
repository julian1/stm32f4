
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
#include <string.h>


#include <libopencm3/stm32/spi.h>


/*
  everything is typed on spi_t not spi_ice40_t
*/
#include <peripheral/spi-ice40.h>




static uint32_t spi_xfer_32(uint32_t spi, uint32_t val)
{
  uint8_t a = spi_xfer( spi, (val >> 24) & 0xff );  // write MSB first
  uint8_t b = spi_xfer( spi, (val >> 16) & 0xff );
  uint8_t c = spi_xfer( spi, (val >> 8)  & 0xff  );
  uint8_t d = spi_xfer( spi,  val        & 0xff  );

  // fixed this.
  // + or |
  return (a << 24) + (b << 16) + (c << 8) + d;
}


/*
static uint32_t spi_reg_xfer_24(uint32_t spi, uint8_t reg, uint32_t val)
{
  // for write, or transfer
  return spi_xfer_32(spi, reg << 24 | val);
}
*/



static uint32_t spi_ice40_reg_xfer( spi_t *spi, uint8_t reg, uint8_t write_flag,  uint32_t val)
{
  spi_cs_assert( spi);

  // OLD write byte for reg. write bit lo == active.
  // spi_xfer( spi->spi, reg );

  // write reg packed in MSB, write_flag in LSB, and use active HI write_flag.
  spi_xfer( spi->spi, (reg << 1) | write_flag);

  // return the data
  uint32_t ret = spi_xfer_32( spi->spi, val);

  spi_cs_deassert( spi);

  return ret;
}


uint32_t spi_ice40_reg_write32( spi_t *spi, uint8_t reg, uint32_t val)
{

  return spi_ice40_reg_xfer( spi, reg, 0b1, val);
}



uint32_t spi_ice40_reg_read32( spi_t *spi, uint8_t reg)
{
  // OLD call write with, with write bit MSB, and write_bit hi == no write, and pass dummy value.
  // return spi_ice40_reg_write32( spi, (1 << 7) | reg, 0);

  // call xfer with write_flag cleared, and pass dummy val
  return spi_ice40_reg_xfer( spi, reg, 0b0, 0);
}



void spi_ice40_reg_write_n( spi_t *spi, uint8_t reg, const void *s, size_t n)
{
  // convenience for structs.
  // only 32bit supported atm.
  assert(n == 4);

  /*return */
  spi_ice40_reg_write32( spi, reg, *(uint32_t *)s );
}


void spi_ice40_reg_read_n( spi_t *spi, uint8_t reg, void *s, size_t n)
{
  // convenience for structs.
  // only 32bit supported atm.
  assert(n == 4);

  uint32_t val = spi_ice40_reg_read32( spi, reg);

  memcpy( s, &val, n);
}





