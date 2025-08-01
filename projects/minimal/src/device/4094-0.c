/*
  - fpga will invert the cs/strobe.


*/

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>


#include <stdio.h>
#include <string.h>   // memset
#include <assert.h>
#include <stdlib.h>




#include <peripheral/spi.h>
#include <support.h>    // spi_wait_ready().  should perhaps move to spi.  no. because it's an peripheral abstraction


#include <device/4094-0.h>


// copy from u102.c
#define SPI1_PORT       GPIOA

// #define SPI1_CS2        GPIO10      // control-panel-08
// #define SPI1_CS2        GPIO15     // gerber 257. control-panel-07

#define SPI1_CS2        GPIO10      // moved april 2025.



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

  // should always rest hi.
  assert( gpio_get( SPI1_PORT, SPI1_CS2) /*!= 0 */ );


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



static void cs( spi_t *spi, uint8_t val)
{
  assert(spi->spi == SPI1);

  // printf("4094-0 strobe %u\n", val );

  spi_wait_ready( spi->spi);
  gpio_write_val( SPI1_PORT, SPI1_CS2, val);
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

  // base
  spi->spi    = SPI1;     // NOT sure if the spi should be passed in the contructor.
  spi->cs     = cs;
  spi->setup   =  setup;
  spi->port_configure = port_configure;

  return spi;
}



