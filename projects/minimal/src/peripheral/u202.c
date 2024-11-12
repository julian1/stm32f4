


#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>

#include <string.h>   // memset
#include <assert.h>
#include <stdlib.h>




#include <gpio.h>


#include <peripheral/u202.h>


/* 
  - actual instantiation of data structure (eg. actual memory requirement) should be done here.
    or else in main/app. 
  and should not exposed anywhere else.

  - this means returning a pointer. but we really dont want to use malloc.

  - NO. instantiate  in main.c.    and reference pointer
  or including this filei.   in the app.c
  ----------

  Do if we did distinguish between spi controller, and spi peripheral.
  Then could organize the port_config()  more effectively.

  - 


*/

// this isn't really spi2 cs.  it is specifially u202.
// BUT we could name it u202.


/*
  - advantage of using functions. is don't have to expose  gpio. port and pin detail. 
  - enough function to program fpga.
  ----------

  this is device specific. not a spi port viewed from mcu side.
  should move to separate file.
*/



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



static void setup(spi_ice40_t *spi )
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


static void init( spi_ice40_t *spi)
{
  assert(spi);
  memset(spi, 0, sizeof(spi_ice40_t));

  spi->spi    = SPI2;   // could move this into the setup() func
  spi->cs     = cs;
  spi->rst    = rst;
  spi->cdone  = cdone;
  spi->setup   =  setup;
}






spi_ice40_t * spi2_u202_create()
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



