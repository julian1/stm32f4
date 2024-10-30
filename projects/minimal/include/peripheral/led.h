

#pragma once

// #include <libopencm3/stm32/gpio.h>    // led


/*
  implementation detail escape, but has to be shared - to support passing  to critical_error_blink() function.
*/



#if 0

#define LED_PORT  GPIOA
#define LED_OUT   GPIO9

void led_on(void);
void led_off(void);
void led_setup(void);


#endif




void led_on(uint16_t led);
void led_off(uint16_t led);
void led_setup(uint16_t led);

