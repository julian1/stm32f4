

#include "spi1.h"
#include "mux.h"
#include "reg.h"

#include "spi-ice40.h"
#include "mcp3208.h"
#include "w25.h"
#include "dac8734.h"
#include "ads131a04.h"



/*
  OK. we need to think about all this...

  to switch from ice40 to alternate. 
    cs to cs2. 
    then use the appropriate functions.

  so we have two alternate setup functions.

  spi_ice40_setup_cs1(spi);
  spi_ice40_setup_cs2(spi);

*/

// one bit
// put all these

#if 1
void mux_fpga(uint32_t spi)
{
  // spi on mcu side, must be correctly configured
  // in addition, relies on the special flag to mux

  spi1_port_setup();
  spi_ice40_setup(spi);
}
#endif




void mux_w25(uint32_t spi)
{
  // mux fpga, to write the reg.
  mux_fpga(spi);
  spi_ice40_reg_write(spi, REG_SPI_MUX, SPI_MUX_FLASH);

  // now mux the device
  spi1_port_setup2();
  spi_w25_setup(spi);
}

void mux_adc03(uint32_t spi)
{
  // spi_ice40_setup(spi);

  // mux fpga, to write the reg.
  mux_fpga(spi);
  spi_ice40_reg_write(spi, REG_SPI_MUX, SPI_MUX_ADC03);

  // now mux the device
  spi1_port_setup2();
  spi_mcp3208_setup(spi);
}




void mux_dac(uint32_t spi)
{
/*
  spi_ice40_setup(spi);
  spi_ice40_reg_write(spi, REG_SPI_MUX, SPI_MUX_DAC);
  spi_dac_setup(spi);
*/
  mux_fpga(spi);
  spi_ice40_reg_write(spi, REG_SPI_MUX, SPI_MUX_DAC);
 
  spi1_port_setup2();
  spi_dac_setup(spi);
}

void mux_adc(uint32_t spi)
{
/*
  spi_ice40_setup(spi);
  spi_ice40_reg_write(spi, REG_SPI_MUX, SPI_MUX_ADC);
  spi_adc_setup(spi);
*/
  mux_fpga(spi);
  spi_ice40_reg_write(spi, REG_SPI_MUX, SPI_MUX_ADC);

  spi1_port_setup2();
  spi_adc_setup(spi);
}


#if 0
// TODO remove. it's just a wrapper around mux_fpga().

// is this even needed
// sp

#define UNUSED(x) (void)(x)

void mux_fpga(uint32_t spi)
{
  // UNUSED(spi);
  mux_fpga(spi);
}
#endif

/////////////////////////////

// Not sure if should not just change, the name of the ice40 functions,
// but this differentiates the abstraction for consumers from underlying method.


////////////////////////////////


// TODO. wrapping/exposing this. is wrong i think.
// remove these.

void reg_set( uint32_t spi, uint8_t r, uint8_t v)
{
  spi_ice40_reg_set( spi, r, v);
}

void reg_clear( uint32_t spi, uint8_t r, uint8_t v)
{
  spi_ice40_reg_clear( spi, r, v);
}


void reg_write( uint32_t spi, uint8_t r, uint8_t v)
{
  spi_ice40_reg_write( spi, r, v);
}

void reg_toggle( uint32_t spi, uint8_t r, uint8_t v)
{
  spi_ice40_reg_toggle( spi, r, v);
}

void reg_write_mask( uint32_t spi, uint8_t r, uint8_t mask, uint8_t v)
{
  spi_ice40_reg_write_mask( spi, r, mask, v);
}


