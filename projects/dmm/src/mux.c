
#include <stdio.h>    // printf

#include "spi-port.h"
#include "mux.h"
#include "reg.h"

#include "spi-ice40.h"

#include "4094.h"
// #include "mcp3208.h"
#include "w25.h"
// #include "dac8734.h"
// #include "ads131a04.h"

#include "assert.h"


/*
  can cache current setup port state with enum here. for speed.

  --------
  OK. we need to think about all this...

  to switch from ice40 to alternate.
    cs to cs2.
    then use the appropriate functions.

  so we have two alternate setup functions.

  spi_ice40_setup_cs1(spi);
  spi_ice40_setup_cs2(spi);

*/



// these  shouldn't need to be exposed.

#define REG_SPI_MUX     8

#define SPI_MUX_NONE    0
#define SPI_MUX_4094    1

/*
// FIXME. THESE ARE ALL WRONG, we now using monotonic number, not bit operations.
#define SPI_MUX_ADC03   (1<<0)
#define SPI_MUX_DAC     (1<<1)
#define SPI_MUX_FLASH   (1<<2)
#define SPI_MUX_ADC     (1<<3)
*/



// one bit
// put all these

void mux_ice40(uint32_t spi)
{
  // spi on mcu side, must be correctly configured
  // in addition, relies on the special flag to mux

  spi1_port_cs1_setup();
  spi_ice40_setup(spi);
}





void mux_no_device(uint32_t spi )
{
  // useful , turns off  common spi sigals (clk, mosi, miso etc).

  // printf("mux no device\n");

  assert( SPI_MUX_NONE == 0); // june 2023. dmm03.

  // needed to setup mcu port configuration
  mux_ice40(spi);
  spi_ice40_reg_write32(spi, REG_SPI_MUX,  SPI_MUX_NONE );

}

void mux_4094(uint32_t spi )
{
  // printf("mux 4094\n");

  assert( SPI_MUX_4094 == 1); // june 2023. for dmm03.

  mux_ice40(spi);
  // ice40_reg_set(spi, REG_SPI_MUX,  /*0x5*/ item );


  // JA
  spi_ice40_reg_write32(spi, REG_SPI_MUX,  SPI_MUX_4094 );

  // ensure gpio cs2 is disabled before switching from mcu AF to gpio control.
  // may not be needed, if gpio cs2 is not used in any other context
  spi1_port_cs2_disable();

  spi1_port_cs2_gpio_setup();
  spi_4094_setup(spi);

}


#if 0



void mux_w25(uint32_t spi)
{
  printf("mux w25\n");

  mux_ice40(spi);
  ice40_reg_write(spi, REG_SPI_MUX, SPI_MUX_FLASH);

  // now mux the device
  spi1_port_cs2_setup();
  spi_w25_setup(spi);
}




void mux_adc03(uint32_t spi)
{
  printf("mux adc03\n");

  mux_ice40(spi);
  ice40_reg_write(spi, REG_SPI_MUX, SPI_MUX_ADC03);

  // now mux the device
  spi1_port_cs2_setup();
  spi_mcp3208_setup(spi);
}




void mux_dac(uint32_t spi)
{
  printf("mux dac\n");

  mux_ice40(spi);
  ice40_reg_write(spi, REG_SPI_MUX, SPI_MUX_DAC);

  spi1_port_cs2_setup();
  spi_dac_setup(spi);
}

void mux_adc(uint32_t spi)
{
  printf("mux adc\n");

  mux_ice40(spi);
  ice40_reg_write(spi, REG_SPI_MUX, SPI_MUX_ADC);

  spi1_port_cs2_setup();
  spi_adc_setup(spi);
}

#endif



