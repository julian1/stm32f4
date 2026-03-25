
// fpga device after configuration


#include <stdio.h>
#include <string.h>   // memset
#include <assert.h>
#include <stdlib.h>


#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>




#include <peripheral/spi.h>           // interface/abstraction
#include <device/spi-fpga0.h>         // implementation/device
#include <device/spi-fpga0-reg.h>     // cs vec
#include <device/support.h>




#define FPGA0_MAGIC   9121214


static void setup(spi_t *spi )
{
  assert(spi && spi->magic == FPGA0_MAGIC);

  printf("fpga0/u102 setup all 3 cs lines\n");


  /* note GPIO7 was already set up in fpga0-pc.
    potential for conflict
  */

  // cs  PC7,8,9
  gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,          GPIO7 | GPIO8 | GPIO9);
  gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,  GPIO7 | GPIO8 | GPIO9);
}



static void port_configure( spi_t *spi_)
{
  assert(spi_ && spi_->magic == FPGA0_MAGIC);

  uint32_t spi = spi_->spi;
  assert(spi == SPI1);

  spi_reset( spi );


  spi_init_master(
    spi,
    // SPI_CR1_BAUDRATE_FPCLK_DIV_2,  // div2 seems to work with iso, but not adum. actually misses a few bits with iso.
    SPI_CR1_BAUDRATE_FPCLK_DIV_4,
    // SPI_CR1_BAUDRATE_FPCLK_DIV_16,
    // SPI_CR1_BAUDRATE_FPCLK_DIV_32,
    SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,  // park to 0/lo == positive clok edge. park to 1 == negative clk edge.
    SPI_CR1_CPHA_CLK_TRANSITION_1,    // 1 == leading edge,  2 == falling edge
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST
  );

  spi_enable( spi );
}




static void cs_assert(spi_t *spi)
{
  assert(spi && spi->magic == FPGA0_MAGIC);

  // TODO add magic number?
  assert(spi->spi == SPI1);
  spi_wait_ready( spi->spi);

  assert(SPI_CS_FPGA0 == 1);
  gpio_write_with_mask( GPIOC, 7, 0b111, SPI_CS_FPGA0);
}

static void cs_deassert(spi_t *spi)
{
  assert(spi && spi->magic == FPGA0_MAGIC);

  assert(spi->spi == SPI1);
  spi_wait_ready( spi->spi);

  gpio_write_with_mask( GPIOC, 7, 0b111, SPI_CS_DEASSERT);
}


void spi_fpga0_init( spi_t *spi)
{
  assert(spi);
  memset(spi, 0, sizeof(spi_t));

  spi->magic          = FPGA0_MAGIC;

  spi->spi            = SPI1;
  spi->setup          =  setup;
  spi->port_configure = port_configure;
  spi->cs_assert      = cs_assert;
  spi->cs_deassert    = cs_deassert;
}

