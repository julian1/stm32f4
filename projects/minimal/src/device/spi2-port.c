

#include <stdio.h>

#include <libopencm3/stm32/gpio.h>

#include <device/spi2-port.h>



void spi2_port_setup(void)
{
  printf("spi2 port setup\n");

  // rcc_periph_clock_enable(RCC_SPI2);


  // spi2.  oct 2024.
  // clk. PB10.
  // miso PB14
  // mosi PB15

  gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10  | GPIO14 | GPIO15  );    // clk/miso/mosi  PB10,PB14,PB15
  gpio_set_af(GPIOB, GPIO_AF5, GPIO10 | GPIO14 | GPIO15);       // clk/miso/mosi
  gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO10 | GPIO15);   // clk, mosi   PB10,PB15


  // dont setup port pin as input when using AF
  //  gpio_mode_setup(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO14)     ;   // miso PB14.   default


  printf("spi2 port setup done\n");

}


