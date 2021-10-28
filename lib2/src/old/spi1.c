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

#define SPI_ICE40_INTERUPT GPIO2   // PA2

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


// usart_printf("interupt CONFIGURE \n");
// usart_flush();
// usart_printf("configure done\n");
// usart_flush();

