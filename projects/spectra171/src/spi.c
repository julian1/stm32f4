
#include "spi.h"
#include "assert.h"
#include "streams.h"   // usart_printf


#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>



#define SPI_ICE40       SPI1

#define SPI_ICE40_PORT  GPIOA
#define SPI_ICE40_CS    GPIO4     // PA4
#define SPI_ICE40_CLK   GPIO5     // PA5
#define SPI_ICE40_MISO  GPIO6     // PA6
#define SPI_ICE40_MOSI  GPIO7     // PA7

#define SPI_ICE40_CS2   GPIO15   // PA15 nss 2. moved.

// #define SPI_ICE40_INTERUPT GPIO2   // PA2

// #define UNUSED(x) (void)(x)





void spi1_port_cs1_setup(void)
{
  // rcc_periph_clock_enable(RCC_SPI1);

  // setup spi with cs ...
  uint16_t out = SPI_ICE40_CLK | SPI_ICE40_CS | SPI_ICE40_MOSI ; // not MISO
  uint16_t all = out | SPI_ICE40_MISO;

  gpio_mode_setup(SPI_ICE40_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, all);
  gpio_set_af(SPI_ICE40_PORT, GPIO_AF5, all); // af 5
  gpio_set_output_options(SPI_ICE40_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, out);

  // we should be able to simplify this. to non configured or input.
  // http://libopencm3.org/docs/latest/gd32f1x0/html/group__gpio__mode.html

#if 0
  /*

  ''During and just after reset, the alternate functions are not active and the I/O ports are configured in Input Floating mode (CNFx[1:0]=01b, MODEx[1:0]=00b).''
  */
  // set cs2 hi - with external pullup.
  gpio_mode_setup(SPI_ICE40_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SPI_ICE40_CS2);
#endif
}







void spi1_port_cs2_setup(void) // with CS2
{
  // rcc_periph_clock_enable(RCC_SPI1);

  // setup spi with cs ...
  uint16_t out = SPI_ICE40_CLK /* | SPI_ICE40_CS2 */  | SPI_ICE40_MOSI ; // not MISO
  uint16_t all = out | SPI_ICE40_MISO ;

  gpio_mode_setup(SPI_ICE40_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, all);
  gpio_set_af(SPI_ICE40_PORT, GPIO_AF5, all); // af 5
  gpio_set_output_options(SPI_ICE40_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, out); // probably need to reset each time.

  // set cs1 hi-z/hi - with external pullup.
  gpio_mode_setup(SPI_ICE40_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SPI_ICE40_CS);

  // set CS2 to high-z input with external gpio. only should only do once.
  gpio_mode_setup(SPI_ICE40_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI_ICE40_CS2);
  gpio_set_output_options(SPI_ICE40_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, SPI_ICE40_CS2);
}




// change name assert
// should be called stobe.    spi1_strobe_assert()
// no. keep separate. strobe has meaning for 4094. strobe...

void spi1_cs2_set(void)
{
  gpio_set(SPI_ICE40_PORT, SPI_ICE40_CS2);
}

void spi1_cs2_clear(void)
{
  gpio_clear(SPI_ICE40_PORT, SPI_ICE40_CS2);
}






