
// should move to /device - because instance/implementation?


#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>

#include <string.h>   // memset
#include <assert.h>
#include <stdlib.h>


#include <support.h>


#include <peripheral/spi-ice40.h>   // interface/abstraction
#include <device/u102.h>        // implementation/device


// pulled from spi-port code.
#define SPI1_PORT       GPIOA
#define SPI1_CS1        GPIO4     // PA4
#define SPI1_CS2        GPIO15     // gerber157.
#define SPI1_INTERUPT   GPIO3     // PA3



static void cs( spi_ice40_t *spi, uint8_t val)
{
  assert(spi->spi == SPI1);

  spi_wait_ready( spi->spi);
  gpio_write_val( SPI1_PORT, SPI1_CS1, val);   // PC0
}

static void rst( spi_ice40_t *spi, uint8_t val)
{
  assert(spi->spi == SPI1);

  // spi_wait_ready( spi->spi); not needed for rst
  gpio_write_val( GPIOC, GPIO6, val);   // PC6

  /*
    OK. 
      CS2 works to assert reset, when CS1 is already lo.
      ONLY MANIPULATE CS2 here - relying on the OR gate to assert the reset, 
      not cs1, and cs2.
  */
  gpio_write_val( SPI1_PORT, SPI1_CS2, val);   // PC6
}


static bool cdone(spi_ice40_t *spi )
{
  assert(spi->spi == SPI1);

  return gpio_get(SPI1_PORT, SPI1_INTERUPT)  != 0;
}



static void setup(spi_ice40_t *spi )
{
  assert(spi->spi == SPI1);

#if 0
  // u102,
  // cs pc0
  // creset pc6
  gpio_mode_setup( GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO0 | GPIO6 );      // PC0,PC6
  gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO0 | GPIO6 );


  // input
  // cdone u202ca pc3.
  gpio_mode_setup(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO3);
#endif


  // CS1, CS2 to manual external gpio output
  gpio_mode_setup(SPI1_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI1_CS1 | SPI1_CS2);
  gpio_set_output_options(SPI1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI1_CS1 | SPI1_CS2);

  // shared for cdone/ interupt.
  gpio_mode_setup(SPI1_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SPI1_INTERUPT);
}





spi_ice40_t * spi_u102_create( )
{
  /* done once at startup.
    it is really the malloc that buys us structure opaqueness.
    - only other way is to pull the structure in as a header.
    --------
  */
  spi_ice40_t *p = malloc(sizeof(  spi_ice40_t));
  assert(p);
  memset(p, 0, sizeof(spi_ice40_t));

  p->spi    = SPI1;
  p->cs     = cs;
  p->rst    = rst;
  p->cdone  = cdone;
  p->setup  =  setup;

  return p;
}


