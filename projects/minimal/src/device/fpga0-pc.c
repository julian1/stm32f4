
// device / instance/implementation


#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>

#include <stdio.h>
#include <string.h>   // memset
#include <assert.h>
#include <stdlib.h>


#include <support.h>


#include <peripheral/spi-ice40.h>   // interface/abstraction
#include <device/fpga0-pc.h>        // implementation/device

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

  // cs  PC7
  gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO7 /*| GPIO8 | GPIO9*/);
  gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO7 /*| GPIO8 | GPIO9*/);



  // cdone PE0
  gpio_mode_setup( GPIOE , GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO0 );



  // creset  PE1
  gpio_mode_setup(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1 );
  gpio_set_output_options(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO1 );


}




static void port_configure( spi_t *spi_)
{
  assert(spi_);
  uint32_t spi = spi_->spi;


  assert(spi == SPI1 || spi == SPI2);


  spi_reset( spi );

  /*
    - assert mosi on neg edge.  ice40 reads on pos edge.

    the 49 additional clock cycles is useful-  since it clearly demarcates where configuration ends, and when spi becomes active with user comms.
    actually cs is high already by this time. so it doesn't matter too much.

    - mini-grabbers on soic is a problem, because they end up touching eash other, giving wrong signals.
  */



  spi_init_master(
    spi,
    // SPI_CR1_BAUDRATE_FPCLK_DIV_2,  // div2 seems to work with iso, but not adum. actually misses a few bits with iso.
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



static void cs( spi_t *spi, uint8_t val)
{
  // normal pin control of cs

  assert(spi->spi == SPI1);

  spi_wait_ready( spi->spi);
  gpio_write_val( GPIOC, GPIO7, val);

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






spi_ice40_t * spi_u102_pc_create( )
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




