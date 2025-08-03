/*
  This is the same as mdac0.  except for the cs_assert()
  we could just clone the pointer array to use the same functions.
  but would have to pass the mdac0 device as a reference.
  which is messy.
  So just duplicate the code for the moment.

*/

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>

#include <stdio.h>
#include <string.h>   // memset
#include <assert.h>
#include <stdlib.h>


#include <peripheral/spi.h>
#include <support.h>      // spi_wait_read()

#include <device/mdac1.h>
#include <device/fpga0_reg.h>   // cs vec




#define UNUSED(x) ((void)(x))


#define SPI1_PORT       GPIOA
#define SPI1_CS2        GPIO15     // gerber 257. control-panel-07



static void setup(spi_t *spi )
{
  UNUSED(spi);

  // cs vec already have been setup by the spi controller.
}



static void port_configure( spi_t *spi_)
{
  assert( spi_ && spi_->spi );
  uint32_t spi = spi_->spi;
  assert(spi == SPI1);

  // ensure cs disabled

  // dac8811  data is clked in on clk leading rising edge.
  // ad5446 on falling edge.
  spi_init_master(
    spi,
    SPI_CR1_BAUDRATE_FPCLK_DIV_4,       // actually works over 50cm. idc cable.
    // SPI_CR1_BAUDRATE_FPCLK_DIV_16,
    // SPI_CR1_BAUDRATE_FPCLK_DIV_32,
    SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,      // ad5446 reads on neg edge. ONLY DIFFERENCE.   park to 0/lo == positive clok edge. park to 1 == negative clk edge.
    SPI_CR1_CPHA_CLK_TRANSITION_1,    // 1 == leading edge,  2 == falling edge
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST
  );

  spi_enable( spi );
}



static void cs_assert(spi_t *spi)
{
  assert(spi->spi == SPI1);
  spi_wait_ready( spi->spi);

  assert(SPI_CS_VEC_MDAC0 == 3);
  gpio_write_with_mask( GPIOC, 7, 0b111, SPI_CS_VEC_MDAC1);

}

static void cs_deassert(spi_t *spi)
{
  assert(spi->spi == SPI1);
  spi_wait_ready( spi->spi);

  gpio_write_with_mask( GPIOC, 7, 0b111, SPI_CS_VEC_DEASSERT);
}






spi_t * spi_mdac1_create( )
{
  spi_t *spi = malloc(sizeof( spi_t));
  assert(spi);
  memset(spi, 0, sizeof(spi_t));

  // base
  spi->spi    = SPI1;     // NOT sure if the spi should be passed in the contructor.
  spi->setup   =  setup;
  spi->port_configure = port_configure;
  spi->cs_assert    = cs_assert;
  spi->cs_deassert  = cs_deassert;

  return spi;
}



