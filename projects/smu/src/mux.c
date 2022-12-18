
#include <stdio.h>    // printf

#include "spi-port.h"
#include "mux.h"
#include "reg.h"

#include "spi-ice40.h"
#include "mcp3208.h"
#include "w25.h"
#include "dac8734.h"
#include "ads131a04.h"
#include "4094.h"



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

void mux_ice40(uint32_t spi)
{
  // spi on mcu side, must be correctly configured
  // in addition, relies on the special flag to mux

  spi1_port_cs1_setup();
  spi_ice40_setup(spi);
}




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


void mux_4094(uint32_t spi)
{
  printf("mux 4094\n");

  mux_ice40(spi);
  ice40_reg_set(spi, REG_SPI_MUX,  0x5 );

  // set the cs2 gpio bit to high, before change from pull-up , to enable spi peripheral gpio, to avoid generating a false strobe

  // clocking, mosi ok.
  // needs to be at 200ns to see

  // spi1_port_cs2_setup();
  // normal state is lo
  spi1_port_cs2_disable();      // inverted by fpga.

  spi1_port_cs2_gpio_setup();
  spi_4094_setup(spi);

}



