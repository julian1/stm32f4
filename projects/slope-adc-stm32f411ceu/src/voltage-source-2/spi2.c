
#include "spi2.h"
#include "assert.h"
#include "streams.h"   // usart1_printf


#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>


// NO. we can do this...


#define SPI1_PORT  GPIOA
#define SPI1_CS    GPIO4     // PA4
#define SPI1_CLK   GPIO5     // PA5
#define SPI1_MISO  GPIO6     // PA6
#define SPI1_MOSI  GPIO7     // PA7

#define SPI1_CS2   GPIO15   // PA15 nss 2. moved.

// #define SPI1_INTERUPT GPIO2   // PA2
// #define UNUSED(x) (void)(x)





static void spi1_port_cs1_setup(void)
{
  // rcc_periph_clock_enable(RCC_SPI1);

  // setup spi with cs ...
  uint16_t out = SPI1_CLK | SPI1_CS | SPI1_MOSI ; // not MISO
  uint16_t all = out | SPI1_MISO;

  gpio_mode_setup(SPI1_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, all);
  gpio_set_af(SPI1_PORT, GPIO_AF5, all); // af 5
  gpio_set_output_options(SPI1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, out);

  // cs2 left in whatever condition.
}


static void spi1_port_cs2_setup(void) // with CS2
{
  // rcc_periph_clock_enable(RCC_SPI1);

  // setup spi with cs ...
  uint16_t out = SPI1_CLK /* | SPI1_CS2 */  | SPI1_MOSI ; // not MISO
  uint16_t all = out | SPI1_MISO ;

  gpio_mode_setup(SPI1_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, all);
  gpio_set_af(SPI1_PORT, GPIO_AF5, all); // af 5
  gpio_set_output_options(SPI1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, out); // probably need to reset each time.

  // set cs1 hi-z/hi - with external pullup.
  gpio_mode_setup(SPI1_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SPI1_CS);

  // set CS2 to high-z input with external gpio. only should only do once.
  gpio_mode_setup(SPI1_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI1_CS2);
  gpio_set_output_options(SPI1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, SPI1_CS2);
}


// note AF5. also. like spi1.
#define SPI2_PORT    GPIOB
#define SPI2_CS      GPIO12
#define SPI2_CLK    GPIO13
#define SPI2_MISO    GPIO14
#define SPI2_MOSI    GPIO15
#define SPI2_CS2     GPIO9
// second gpio is GPIO10 / interupt.


/*
  this is basically for the 4096 which needs cs2 in strobe mode. and dac8734
  these are poorly named.
  perhaps should be spi_dac_mode().  spi_4096_mode().
*/

static void spi2_port_cs1_setup(void)
{
  // rcc_periph_clock_enable(RCC_SPI2);

  // setup spi with cs ...
  uint16_t out = SPI2_CLK | SPI2_CS | SPI2_MOSI ; // not MISO
  uint16_t all = out | SPI2_MISO;

  gpio_mode_setup(SPI2_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, all);
  gpio_set_af(SPI2_PORT, GPIO_AF5, all); // af 5
  gpio_set_output_options(SPI2_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, out);

  // cs2 left in whatever condition.
}


static void spi2_port_cs2_setup(void) // with CS2
{
  // rcc_periph_clock_enable(RCC_SPI2);

  // setup spi with cs ...
  uint16_t out = SPI2_CLK /* | SPI2_CS2 */  | SPI2_MOSI ; // not MISO
  uint16_t all = out | SPI2_MISO ;

  gpio_mode_setup(SPI2_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, all);
  gpio_set_af(SPI2_PORT, GPIO_AF5, all); // af 5
  gpio_set_output_options(SPI2_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, out); // probably need to reset each time.

  // set cs1 hi-z/hi - with external pullup.
  gpio_mode_setup(SPI2_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SPI2_CS);

  // set CS2 to high-z input with external gpio. only should only do once.
  gpio_mode_setup(SPI2_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI2_CS2);
  gpio_set_output_options(SPI2_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, SPI2_CS2);
}








void spi_port_cs1_setup(uint32_t spi)
{
  switch(spi) {
    case SPI1: spi1_port_cs1_setup(); break;
    case SPI2: spi2_port_cs1_setup(); break;
    default: assert(0);
  }
}


void spi_port_cs2_setup(uint32_t spi )
{
  switch(spi) {
    case SPI1: spi1_port_cs2_setup(); break;
    case SPI2: spi2_port_cs2_setup(); break;
    default: assert(0);
  }
}



static void spi2_cs2_set(void)
{
  gpio_set(SPI2_PORT, SPI2_CS2);
}

static void spi2_cs2_clear(void)
{
  gpio_clear(SPI2_PORT, SPI2_CS2);
}




void spi_cs2_set(uint32_t spi)
{
  switch(spi) {
    case SPI2: spi2_cs2_set(); break;
    default: assert(0);
  }
}


void spi_cs2_clear(uint32_t spi)
{
  switch(spi) {
    case SPI2: spi2_cs2_clear(); break;
    default: assert(0);
  }
}







// change name assert
// should be called stobe.    spi1_strobe_assert()
// no. keep separate. strobe has meaning for 4094. strobe...

/*
    This is all really damn messy.
    should be able to switch regardless of the spi used.

    So that peripherals are not dependent .
*/


  // we should be able to simplify this. to non configured or input.
  // http://libopencm3.org/docs/latest/gd32f1x0/html/group__gpio__mode.html

#if 0
  /*

  ''During and just after reset, the alternate functions are not active and the I/O ports are configured in Input Floating mode (CNFx[1:0]=01b, MODEx[1:0]=00b).''
  */
  // set cs2 hi - with external pullup.
  gpio_mode_setup(SPI1_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SPI1_CS2);
#endif

