

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>


#include <stdio.h>
#include <string.h>   // memset
#include <assert.h>
#include <stdlib.h>



#include <peripheral/spi.h>
#include <support.h>    // spi_wait_ready().  should perhaps move to spi.  no. because it's an peripheral abstraction


#include <device/4094-0.h>
#include <device/fpga0_reg.h>   // cs vec





#define UNUSED(x) ((void)(x))


static void setup(spi_t *spi )
{
  UNUSED(spi);


  // SPI1_CS2 should already have been setup by spi.
  // because it is used for configure.

  // assert(0);
}



static void port_configure( spi_t *spi_)
{
  assert(spi_);
  uint32_t spi = spi_->spi;
  assert(spi == SPI1);

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



static void cs_assert(spi_t *spi)
{
  // TODO magic
  assert(spi->spi == SPI1);
  spi_wait_ready( spi->spi);

  assert(SPI_CS_VEC_4094 == 2);
  gpio_write_with_mask( GPIOC, 7, 0b111, SPI_CS_VEC_4094);
}

static void cs_deassert(spi_t *spi)
{
  assert(spi->spi == SPI1);
  spi_wait_ready( spi->spi);

  gpio_write_with_mask( GPIOC, 7, 0b111, SPI_CS_VEC_DEASSERT);
}







spi_t * spi_4094_0_create( /* pass the spi_ice40 */  )
{
  /* called once at startup only, in main().
    it is really the malloc that buys us structure opaqueness.
    where opaqueness - is the header dependencies, and struct size needed to instantiate
    - only other way is to pull the structure in as a header.
    --------
  */
  spi_t *spi = malloc(sizeof( spi_t));
  assert(spi);
  memset(spi, 0, sizeof(spi_t));


  spi->spi    = SPI1;     // NOT sure if the spi should be passed in the contructor.
  spi->setup   =  setup;
  spi->port_configure = port_configure;
  spi->cs_assert    = cs_assert;
  spi->cs_deassert  = cs_deassert;


  return spi;
}



