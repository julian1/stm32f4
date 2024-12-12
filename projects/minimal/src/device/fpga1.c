
// should move to /device - because instance/implementation?


#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>

#include <string.h>   // memset
#include <assert.h>
#include <stdlib.h>




#include <support.h>      // spi_wait_ready(), write_val();

#include <peripheral/spi-ice40.h>   // interface/abstraction


#include <device/fpga1.h>        // implementation/device

#define UNUSED(x) ((void)(x))




static void setup( spi_t *spi )    // rename port() ?.
{
  UNUSED(spi);


  // set reset, ss lo. before we configure. to prevent ice40 assuming spi master
  gpio_clear( GPIOC, GPIO0 | GPIO6);


  // u202,
  // cs pc0
  // creset pc6
  gpio_mode_setup( GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO0 | GPIO6 );      // PC0,PC6
  gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO0 | GPIO6 );


  // input
  // cdone u202ca pc3.
  gpio_mode_setup(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO3);
}




static void port_configure( spi_t *spi_)
{
  //  this is device specific. so belongs on the device structure
  // taken from,  void spi_mux_ice40(uint32_t spi) in spi-ice40.c

  assert(spi_);
  uint32_t spi = spi_->spi;
  assert(spi == SPI2);

  spi_reset( spi );

  // consider could/should assert()) SS is HI/disabled here.
  assert( gpio_get( GPIOC, GPIO0) != 0  );


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




static void cs( spi_t *spi, uint8_t val)
{
  assert(spi->spi == SPI2);

  spi_wait_ready( spi->spi);
  gpio_write_val( GPIOC, GPIO0, val);   // PC0
}





//////////////////





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






spi_ice40_t * spi2_u202_create()
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

  // base
  spi->spi    = SPI2;
  spi->setup   =  setup;
  spi->port_configure = port_configure;
  spi->cs     = cs;

  // derived stuff
  spi->rst    = rst;
  spi->cdone  = cdone;

  return spi;
}





//////////////



/*
  - actual instantiation of data structure (eg. actual memory requirement) should be done here.
    or else in main/app.
  and should not exposed anywhere else.

  - this means returning a pointer. althouth we prefer to not use malloc.

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




/*

  now we need spi functions. to read/write registers.
  these are not u202. specific.

  they are generic.

*/



// Hmmmm

/*

  Hang on.  the u202. can setup the cs,rst,cdone lines.
*/


// interupt could go here also.


