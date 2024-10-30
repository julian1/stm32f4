


// with all the enable/disable.
// probably better. to have  a  cs_set( val )   // lo,hi.


#include <stdio.h>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>       // TODO REMOVE.


#include <device/spi-port.h>


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


// for some reason this fucks up ability to read spi...
//  gpio_mode_setup(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO14)     ;   // miso PB14.   default


  printf("spi2 port setup done\n");

}




  // as well as more than one interupt. may get more

#if 0

  // creset pc6
  // CS_u509 / pc7
  gpio_mode_setup( GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,  GPIO7);      // PC0,PC6, PC7
  gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,  | GPIO7 );


  // CS_u704 / pe1
  gpio_mode_setup( GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,  GPIO1);              // PE1
  gpio_set_output_options(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,  GPIO1 );

  // CS_spt2046 / pb9
  gpio_mode_setup( GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,  GPIO9);              // PB9
  gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,  GPIO9 );


  // spi2_u202_int  pc2
  // spi2_u509_int   pc8.
  // spi2_xpt2046   pb4.

  // need reset/cdone.    perhaps spi_port_creset_u202().
  // bool spi_port_cdone_u202_get(void)


  // input

#endif


