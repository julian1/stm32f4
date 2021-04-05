
#include "mux.h"
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

  // if we have more than one. then pass a structure - with spi handle, and other registers.
  // the problem is we have to prefex all the calls and it gets too messy.
  spi_adc_setup(spi);


}



