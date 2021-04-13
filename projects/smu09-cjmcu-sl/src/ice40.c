
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
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
    SPI_CR1_CPHA_CLK_TRANSITION_2,    // 2 == falling edge
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST
  );

  spi_disable_software_slave_management( spi);
  spi_enable_ss_output(spi);
}


static uint32_t spi_xfer_register_16(uint32_t spi, uint32_t r)
{
  uint8_t a = spi_xfer( spi, (r >> 8) & 0xff  );
  uint8_t b = spi_xfer( spi, r & 0xff  );

  return (a << 8) + b; // msb first, same as dac
}





static uint16_t spi_ice40_xfer(uint32_t spi, uint32_t r)
{
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

void spi_ice40_reg_write( uint32_t spi, uint8_t r, uint8_t v)
{
  uint8_t x = (~v << 4) | (v & 0xF );
  spi_ice40_xfer2(spi, r, x );
}



void spi_ice40_reg_toggle( uint32_t spi, uint8_t r, uint8_t v)
{
  uint8_t x = (v << 4) | (v & 0xF );
  spi_ice40_xfer2(spi, r, x );
}


void spi_ice40_reg_write_mask( uint32_t spi, uint8_t r, uint8_t mask, uint8_t v)
{
  // not tested
  mask = mask & 0xf;

  uint8_t x = ((~v << 4) & (mask << 4)) | ((v & 0xF ) & mask);
  spi_ice40_xfer2(spi, r, x );
}


