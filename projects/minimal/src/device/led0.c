


#include <assert.h>
#include <string.h>  // memset
#include <stdlib.h>  // malloc


#include <support.h>     // gpio_write_val()

#include <libopencm3/stm32/gpio.h>


#include <device/led0.h>





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



gpio_t *led0_create()
{
  gpio_t *p = malloc(sizeof( gpio_t));
  assert(p);
  memset(p, 0, sizeof( gpio_t));

  p->setup = setup;
  p->write = write;

  return p;
}






#if 0



void led_on(uint16_t led)
{
  UNUSED(led);
  // gpio_set( PORT, LED_OUT);      // cm3
  // hal_gpio_write( led, 1);           // hal
  // hal_gpio_write( PIN('A', 9), 1);   // hal
  // GPIO_BSRR( GPIOA ) |= (1<<9);      // cm3




  gpio_write_val(GPIOA, GPIO9, 1 );
}



void led_off(uint16_t led)
{
  UNUSED(led);
  // gpio_clear( PORT, LED_OUT);      // cm3
  // hal_gpio_write( led, 0);             // hal
  // hal_gpio_write( PIN('A', 9), 0);     // hal
  // GPIO_BSRR( GPIOA ) |= (1<<(9+16));   // cm3

  gpio_write_val(GPIOA, GPIO9, 0 );
}



void led_setup(uint16_t led)
{
  // UNUSED(led);
  // rcc_periph_clock_enable(RCC_GPIOA);

/*
  gpio_mode_setup( PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_OUT );
  gpio_set_output_options( PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, LED_OUT );
*/

  // works
  hal_gpio_set_mode( led,  GPIO_MODE_OUTPUT_ );


}





#if 0

static inline void hal_gpio_write(uint16_t pin, bool val) {
  GPIO_TypeDef *gpio = GPIO(PINBANK(pin));
  gpio->BSRR = (1U << PINNO(pin)) << (val ? 0 : 16);
}





http://libopencm3.org/docs/latest/stm32f4/html/gpio__common__all_8c_source.html

void gpio_set(uint32_t gpioport, uint16_t gpios)
{
        GPIO_BSRR(gpioport) = gpios;
}


void  gpio_clear(uint32_t gpioport, uint16_t gpios)
{
        GPIO_BSRR(gpioport) = (gpios << 16);
}

#endif


#endif

