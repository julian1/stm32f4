

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>


#include "spi1.h"

//////////////////////////

// BELONGS in its own file  spi1.c


// fairly application specific but that's ok.

#define SPI_ICE40       SPI1

#define SPI_ICE40_PORT  GPIOA
#define SPI_ICE40_CLK   GPIO5     // PA5
#define SPI_ICE40_CS    GPIO4     // PA4
#define SPI_ICE40_MOSI  GPIO7     // PA7
#define SPI_ICE40_MISO  GPIO6     // PA6

// output reg.
#define SPI_ICE40_SPECIAL GPIO3   // PA4


#define UNUSED(x) (void)(x)



void spi1_port_setup(void)
{
  // same...
  uint16_t out = SPI_ICE40_CLK | SPI_ICE40_CS | SPI_ICE40_MOSI ; // not MISO
  uint16_t all = out | SPI_ICE40_MISO;

  // rcc_periph_clock_enable(RCC_SPI1);

  gpio_mode_setup(SPI_ICE40_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, all);
  gpio_set_af(SPI_ICE40_PORT, GPIO_AF5, all); // af 5
  gpio_set_output_options(SPI_ICE40_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, out);

}



void spi1_special_gpio_setup(void)
{

  // special
  gpio_mode_setup(SPI_ICE40_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI_ICE40_SPECIAL);
  gpio_set_output_options(SPI_ICE40_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI_ICE40_SPECIAL);

  gpio_set(SPI_ICE40_PORT, SPI_ICE40_SPECIAL ); // hi == off, active low...

}


void spi_special_flag_clear(uint32_t spi)
{
  UNUSED(spi);
  gpio_clear(SPI_ICE40_PORT, SPI_ICE40_SPECIAL ); // assert special, active low...
}


void spi_special_flag_set(uint32_t spi)
{
  UNUSED(spi);
  gpio_set(SPI_ICE40_PORT, SPI_ICE40_SPECIAL );
}


