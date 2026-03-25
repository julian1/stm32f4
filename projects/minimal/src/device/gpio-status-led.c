


#include <assert.h>
#include <string.h>  // memset

#include <libopencm3/stm32/gpio.h>


#include <device/gpio-status-led.h>
#include <device/support.h>     // gpio_write_val()





// PA9
#define PORT  GPIOA
#define PIN   GPIO9



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



void gpio_status_led_init( gpio_t *p)
{
  assert( p);
  memset( p, 0, sizeof( gpio_t));

  p->setup = setup;
  p->write = write;
}



