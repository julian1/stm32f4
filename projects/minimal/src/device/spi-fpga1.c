
// fpga device after configuration


#include <stdio.h>
#include <assert.h>


#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>


#include <peripheral/spi.h>           // interface/abstraction
#include <device/spi-fpga1.h>         // implementation/device
#include <device/support.h>


#define FPGA1_MAGIC   77122877


static void port_configure(spi_t *spi )
{
  assert(spi && spi->magic == FPGA1_MAGIC);

  printf("fpga1/u202 port configure\n");


  /* note was already set up in spi-fpga1-pc.
    potential for conflict
  */

  // spi2 cs pc0
  gpio_mode_setup( GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO0);
  gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO0);
}


static void controller_configure( spi_t *spi_)
{
  assert(spi_ && spi_->magic == FPGA1_MAGIC);

  uint32_t spi = spi_->spi;

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
  assert(spi && spi->magic == FPGA1_MAGIC);

  spi_wait_ready( spi->spi);
  gpio_write_val( GPIOC, GPIO0, 0);
}


static void cs_deassert(spi_t *spi)
{
  assert(spi && spi->magic == FPGA1_MAGIC);

  spi_wait_ready( spi->spi);
  gpio_write_val( GPIOC, GPIO0, 1);
}



void spi_fpga1_init( spi_t *spi)
{
  assert(spi);

  *spi = (const spi_t) {

    .magic          = FPGA1_MAGIC,
    .spi            = SPI2,
    .port_configure = port_configure,
    .controller_configure = controller_configure,
    .cs_assert      = cs_assert,
    .cs_deassert    = cs_deassert,
  };

}

