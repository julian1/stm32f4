


#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>

#include <string.h>   // memset
#include <assert.h>




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



static void spi2_u202_cs( spi_ice40_t *spi, uint8_t val)
{
  assert(spi->spi == SPI2);

  spi_wait_ready( spi->spi);
  gpio_write_val( GPIOC, GPIO0, val);   // PC0
}

static void spi2_u202_rst( spi_ice40_t *spi, uint8_t val)
{
  assert(spi->spi == SPI2);

  // spi_wait_ready( spi->spi); not needed for rst
  gpio_write_val( GPIOC, GPIO6, val);   // PC6
}


static bool spi2_u202_cdone(spi_ice40_t *spi )
{
  assert(spi->spi == SPI2);
  return gpio_get(GPIOC, GPIO3) != 0;   // PC3
}



void spi2_u202_init( spi_ice40_t *spi)
{
  assert(spi);
  memset(spi, 0, sizeof(spi_ice40_t));

  spi->spi    = SPI2;
  spi->cs     = spi2_u202_cs;
  spi->rst    = spi2_u202_rst;
  spi->cdone  = spi2_u202_cdone;
}


