/*
  demonstrates interface
  only really useful with multiple leds
*/



// #include <stdio.h>  // printf don't use printf not configured.
#include <assert.h>  // assert_simple()
#include <string.h>  // memset
#include <stdlib.h>  // malloc



#include <libopencm3/stm32/gpio.h>


#include <peripheral/led.h>


#include <gpio.h>     // gpio_write_val



// PA9
#define LED_PORT  GPIOA
#define LED_PIN   GPIO9


static void set( led_t *led, bool val)
{
  assert(led);

  gpio_write_val(LED_PORT, LED_PIN, val );
}



static void setup(led_t *led)
{
  assert(led);

  gpio_mode_setup( LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN);
  gpio_set_output_options( LED_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, LED_PIN);
}



led_t *led_create()
{
  led_t *p= malloc(sizeof(  led_t ));
  assert(p);

  p->set = set;
  p->setup = setup;

  return p;
}






#if 0



void led_on(uint16_t led)
{
  UNUSED(led);
  // gpio_set( LED_PORT, LED_OUT);      // cm3
  // hal_gpio_write( led, 1);           // hal
  // hal_gpio_write( PIN('A', 9), 1);   // hal
  // GPIO_BSRR( GPIOA ) |= (1<<9);      // cm3




  gpio_write_val(GPIOA, GPIO9, 1 );
}



void led_off(uint16_t led)
{
  UNUSED(led);
  // gpio_clear( LED_PORT, LED_OUT);      // cm3
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
  gpio_mode_setup( LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_OUT );
  gpio_set_output_options( LED_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, LED_OUT );
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

