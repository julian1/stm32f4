

#pragma once

// #include <libopencm3/stm32/gpio.h>    // led


/*
  implementation detail escape, but has to be shared - to support passing  to critical_error_blink() function.
  hang on.

  this cannot really be generic.
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


/*
  - actually really has to be more of a blink led controller.
  - controller

*/

#if 0

static inline void led_set( led_t *l, uint8_t val)
{
  // convenience.
  l->set( l, val);
};



static void led_create( led_t *l)
{
  // different leds. will have 
  // fill in the functions etc.
  // issue is where/ and how to advertize the size of the  structure, to allocate it.
  // can inclde led.c in main.c
  //  

  l->port_setup = NULL;
  l->set = NULL;
  

}

#endif


