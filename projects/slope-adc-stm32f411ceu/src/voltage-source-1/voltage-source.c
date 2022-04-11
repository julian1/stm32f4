
/*
  what kind of interface do we want for this?
  +1,0,-1.
*/



#include <stdio.h> // printf
#include <libopencm3/stm32/gpio.h>

#include "voltage-source-1/voltage-source.h"
#include "assert.h"

#define VS_PORT   GPIOB
#define VS_UP     GPIO9     // PA9,  net  SPI2_CS2
#define VS_DOWN   GPIO10    // PA10       SPI2_GPIO2


void voltage_source_setup(void)
{
  uint16_t all = VS_UP | VS_DOWN;

  gpio_mode_setup( VS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, all);
  gpio_set_output_options( VS_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, all);    // optocouplers are slow

  // turn off at init
  voltage_source_set_dir( 0);
}



void voltage_source_set_dir( int val )
{
  switch(val) {

    case 1:
      // printf("voltage_source_set doing up!\n" );
      gpio_set(  VS_PORT, VS_UP);
      gpio_clear(VS_PORT, VS_DOWN);
      break;

    case -1:
      // printf("voltage_source_set doing down!\n" );
      gpio_clear(VS_PORT, VS_UP);
      gpio_set(  VS_PORT, VS_DOWN);
      break;

    case 0:
      // printf("voltage_source_set clear!\n" );
      gpio_clear(VS_PORT, VS_UP);
      gpio_clear(VS_PORT, VS_DOWN);
      break;

    default:
      assert(0);
  }
}

