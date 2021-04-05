
#include "mux.h"
#include "ice40.h"

#include "mcp3208.h"
#include "w25.h"
#include "dac8734.h"

// one bit
// put all these

void mux_fpga(uint32_t spi)
{
  spi_ice40_reg_setup(spi);
  // spi uses the special flag to communicate
}


void mux_adc03(uint32_t spi)
{
  spi_ice40_reg_setup(spi);
  spi_ice40_reg_write(spi, SPI_MUX_REGISTER, SPI_MUX_ADC03);
  spi_mcp3208_setup(spi);
}

void mux_w25(uint32_t spi)
{
  spi_ice40_reg_setup(spi);
  spi_ice40_reg_write(spi, SPI_MUX_REGISTER, SPI_MUX_FLASH);
  spi_w25_setup(spi);
}


void mux_dac(uint32_t spi)
{
  spi_ice40_reg_setup(spi);
  spi_ice40_reg_write(spi, SPI_MUX_REGISTER, SPI_MUX_DAC);
  spi_dac_setup(spi);
}




