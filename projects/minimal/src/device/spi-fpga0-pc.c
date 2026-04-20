
/*
// device / instance/implementation
  fpga-pc   -  fpga pre bitstream configuration
*/

#include <stdio.h>
#include <string.h>   // memcpy
#include <assert.h>


#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>


#include <peripheral/spi-ice40-pc.h>   // interface/abstraction
#include <device/spi-fpga0-pc.h>        // implementation/device
#include <device/support.h>


#define FPGA0_MAGIC   834234234


static void setup(spi_t *spi )
{
  assert(spi && spi->magic == FPGA0_MAGIC);


  printf("fpga0 pc cs\n");

  /*
    - remember before fpga configuration. only one spi cs line is active
    - we will reconfigure this again for the normal spi. device
  */

  // cs  PC7
  gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO7 );
  gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO7 );


  // cdone PE0 input
  gpio_mode_setup( GPIOE , GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO0 );


  // creset  PE1
  gpio_mode_setup(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1 );
  gpio_set_output_options(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO1 );
}


static void port_configure( spi_t *spi_)
{

  assert(spi_ && spi_->magic == FPGA0_MAGIC);

  uint32_t spi = spi_->spi;

  assert(spi == SPI1 || spi == SPI2);

  spi_reset( spi );

  /*
    - assert mosi on neg edge.  ice40 reads on pos edge.

    the 49 additional clock cycles is useful-  since it clearly demarcates where configuration ends, and when spi becomes active with user comms.
    actually cs is high already by this time. so it doesn't matter too much.

    - mini-grabbers on soic is a problem, because they end up touching eash other, giving wrong signals.
  */

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

  spi_enable( spi );

}


static void cs_assert(spi_t *spi)
{
  assert(spi && spi->magic == FPGA0_MAGIC);
  assert(spi->spi == SPI1);
  /*
    - remember before configuration. only one spi cs line is active
  */
  spi_wait_ready( spi->spi);
  gpio_write_val( GPIOC, GPIO7, 0);

}

static void cs_deassert(spi_t *spi)
{
  assert(spi && spi->magic == FPGA0_MAGIC);

  spi_wait_ready( spi->spi);
  gpio_write_val( GPIOC, GPIO7, 1);
}



static void rst( spi_ice40_t *spi, uint8_t val)
{
  assert(spi && spi->magic == FPGA0_MAGIC);
  assert(spi->spi == SPI1);

  // gpio_write_val( SPI1_PORT, SPI1_CS2, val);
  gpio_write_val( GPIOE, GPIO1, val);
}


static bool cdone(spi_ice40_t *spi )
{
  assert(spi && spi->magic == FPGA0_MAGIC);
  assert(spi->spi == SPI1);


  // return gpio_get(SPI1_PORT, SPI1_INT_CDONE)  != 0;
  return gpio_get( GPIOE, GPIO0) != 0;
}


void spi_fpga0_pc_init( spi_ice40_t *spi)
{
  assert( spi);

  const spi_ice40_t temp = {

    .magic          = FPGA0_MAGIC,

    // base
    .spi            = SPI1,
    .setup          = setup,
    .port_configure = port_configure,
    .cs_assert      = cs_assert,
    .cs_deassert    = cs_deassert,

    // derived
    .rst    = rst,
    .cdone  = cdone,
  };

  memcpy( spi, &temp, sizeof( spi_ice40_t));
}





/*
  // pulled from old spi-port code.
  #define SPI1_PORT       GPIOA
  // #define SPI1_CS1        GPIO4     // PA4
  #define SPI1_CS1        GPIO8     // moved. april. 2025.

  // change in 4094-0.c also
  #define SPI1_CS2        GPIO10      // moved april 2025.
  // #define SPI1_CS2        GPIO15     // gerber 257. control-panel-07

  #define SPI1_INT_CDONE   GPIO3     // PA3  shared for cdone/ and interrupt
*/

