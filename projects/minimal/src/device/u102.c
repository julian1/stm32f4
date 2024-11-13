
// should move to /device - because instance/implementation?


#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>

#include <string.h>   // memset
#include <assert.h>
#include <stdlib.h>


#include <support.h>


#include <peripheral/spi-ice40.h>   // interface/abstraction
#include <device/u102.h>        // implementation/device




static void cs( spi_ice40_t *spi, uint8_t val)
{
  assert(spi->spi == SPI2);

  spi_wait_ready( spi->spi);
  gpio_write_val( GPIOC, GPIO0, val);   // PC0
}

static void rst( spi_ice40_t *spi, uint8_t val)
{
  assert(spi->spi == SPI2);

  // spi_wait_ready( spi->spi); not needed for rst
  gpio_write_val( GPIOC, GPIO6, val);   // PC6
}


static bool cdone(spi_ice40_t *spi )
{
  assert(spi->spi == SPI2);
  return gpio_get(GPIOC, GPIO3) != 0;   // PC3
}



static void setup(spi_ice40_t *spi )    // rename port() ?.
{
  assert(spi->spi == SPI2);

  // u202,
  // cs pc0
  // creset pc6
  gpio_mode_setup( GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO0 | GPIO6 );      // PC0,PC6
  gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO0 | GPIO6 );


  // input
  // cdone u202ca pc3.
  gpio_mode_setup(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO3);
}





/*

  now we need spi functions. to read/write registers.
  these are not u202. specific.

  they are generic.

*/


static void init( spi_ice40_t *spi)
{
  assert(spi);
  memset(spi, 0, sizeof(spi_ice40_t));

  spi->spi    = SPI2;   // could move this into the setup() func
  spi->cs     = cs;
  spi->rst    = rst;
  spi->cdone  = cdone;
  spi->setup   =  setup;
  // spi->config =  config;
}




/*
  func has to return a pointer, not take a pointer.
  to be opaque.  which requires calling malloc() .
  but only needs to be done once at startup.
  -  otherwise would have to instantiate in main.c

*/



spi_ice40_t * spi_u102_create( )
{
  /* done once at startup.
    it is really the malloc that buys us structure opaqueness.
    - only other way is to pull the structure in as a header.
    --------
  */
  spi_ice40_t *p = malloc(sizeof(  spi_ice40_t));
  assert(p);
  init(p);
  return p;
}


// Hmmmm

/*

  Hang on.  the u202. can setup the cs,rst,cdone lines.


*/



