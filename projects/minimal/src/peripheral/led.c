/*
  not sure how useful this is, perhaps could just add to main.c

  same initerface as spi_enable()/spi_disable(). etc
  and ice40_creset_enable()/disable()
*/

#include <libopencm3/stm32/gpio.h>


#include <peripheral/led.h>


void led_on(void)
{
    gpio_set( LED_PORT, LED_OUT);
}


void led_off(void)
{
    gpio_clear( LED_PORT, LED_OUT);
}


void led_setup(void)
{
  // rcc_periph_clock_enable(RCC_GPIOA);

  gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_OUT);


  gpio_set_output_options(LED_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, LED_OUT);
}


