

#pragma once

static inline void gpio_write_val(uint32_t gpioport, uint16_t gpios, bool val)
{
  /*
  // set/clear gpios bits, according to bool val
  // use CM3 style args
  // eg. gpio_write_val( GPIOA, GPIO9, 1);
  // eg.

  assert( GPIO9 == 1<< 9 );
  assert( GPIOA ==  (PERIPH_BASE_AHB1 + 0x0000) );

  */

  // where should this go?
  // just inline in include/gpio perhaps.

  // BSRR == bit set/reset register.

  // GPIO_BSRR( gpioport) |= gpios  << (val ? 16: 0);
  GPIO_BSRR( gpioport) |= gpios  << (val ? 0: 16);
}


