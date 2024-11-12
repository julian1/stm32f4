

#pragma once

static inline void gpio_write_val(uint32_t gpioport, uint16_t gpios, bool val)
{
  // better place for this?
  // inline in include/gpio perhaps.

  /*
  // set/clear gpios bits, according to bool val
  // use CM3 style args
  // eg. gpio_write_val( GPIOA, GPIO9, 1);
  // eg.

  assert( GPIO9 == 1<< 9 );
  assert( GPIOA ==  (PERIPH_BASE_AHB1 + 0x0000) );

  */


  // BSRR == bit set/reset register.

  GPIO_BSRR( gpioport) |= gpios  << (val ? 0: 16);
}


