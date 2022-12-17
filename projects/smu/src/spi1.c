/*
  rename.
  Code is shared for smu and adc
  should it be put in a shared library?

*/

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>


#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>


#include <stddef.h>   // NULL
#include <assert.h>

#include "spi1.h"

#include "util.h"   // critical_error_blink()

//////////////////////////

// BELONGS in its own file  spi1.c


// fairly application specific but that's ok.

#define SPI_ICE40       SPI1

#define SPI_ICE40_PORT  GPIOA
#define SPI_ICE40_CLK   GPIO5     // PA5
#define SPI_ICE40_CS    GPIO4     // PA4
#define SPI_ICE40_MOSI  GPIO7     // PA7
#define SPI_ICE40_MISO  GPIO6     // PA6



#define SPI_ICE40_CS2   GPIO15   // PA15 nss 2. moved.

#if 0
// output reg.
#define SPI_ICE40_SPECIAL GPIO3   // PA4
#endif

#define SPI_ICE40_INTERUPT GPIO2   // PA2

#define UNUSED(x) (void)(x)



#define SPI1_PORT  GPIOA
#define SPI1_CS2   GPIO15   // PA15 nss 2. moved.




void spi_strobe_assert( uint32_t spi)
{
  // remember relies on port with CS2 configured as gpio.

  switch(spi) {
    case SPI1: {

      gpio_set(SPI1_PORT, SPI1_CS2);

      for(uint32_t i = 0; i < 100; ++i)
         __asm__("nop");

      gpio_clear(SPI1_PORT, SPI1_CS2);
      break;
    }

    default:
      assert(0);
      critical_error_blink();

  }
}








// TODO rename
// static void spi1_port_cs1_setup(void)

void spi1_port_setup(void)
{
  // rcc_periph_clock_enable(RCC_SPI1);

  // setup spi with cs ...
  uint16_t out = SPI_ICE40_CLK | SPI_ICE40_CS | SPI_ICE40_MOSI ; // not MISO
  uint16_t all = out | SPI_ICE40_MISO;

  gpio_mode_setup(SPI_ICE40_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, all);
  gpio_set_af(SPI_ICE40_PORT, GPIO_AF5, all); // af 5
  gpio_set_output_options(SPI_ICE40_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, out);

  // we should be able to simplify this. to non configured or input.
  // http://libopencm3.org/docs/latest/gd32f1x0/html/group__gpio__mode.html

  /*

  ''During and just after reset, the alternate functions are not active and the I/O ports are configured in Input Floating mode (CNFx[1:0]=01b, MODEx[1:0]=00b).''
  */
  // set cs2 hi - with external pullup.
  gpio_mode_setup(SPI_ICE40_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SPI_ICE40_CS2);
}


void spi1_port_setup2(void)
{
  // rcc_periph_clock_enable(RCC_SPI1);

  // setup spi with cs ...
  uint16_t out = SPI_ICE40_CLK | SPI_ICE40_CS2 | SPI_ICE40_MOSI ; // not MISO
  uint16_t all = out | SPI_ICE40_MISO;

  gpio_mode_setup(SPI_ICE40_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, all);
  gpio_set_af(SPI_ICE40_PORT, GPIO_AF5, all); // af 5
  gpio_set_output_options(SPI_ICE40_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, out); // probably need to reset each time.

  // set cs1 hi - with external pullup.
  gpio_mode_setup(SPI_ICE40_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SPI_ICE40_CS);
}


#if 0
void spi1_special_gpio_setup(void)
{
  // TODO change name this is not spi....
  // special
  gpio_mode_setup(SPI_ICE40_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI_ICE40_SPECIAL);
  gpio_set_output_options(SPI_ICE40_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI_ICE40_SPECIAL);

  gpio_set(SPI_ICE40_PORT, SPI_ICE40_SPECIAL ); // hi == off, active low...

}



void spi_special_flag_set(uint32_t spi)
{
  UNUSED(spi);
  gpio_set(SPI_ICE40_PORT, SPI_ICE40_SPECIAL );
}

void spi_special_flag_clear(uint32_t spi)
{
  // TODO change name this is not spi....
  UNUSED(spi);
  gpio_clear(SPI_ICE40_PORT, SPI_ICE40_SPECIAL ); // assert special, active low...
}

#endif




//////////////////

// #include "util.h"


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
ads131a04  DYDR Data ready; active low; host interrupt and synchronization for multi-devices
*/

void spi1_interupt_gpio_setup(void (*pfunc)(void *),  void *ctx)
{
  // TODO check non-null init args ...

  spi1_interupt = pfunc;
  spi1_ctx = ctx;

  gpio_mode_setup(SPI_ICE40_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SPI_ICE40_INTERUPT);

  // gpio_set_output_options(SPI_ICE40_PORT, GPIO_ITYPE, GPIO_ISPEED_50MHZ, SPI_ICE40_SPECIAL);   is there a way to set the speed?
                                                                                                  // looks like GPIO_ITYPE is recognized.

  // ie. use exti2 for pa2
  nvic_enable_irq(NVIC_EXTI2_IRQ);
  // nvic_set_priority(NVIC_EXTI2_IRQ, 5 );

  exti_select_source(EXTI2, SPI_ICE40_PORT);
  exti_set_trigger(EXTI2 , EXTI_TRIGGER_FALLING);
  exti_enable_request(EXTI2);
}


// printf("interupt CONFIGURE \n");
// usart1_flush();
// printf("configure done\n");
// usart1_flush();

