
#include <libopencm3/stm32/spi.h>

#include "ice40.h"
#include "spi1.h"

// fpga as a target

void spi_ice40_setup(uint32_t spi)
{
  // the fpga as a spi slave.

  spi_init_master(
    spi,
    SPI_CR1_BAUDRATE_FPCLK_DIV_4,
    // SPI_CR1_BAUDRATE_FPCLK_DIV_16,
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
    SPI_CR1_CPHA_CLK_TRANSITION_2,    // 2 == falling edge
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST
  );

  // Think these are default
  // spi_disable_software_slave_management( spi);
  // spi_enable_ss_output(spi);
}



////////////////


static uint32_t spi_xfer_32(uint32_t spi, uint32_t val)
{
  spi_enable(spi);
  uint8_t a = spi_xfer( spi, (val >> 24) & 0xff );  // correct reg should be the first bit that is sent.
  uint8_t b = spi_xfer( spi, (val >> 16) & 0xff );
  uint8_t c = spi_xfer( spi, (val >> 8)  & 0xff  );
  uint8_t d = spi_xfer( spi,  val        & 0xff  );
  spi_disable(spi);

  // fixed this.
  // + or |
  return (a << 24) + (b << 16) + (c << 8) + d;        // this is better. needs no on reading value .
}



static uint32_t spi_reg_xfer_24(uint32_t spi, uint8_t reg, uint32_t val)
{
  // for write, or transfer
  return spi_xfer_32(spi, reg << 24 | val);

}

uint32_t spi_ice40_reg_read(uint32_t spi, uint8_t reg)
{
  // TODO. maybe rename to drop the 24. since 24 refers to val.
  // set the hi bit of the register
  // allows read, without value overwrite
  return spi_reg_xfer_24(spi, reg | (1 << 7), 0);
}


uint32_t spi_ice40_reg_write(uint32_t spi, uint8_t reg, uint32_t val)
{
  // spi_reg_xfer_24(SPI1, 7, 0x7f00ff );
  return spi_reg_xfer_24(spi, reg , val );
}










#if 0

static uint32_t spi_xfer_register_16(uint32_t spi, uint32_t r)
{
  // TODO change name. remove register
  // and rename r to val.
  uint8_t a = spi_xfer( spi, (r >> 8) & 0xff  );
  uint8_t b = spi_xfer( spi, r & 0xff  );

  return (a << 8) + b; // msb first, same as dac
}





static uint16_t spi_ice40_xfer(uint32_t spi, uint32_t r)
{
  // TODO rename xfer_16
  spi_special_flag_clear(spi);
  spi_enable(spi);
  uint16_t ret = spi_xfer_register_16(spi, r );
  spi_disable(spi);
  spi_special_flag_set(spi);
  return ret;
}


// OK. we are using this to write the spi muxing register with 8 bits.
// if need more bits then it's problematic

static uint16_t spi_ice40_xfer2( uint32_t spi, uint8_t r, uint8_t v)
{
  // change name to xfer also. I think.
  uint16_t ret = spi_ice40_xfer(spi, r << 8 | v );
  return ret;
}

// consumers should use the io_ functions.

void spi_ice40_reg_set( uint32_t spi, uint8_t r, uint8_t v)
{
  spi_ice40_xfer2(spi, r, (v & 0xF)); // ie. lo 4 bits
}

void spi_ice40_reg_clear( uint32_t spi, uint8_t r, uint8_t v)
{
  spi_ice40_xfer2(spi, r, v << 4);    // ie. hi 4 bits
}

// OK. don't think we need a separate hardware register...

uint16_t spi_ice40_reg_write( uint32_t spi, uint8_t r, uint8_t v)
{
  uint8_t x = (~v << 4) | (v & 0xF );
  return spi_ice40_xfer2(spi, r, x);
}



void spi_ice40_reg_toggle( uint32_t spi, uint8_t r, uint8_t v)
{
  uint8_t x = (v << 4) | (v & 0xF );
  spi_ice40_xfer2(spi, r, x);
}


void spi_ice40_reg_write_mask( uint32_t spi, uint8_t r, uint8_t mask, uint8_t v)
{
  mask = mask & 0xf;

  uint8_t x = ((~v << 4) & (mask << 4)) | ((v & 0xF ) & mask);
  spi_ice40_xfer2(spi, r, x);
}
#endif

