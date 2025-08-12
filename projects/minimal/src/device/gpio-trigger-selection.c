


#include <assert.h>
#include <string.h>  // memset
#include <stdlib.h>  // malloc


#include <support.h>     // gpio_write_val()

#include <libopencm3/stm32/gpio.h>


#include <device/gpio-trigger-selection.h>





// PE2
#define PORT  GPIOE
#define PIN   GPIO2



static void setup(gpio_t *p)
{
  assert(p);
  gpio_mode_setup( PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PIN);
  gpio_set_output_options( PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, PIN);
}


static void write( gpio_t *p, uint8_t val)
{
  assert(p);
  gpio_write_val( PORT, PIN, val);
}



gpio_t *gpio_trigger_selection_create()
{
  gpio_t *p = malloc(sizeof( gpio_t));
  assert(p);
  memset(p, 0, sizeof( gpio_t));

  p->setup = setup;
  p->write = write;
  return p;
}




