
#include "core.h"
#include "ice40.h"

#include "mcp3208.h"
#include "w25.h"
#include "dac8734.h"
#include "ads131a04.h"

// one bit
// put all these

void mux_fpga(uint32_t spi)
{
  spi_ice40_setup(spi);
  // spi uses the special flag to communicate
}


void mux_adc03(uint32_t spi)
{
  spi_ice40_setup(spi);
  spi_ice40_reg_write(spi, SPI_MUX_REGISTER, SPI_MUX_ADC03);
  spi_mcp3208_setup(spi);
}

void mux_w25(uint32_t spi)
{
  spi_ice40_setup(spi);
  spi_ice40_reg_write(spi, SPI_MUX_REGISTER, SPI_MUX_FLASH);
  spi_w25_setup(spi);
}


void mux_dac(uint32_t spi)
{
  spi_ice40_setup(spi);
  spi_ice40_reg_write(spi, SPI_MUX_REGISTER, SPI_MUX_DAC);
  spi_dac_setup(spi);
}

void mux_adc(uint32_t spi)
{
  spi_ice40_setup(spi);
  spi_ice40_reg_write(spi, SPI_MUX_REGISTER, SPI_MUX_ADC);
  spi_adc_setup(spi);
}



/////////////////////////////

// Not sure if should not just change, the name of the ice40 functions,
// but this differentiates the abstraction for consumers from underlying method.



void mux_io(uint32_t spi)
{
  mux_fpga(spi);
}

void io_set( uint32_t spi, uint8_t r, uint8_t v)
{
  spi_ice40_reg_set( spi, r, v);
}

void io_clear( uint32_t spi, uint8_t r, uint8_t v)
{
  spi_ice40_reg_clear( spi, r, v);
}


void io_write( uint32_t spi, uint8_t r, uint8_t v)
{
  spi_ice40_reg_write( spi, r, v);
}

void io_toggle( uint32_t spi, uint8_t r, uint8_t v)
{
  spi_ice40_reg_toggle( spi, r, v);
}

void io_write_mask( uint32_t spi, uint8_t r, uint8_t mask, uint8_t v)
{
  spi_ice40_reg_write_mask( spi, r, mask, v);
}


