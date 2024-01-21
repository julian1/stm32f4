/*

  anything through a 6 pin adum/cap isolator

  Code is shared for smu and adc
  move to shared library?

  -----------

  EXTR. dec 18 2022.
    at 100MHz (instead of 50MHz) there are flip issues, under stress test. with iso7762

  could use an and gate ic with gpio. but dificulty of synchronizing signals

*/

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>


#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>


#include <stddef.h>   // NULL
#include <assert.h>

#include "spi-port.h"

#include <lib2/util.h>   // critical_error_blink()

//////////////////////////

#define UNUSED(x) (void)(x)


// TODO - no reason to have the ICE40  prefix here.
// should just name to SPI1c

/*
  jan 2024.
  - if put all these in a data structure - then the enable/disable could be made more generic. for spi1,or spi2 etc.
  - we would'nt have to carry around the spi integer. in app.
  - not sure the callback works. 
  ----
  - rather than having to prefix with spi1... everywhere.

*/


#define SPI_SPI   SPI1

#define SPI_PORT  GPIOA
#define SPI_CLK   GPIO5     // PA5
#define SPI_CS1   GPIO4     // PA4
#define SPI_MOSI  GPIO7     // PA7
#define SPI_MISO  GPIO6     // PA6
#define SPI_CS2   GPIO15    // PA15 nss 2. moved.


#define SPI_INTERUPT GPIO2   // PA2


/*

  whether active high/lo depends on the peripheral device. and fpga may invert
  so use  _enable(), disable() . instead of clear
  following normal CS convention. active low
*/


void spi1_port_cs2_enable(void)
{
  gpio_clear(SPI_PORT, SPI_CS2);
}


void spi1_port_cs2_disable(void)
{
  gpio_set(SPI_PORT, SPI_CS2);
}


void spi1_port_cs1_enable(void)
{
  gpio_clear(SPI_PORT, SPI_CS1);
}


void spi1_port_cs1_disable(void)
{
  gpio_set(SPI_PORT, SPI_CS1);
}






void spi1_port_cs1_setup(void)
{
  // rcc_periph_clock_enable(RCC_SPI1);

  // setup spi with cs ...
  uint16_t out = SPI_CLK | SPI_CS1 | SPI_MOSI ; // not MISO
  uint16_t all = out | SPI_MISO;

  gpio_mode_setup(SPI_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, all);
  gpio_set_af(SPI_PORT, GPIO_AF5, all); // af 5
  gpio_set_output_options(SPI_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, out);

  // we should be able to simplify this. to non configured or input.
  // http://libopencm3.org/docs/latest/gd32f1x0/html/group__gpio__mode.html

  /*

  ''During and just after reset, the alternate functions are not active and the I/O ports are configured in Input Floating mode (CNFx[1:0]=01b, MODEx[1:0]=00b).''
  */
  // set cs2 hi - with external pullup.
  gpio_mode_setup(SPI_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SPI_CS2);
}


void spi1_port_cs2_setup(void)
{
  // rcc_periph_clock_enable(RCC_SPI1);

  // setup spi with cs ...
  uint16_t out = SPI_CLK | SPI_CS2 | SPI_MOSI ; // not MISO
  uint16_t all = out | SPI_MISO;

  gpio_mode_setup(SPI_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, all);
  gpio_set_af(SPI_PORT, GPIO_AF5, all); // af 5
  gpio_set_output_options(SPI_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, out); // probably need to reset each time.

  // set cs1 hi - with external pullup.
  gpio_mode_setup(SPI_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SPI_CS1);
}


void spi1_port_cs2_gpio_setup(void)
{
  // rcc_periph_clock_enable(RCC_SPI1);

  // setup spi with cs ...
  // uint16_t out = SPI1_CLK /* | SPI1_CS2 */  | SPI1_MOSI ; // not MISO
  // uint16_t all = out | SPI1_MISO ;
  uint16_t out = SPI_CLK |  SPI_MOSI ; // not MISO
  uint16_t all = out | SPI_MISO;


  gpio_mode_setup(SPI_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, all);
  gpio_set_af(SPI_PORT, GPIO_AF5, all); // af 5
  gpio_set_output_options(SPI_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, out); // probably need to reset each time.

  // set cs1 hi-z/hi - with external pullup.
  gpio_mode_setup(SPI_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SPI_CS1);

  // set CS2 to manual external gpio output
  gpio_mode_setup(SPI_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI_CS2);
  gpio_set_output_options(SPI_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI_CS2);
}


void spi1_port_cs1_cs2_gpio_setup(void)
{
  // control both cs1 and cs2 with gpio. for creset assert.

  gpio_mode_setup(SPI_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI_CS1 | SPI_CS2);
  gpio_set_output_options(SPI_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI_CS1 | SPI_CS2);

}








//////////////////


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

  TODO rename spi1_port_ ...
*/

void spi1_port_interupt_gpio_setup(void (*pfunc)(void *),  void *ctx)
{
  // TODO check non-null init args ...

  spi1_interupt = pfunc;
  spi1_ctx = ctx;

  gpio_mode_setup(SPI_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SPI_INTERUPT);

  // gpio_set_output_options(SPI_PORT, GPIO_ITYPE, GPIO_ISPEED_50MHZ, SPI_SPECIAL);   is there a way to set the speed?
                                                                                                  // looks like GPIO_ITYPE is recognized.

  // ie. use exti2 for pa2
  nvic_enable_irq(NVIC_EXTI2_IRQ);
  // nvic_set_priority(NVIC_EXTI2_IRQ, 5 );

  exti_select_source(EXTI2, SPI_PORT);
  // exti_set_trigger(EXTI2 , EXTI_TRIGGER_FALLING);
  exti_set_trigger(EXTI2 , EXTI_TRIGGER_RISING );         // JA. nov 1. 2023. to make consistent with _valid signal hi.
  exti_enable_request(EXTI2);
}


// printf("interupt CONFIGURE \n");
// usart1_flush();
// printf("configure done\n");
// usart1_flush();




#if 0
void spi_cs2_strobe_assert( uint32_t spi)
{
  /*
    might be better as separate functions spi_cs1_assert() , spi_cs2_assert()  or cs1_enable(), disable() then an
    can then do reset, or strobe, or strobe with different timing etc.
    remember relies on port with CS2 configured as gpio.
  */

  switch(spi) {
    case SPI1: {

      gpio_set(SPI_PORT, SPI_CS2);

      for(uint32_t i = 0; i < 50; ++i)
         __asm__("nop");

      gpio_clear(SPI_PORT, SPI_CS2);
      break;
    }

    default:
      assert(0);
      critical_error_blink();
  }
}
#endif




#if 0
void spi1_special_gpio_setup(void)
{
  // TODO change name this is not spi....
  // special
  gpio_mode_setup(SPI_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI_SPECIAL);
  gpio_set_output_options(SPI_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI_SPECIAL);

  gpio_set(SPI_PORT, SPI_SPECIAL ); // hi == off, active low...

}



void spi_special_flag_set(uint32_t spi)
{
  UNUSED(spi);
  gpio_set(SPI_PORT, SPI_SPECIAL );
}

void spi_special_flag_clear(uint32_t spi)
{
  // TODO change name this is not spi....
  UNUSED(spi);
  gpio_clear(SPI_PORT, SPI_SPECIAL ); // assert special, active low...
}

#endif

