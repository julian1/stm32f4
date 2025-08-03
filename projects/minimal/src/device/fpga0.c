
// fpga device after configuration


#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>

#include <stdio.h>
#include <string.h>   // memset
#include <assert.h>
#include <stdlib.h>


#include <support.h>


#include <peripheral/spi.h>   // interface/abstraction
#include <device/fpga0.h>        // implementation/device
#include <device/fpga0_reg.h>   // cs vec



#define UNUSED(x) ((void)(x))





static void setup(spi_t *spi )
{
  UNUSED(spi);

  printf("u102 setup\n");


  // note GPIO7 was already set up in fpga0-pc.
  // potential for conflict


  // cs  PC7,8,9
  gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,          GPIO7 | GPIO8 | GPIO9);
  gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,  GPIO7 | GPIO8 | GPIO9);


  // interupt PA3
  // should not be done here.
  // gpio_mode_setup( GPIOA , GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO3 );

}



static void port_configure( spi_t *spi_)
{
  assert(spi_);
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
  // TODO add magic number?
  assert(spi->spi == SPI1);
  spi_wait_ready( spi->spi);

  assert(CS_VEC_FPGA0 == 1);
  gpio_write_with_mask( GPIOC, 7, 0b111, CS_VEC_FPGA0);
}

static void cs_deassert(spi_t *spi)
{
  assert(spi->spi == SPI1);
  spi_wait_ready( spi->spi);

  gpio_write_with_mask( GPIOC, 7, 0b111, CS_VEC_DEASSERT);
}




spi_t * spi_fpga0_create( )
{
  /* called once at startup only, in main().
    it is the malloc that buys us structure opaqueness.
    where opaqueness - is the header dependencies, and struct size needed to instantiate
    - only other way is to pull the structure in as a header.
    --------
  */

  spi_t *spi = malloc(sizeof(  spi_t));
  assert(spi);
  memset(spi, 0, sizeof(spi_t));

  spi->spi    = SPI1;
  spi->setup   =  setup;
  spi->port_configure = port_configure;
  spi->cs_assert    = cs_assert;
  spi->cs_deassert  = cs_deassert;

  // interupt not handled here

  return spi;
}



