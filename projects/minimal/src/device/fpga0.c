
// device / instance/implementation


#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>

#include <stdio.h>
#include <string.h>   // memset
#include <assert.h>
#include <stdlib.h>


#include <support.h>


#include <peripheral/spi.h>   // interface/abstraction
#include <device/fpga0.h>        // implementation/device

/*
// pulled from spi-port code.
#define SPI1_PORT       GPIOA
// #define SPI1_CS1        GPIO4     // PA4
#define SPI1_CS1        GPIO8     // moved. april. 2025.

// change in 4094-0.c also
#define SPI1_CS2        GPIO10      // moved april 2025.
// #define SPI1_CS2        GPIO15     // gerber 257. control-panel-07

#define SPI1_INT_CDONE   GPIO3     // PA3  shared for cdone/ and interrupt
*/


#define UNUSED(x) ((void)(x))






static void setup(spi_t *spi )
{
  UNUSED(spi);

  printf("u102 setup\n");

  // set reset, ss lo. before we enable outputs. to prevent ice40 assuming spi master
  // gpio_clear( GPIOC, CS /*| CS2 | CS3*/ );


  // cs  PC7,8,9
  gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,          GPIO7 | GPIO8 | GPIO9);
  gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,  GPIO7 | GPIO8 | GPIO9);


  // interupt PA3
  // should not be done here.
  // gpio_mode_setup( GPIOA , GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO3 );

}



static void port_configure( spi_t *spi_)
{
  //  this is device specific. so belongs on the device structure
  // taken from,  void spi_mux_ice40(uint32_t spi) in spi-ice40.c

  assert(spi_);
  uint32_t spi = spi_->spi;

  assert(spi == SPI1);

  spi_reset( spi );

  // consider could/should assert()) SS is HI/disabled here.
  // TODO. does not belong here. anymore.

  // assert( gpio_get( SPI1_PORT, SPI1_CS1) != 0  );


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




#if 0
static void cs( spi_t *spi, uint8_t val)
{
  assert(spi->spi == SPI1);
  spi_wait_ready( spi->spi);

  // gpio_write_val( GPIOC, GPIO7, val);

  // this extended cs - is not a property of the fpga before configuration
  // the difficulty is that this stuff is shared - for all spi devices. and probably only wants to be done once.

/*
  uint32_t shift = 7;     // PC7
  uint32_t mask = 0b111;  // 3 bits PC7,8,9
  gpio_write_with_mask( GPIOC, shift, mask, val);
*/
  assert( val == 0 || val == 1);

  if(val == 0)  // assert
    gpio_write_with_mask( GPIOC, 7, 0b111, 1 );      // virtual device  == 1
  else          // deassert
    gpio_write_with_mask( GPIOC, 7, 0b111, 0 );      // v
}

#endif


static void cs_assert(spi_t *spi)
{
  // TODO magic
  assert(spi->spi == SPI1);
  spi_wait_ready( spi->spi);

  gpio_write_with_mask( GPIOC, 7, 0b111, 1 );      // virtual device  == 1
}

static void cs_deassert(spi_t *spi)
{
  assert(spi->spi == SPI1);
  spi_wait_ready( spi->spi);

  gpio_write_with_mask( GPIOC, 7, 0b111, 0 );      // clear virtual device
}








spi_t * spi_u102_create( )
{
  /* called once at startup only, in main().
    it is really the malloc that buys us structure opaqueness.
    where opaqueness - is the header dependencies, and struct size needed to instantiate
    - only other way is to pull the structure in as a header.
    --------
  */
  // spi_ice40_t *spi = malloc(sizeof(  spi_ice40_t));
  spi_t *spi = malloc(sizeof(  spi_t));
  assert(spi);
  memset(spi, 0, sizeof(spi_t));

  // base
  spi->spi    = SPI1;
  spi->cs_assert    = cs_assert;
  spi->cs_deassert  = cs_deassert;

  spi->setup   =  setup;
  spi->port_configure = port_configure;


  // interupt not

  return spi;
}



