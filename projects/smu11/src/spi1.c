

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>


#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>



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
  UNUSED(spi);
  gpio_clear(SPI_ICE40_PORT, SPI_ICE40_SPECIAL ); // assert special, active low...
}






//////////////////

#include "util.h"

void exti2_isr(void)
{


  usart_printf("x");
  usart_flush();


  exti_reset_request(SPI_ICE40_INTERUPT);
  exti_reset_request(EXTI2);


  // it's just fucking hanging????

/*
  if (exti_direction == FALLING) {
    exti_direction = RISING;
    exti_set_trigger(EXTI2, EXTI_TRIGGER_RISING);

  } else {
    exti_direction = FALLING;
    exti_set_trigger(EXTI2, EXTI_TRIGGER_FALLING);
  }
*/
}



void spi1_interupt_gpio_setup(void)
{
  // this seems to cause 


  //usart_printf("######################\n");
  usart_printf("interupt CONFIGURE \n");
  usart_flush();

  // need interupt clk also
  gpio_mode_setup(SPI_ICE40_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SPI_ICE40_INTERUPT);


  // use GPIO D0 so need EXTI2
  nvic_enable_irq(NVIC_EXTI2_IRQ);
  // nvic_set_priority(NVIC_EXTI2_IRQ, 5 );
 
  exti_select_source(EXTI2, SPI_ICE40_PORT);
  exti_set_trigger(EXTI2 , EXTI_TRIGGER_RISING);
  exti_enable_request(EXTI2);

  usart_printf("configure done\n");
  usart_flush();
}


#if 0
  exti_select_source(SPI_ICE40_INTERUPT, SPI_ICE40_PORT);
  exti_set_trigger(SPI_ICE40_INTERUPT, EXTI_TRIGGER_RISING);
  //exti_enable_request(SPI_ICE40_INTERUPT);
  exti_enable_request(EXTI2);
#endif


#if 0
  /* Configure the EXTI subsystem. */
  exti_select_source(EXTI2, SPI_ICE40_PORT);
  // exti_set_trigger(EXTI2, EXTI_TRIGGER_BOTH  /*EXTI_TRIGGER_FALLING */ );
  exti_set_trigger(EXTI2, EXTI_TRIGGER_RISING);
  exti_enable_request(EXTI2);
#endif

#if 0
  exti_select_source(EXTI0, GPIOD);
  exti_set_trigger(EXTI0, EXTI_TRIGGER_BOTH  /*EXTI_TRIGGER_FALLING */ );
#endif

