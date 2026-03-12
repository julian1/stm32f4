
// should move to /device - because instance/implementation?



#include <string.h>   // memset
#include <assert.h>
#include <stdlib.h>


#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>



#include <peripheral/spi-ice40-pc.h>    // interface/abstraction
#include <device/spi-fpga1-pc.h>           // implementation/device

#include <device/support.h>             // spi_wait_ready(), write_val();


#define FPGA1_MAGIC   77994234





static void setup( spi_t *spi)    // rename port() ?.
{
  assert(spi && spi->magic == FPGA1_MAGIC);

  printf("fpga1 setup pc cs\n");

  // set reset, ss lo. before we configure. to prevent ice40 assuming spi master
  gpio_clear( GPIOC, GPIO0 | GPIO6);


  // u202,
  // spi2 cs pc0
  gpio_mode_setup( GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO0);
  gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO0);

  // cdone pc3 input
  gpio_mode_setup(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO3);

  // creset pc6
  gpio_mode_setup( GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO6);
  gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO6);
}


static void port_configure( spi_t *spi_)
{
  //  this is device specific. so belongs on the device structure
  // taken from,  void spi_mux_ice40(uint32_t spi) in spi-ice40.c

  assert(spi_ && spi_->magic == FPGA1_MAGIC);

  uint32_t spi = spi_->spi;
  assert(spi == SPI2);

  spi_reset( spi );

  // note clk_to_0_when idle. during pc. but not in normal use.

  spi_init_master(
    spi,
    // SPI_CR1_BAUDRATE_FPCLK_DIV_2,  // div2 seems to work with cap iso, but not with adum. actually misses a few bits with cap iso.
//    SPI_CR1_BAUDRATE_FPCLK_DIV_4,
    SPI_CR1_BAUDRATE_FPCLK_DIV_16,
    // SPI_CR1_BAUDRATE_FPCLK_DIV_32,
    // SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,  // park to 0/lo == positive clok edge. park to 1 == negative clk edge.
    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,  // park to 0/lo == positive clok edge. park to 1 == negative clk edge.
    SPI_CR1_CPHA_CLK_TRANSITION_1,    // 1 == leading edge,  2 == falling edge
    SPI_CR1_DFF_8BIT,
    SPI_CR1_MSBFIRST
  );


#if 0
  review - old init code used cpol_clk_to_1.

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
#endif

  spi_enable( spi );
}


static void cs_assert(spi_t *spi)
{
  assert(spi && spi->magic == FPGA1_MAGIC);
  assert(spi->spi == SPI2);

  spi_wait_ready( spi->spi);
  gpio_write_val( GPIOC, GPIO0, 0);
}

static void cs_deassert(spi_t *spi)
{
  assert(spi && spi->magic == FPGA1_MAGIC);

  spi_wait_ready( spi->spi);
  gpio_write_val( GPIOC, GPIO0, 1);
}


static void rst( spi_ice40_t *spi, uint8_t val)
{
  assert(spi && spi->magic == FPGA1_MAGIC);
  assert(spi->spi == SPI2);

  // spi_wait_ready( spi->spi); not needed for rst
  gpio_write_val( GPIOC, GPIO6, val);   // PC6
}


static bool cdone(spi_ice40_t *spi )
{
  assert(spi && spi->magic == FPGA1_MAGIC);
  assert(spi->spi == SPI2);
  return gpio_get(GPIOC, GPIO3) != 0;   // PC3
}






spi_ice40_t * spi_fpga1_pc_new()
// spi_ice40_t * spi2_u202_new()
{
  /* called once at startup only, in main().
    it is really the malloc that buys us structure opaqueness.
    where opaqueness - is the header dependencies, and struct size needed to instantiate
    - only other way is to pull the structure in as a header.
    --------
  */
  spi_ice40_t *spi = malloc(sizeof(  spi_ice40_t));
  assert(spi);
  memset(spi, 0, sizeof(spi_ice40_t));

  spi->magic          = FPGA1_MAGIC;

  // base
  spi->spi            = SPI2;
  spi->setup          =  setup;
  spi->port_configure = port_configure;
  spi->cs_assert      = cs_assert;
  spi->cs_deassert    = cs_deassert;

  // derived stuff
  spi->rst    = rst;
  spi->cdone  = cdone;

  return spi;
}


