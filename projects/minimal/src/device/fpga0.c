
// device / instance/implementation


#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>

#include <stdio.h>
#include <string.h>   // memset
#include <assert.h>
#include <stdlib.h>


#include <support.h>


#include <peripheral/spi-ice40.h>   // interface/abstraction
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

  // perhaps

  // set reset, ss lo. before we enable outputs. to prevent ice40 assuming spi master
  // gpio_clear( GPIOC, CS /*| CS2 | CS3*/ );


  // deselect
  // gpio_set( GPIOC, CS /*| CS2 | CS3*/ );


  // TODO not clear we should init extra CS. here.
  // it is extra device funcionality.

  // cs  PC7
  gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO7 | GPIO8 | GPIO9);
  gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO7 | GPIO8 | GPIO9);


  // interupt PA3
  // should not be done here.
  // gpio_mode_setup( GPIOA , GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO3 );


  // cdone PE0
  gpio_mode_setup( GPIOE , GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO0 );



  // creset  PE1
  gpio_mode_setup(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1 );
  gpio_set_output_options(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO1 );


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






static void cs( spi_t *spi, uint8_t val)
{
  assert(spi->spi == SPI1);

  spi_wait_ready( spi->spi);
  // gpio_write_val( GPIOC, GPIO7, val);


  uint32_t shift = 7;     // PC7
  uint32_t mask = 0b111;  // 3 bits PC7,8,9
  gpio_write_with_mask( GPIOC, shift, mask, val);

}





static void rst( spi_ice40_t *spi, uint8_t val)
{
  assert(spi->spi == SPI1);


  // gpio_write_val( SPI1_PORT, SPI1_CS2, val);

  gpio_write_val( GPIOE, GPIO1, val);
}


static bool cdone(spi_ice40_t *spi )
{
  assert(spi->spi == SPI1);


  // return gpio_get(SPI1_PORT, SPI1_INT_CDONE)  != 0;
  return gpio_get(GPIOE , GPIO0)  != 0;
}






spi_ice40_t * spi_u102_create( )
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
  spi->spi    = SPI1;
  spi->cs     = cs;
  spi->setup   =  setup;
  spi->port_configure = port_configure;

  // derived stuff
  spi->rst    = rst;
  spi->cdone  = cdone;

  // interupt not

  return spi;
}



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


	// hold cs lines lo - to put fpga in reset, avoid isolator/fpga contention, because fpga wants to become spi master and drive spi lines.
  // probably ok, if just SS held lo.
  // there's a delay of about 1ms. though before ice40 samples inputs.
	// spi_port_cs2_enable( SPI1);
  // spi_port_cs1_enable( SPI1);


/*

  // rst is the derived function.  should not call from base.
  // EXTR.  just set values expliticcty.
  // gpio_clear( SPI1_PORT, SPI1_CS1 | SPI1_CS2 );


  spi->cs( spi, 0);
  spi->rst( spi, 0);
*/

