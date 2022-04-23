/*
  Code is shared for both smu and adc
  should it be put in a shared library?
  no. because encodes GPIO pins.
*/

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>


#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>


#include <stddef.h>   // NULL

#include "spi1.h"

//////////////////////////


#define SPI_ICE40       SPI1

#define SPI_ICE40_PORT  GPIOA
#define SPI_ICE40_CS    GPIO4     // PA4
#define SPI_ICE40_CLK   GPIO5     // PA5
#define SPI_ICE40_MISO  GPIO6     // PA6
#define SPI_ICE40_MOSI  GPIO7     // PA7

#define SPI_ICE40_CS2   GPIO15   // PA15 nss 2. moved.

#define SPI_ICE40_INTERUPT GPIO2   // PA2

#define UNUSED(x) (void)(x)



void spi1_port_cs1_setup(void) // with CS.
{
  // rcc_periph_clock_enable(RCC_SPI1);

  // setup spi with cs ...
  uint16_t all = SPI_ICE40_CLK | SPI_ICE40_CS | SPI_ICE40_MOSI |  SPI_ICE40_MISO  ; // it doesn't matter to add input MISO

  gpio_mode_setup(SPI_ICE40_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, all);
  gpio_set_af(SPI_ICE40_PORT, GPIO_AF5, all); // af 5
  gpio_set_output_options(SPI_ICE40_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, all );

  // we should be able to simplify this. to non configured or input.
  // http://libopencm3.org/docs/latest/gd32f1x0/html/group__gpio__mode.html

  /*
  configure as if not configured. eg.
  ''During and just after reset, the alternate functions are not active and the I/O ports are configured in Input Floating mode (CNFx[1:0]=01b, MODEx[1:0]=00b).''
  */
  // set cs2 hi - with external pullup.
  gpio_mode_setup(SPI_ICE40_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SPI_ICE40_CS2);
}

void spi1_port_cs2_setup(void) // with CS2
{
  // rcc_periph_clock_enable(RCC_SPI1);

  // setup spi with cs ...
  uint16_t all = SPI_ICE40_CLK | SPI_ICE40_CS2 | SPI_ICE40_MOSI | SPI_ICE40_MISO; // it doesn't matter to add MISO to output

  gpio_mode_setup(SPI_ICE40_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, all);
  gpio_set_af(SPI_ICE40_PORT, GPIO_AF5, all); // af 5
  gpio_set_output_options(SPI_ICE40_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, all );

  // set cs1 hi - with external pullup.
  gpio_mode_setup(SPI_ICE40_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SPI_ICE40_CS);
}




/*
  this interupt can be for any kind of status change notice
  so, it's better to make a generalized func, rather than push this
  code into the ads131 code.
*/

static void (*spi1_interupt)(void *ctx) = NULL;
static void *spi1_ctx = NULL;


void exti2_isr(void)
{
  // interupt from ice40/fpga.

  /*
    EXTREME
    OK. bizarre. resetting immediately, prevents being called a second time
  */
  exti_reset_request(EXTI2);


  if(spi1_interupt) {
    spi1_interupt(spi1_ctx);
  }

}


/*
  need to separate out these functions
*/

void spi1_interupt_gpio_setup(void (*pfunc)(void *),  void *ctx)
{
  // TODO check non-null init args ...

  spi1_interupt = pfunc;
  spi1_ctx = ctx;

  gpio_mode_setup(SPI_ICE40_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SPI_ICE40_INTERUPT);

  // ie. use exti2 for pa2
  nvic_enable_irq(NVIC_EXTI2_IRQ);

  exti_select_source(EXTI2, SPI_ICE40_PORT);
  exti_set_trigger(EXTI2 , EXTI_TRIGGER_FALLING);
  exti_enable_request(EXTI2);
}


