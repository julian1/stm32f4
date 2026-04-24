

#include <stdio.h>
#include <assert.h>


#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>

#include <peripheral/spi.h>

#include <device/spi-4094-0.h>
#include <device/spi-fpga0-reg.h>   // cs vec
#include <device/support.h>



#define UNUSED(x) ((void)(x))


#define _4094_MAGIC 546123


static void port_configure(spi_t *spi)
{
  UNUSED(spi);


  // SPI1_CS2 should already have been setup by spi.
  // because it is used for configure.
}


static void controller_configure( spi_t *spi_)
{
  assert(spi_ && spi_->magic == _4094_MAGIC);

  uint32_t spi = spi_->spi;

  spi_reset( spi );

  spi_init_master(
    spi,
    SPI_CR1_BAUDRATE_FPCLK_DIV_4,
    // SPI_CR1_BAUDRATE_FPCLK_DIV_16,
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,      // park to 0/lo == positive clok edge. park to 1 == negative clk edge.
    SPI_CR1_CPHA_CLK_TRANSITION_1,    // 1 == leading edge,  2 == falling edge
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST
  );

  spi_enable( spi );
}


static void cs_assert( spi_t *spi)
{
  assert(spi && spi->magic == _4094_MAGIC);

  spi_wait_ready( spi->spi);
  gpio_write_with_mask( GPIOC, 7, 0b111, SPI_CS_4094);
}


static void cs_deassert( spi_t *spi)
{
  assert(spi && spi->magic == _4094_MAGIC);

  spi_wait_ready( spi->spi);
  gpio_write_with_mask( GPIOC, 7, 0b111, SPI_CS_DEASSERT);
}


void spi_4094_0_init( spi_t *spi)
{
  assert(spi);

  *spi = ( const spi_t) {

    .magic        = _4094_MAGIC,
    .spi          = SPI1,     // consider, pass underlying spi in the contructor
    .port_configure = port_configure,
    .controller_configure = controller_configure,
    .cs_assert    = cs_assert,
    .cs_deassert  = cs_deassert,
  };

}



