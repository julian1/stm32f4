


#include <libopencm3/stm32/gpio.h>



/*
void gpio_set(uint32_t gpioport, uint16_t gpios)
{
  GPIO_BSRR(gpioport) = gpios;
}


void  gpio_clear(uint32_t gpioport, uint16_t gpios)
{
  GPIO_BSRR(gpioport) = (gpios << 16);
}

*/

void gpio_write_with_mask(uint32_t gpioport, uint16_t mask, uint16_t vals )
{
  // gpio_set(gpioport, mask & gpios);
  // gpio_clear(gpioport, mask & (~ gpios) );
  

  GPIO_BSRR(gpioport) = (mask & vals) | ((mask & ~vals) << 16);

}


