/*
  not sure how useful this is, perhaps could just add to main.c

  same initerface as spi_enable()/spi_disable(). etc
  and ice40_creset_enable()/disable()
*/



#include <assert.h>  // assert_simple()
#include <stdio.h>  // printf



#include <libopencm3/stm32/gpio.h>


#include <peripheral/led.h>

#include <hal/hal.h>


#define UNUSED(a)   ((void)(a))

/*
#define LED_PORT  GPIOA
#define LED_OUT   GPIO9
*/




static void gpio_write_val(uint32_t gpioport, uint16_t gpios, bool val)
{
  /*
  // set/clear gpios bits, according to bool val
  // call with CM3 arguments
  // eg. gpio_write_val( GPIOA, GPIO9, 1);
  // eg.

  assert( GPIO9 == 1<< 9 );
  assert( GPIOA ==  (PERIPH_BASE_AHB1 + 0x0000) );

  */

  // where should this go?


  GPIO_BSRR( gpioport) |= gpios  << (val ? 16: 0);
}





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




