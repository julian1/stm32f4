/*
  not sure how useful this is, perhaps could just add to main.c

  same initerface as spi_enable()/spi_disable(). etc
  and ice40_creset_enable()/disable()
*/



#include <assert.h>  // assert_simple()
#include <stdio.h>  // printf



#include <libopencm3/stm32/gpio.h>


#include <peripheral/hal.h>

#include <peripheral/led.h>

#if 1

void led_on(uint16_t led)
{

  gpio_set( CM3_PINBANK(led), CM3_PINNO(led) );

}


void led_off(uint16_t led)
{

  gpio_clear( CM3_PINBANK(led), CM3_PINNO(led) );
}


void led_setup(uint16_t led)
{
  // rcc_periph_clock_enable(RCC_GPIOA);

  gpio_mode_setup( CM3_PINBANK(led), GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, CM3_PINNO(led));

  gpio_set_output_options( CM3_PINBANK(led), GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, CM3_PINNO(led));
}

#endif




#if 0
void led_on(uint16_t led)
{
  gpio_set( LED_PORT, LED_OUT);
}


void led_off(uint16_t led)
{
  gpio_clear( LED_PORT, LED_OUT);
}



void led_setup(uint16_t led)
{
  // rcc_periph_clock_enable(RCC_GPIOA);

  gpio_mode_setup( LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_OUT );

  gpio_set_output_options( LED_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, LED_OUT );
}


#endif

