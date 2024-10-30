

#pragma once



/*
  very light abstraction.
  only really useful with multiple leds

  can pass this to critical_error_blink() function easily.

  this cannot really be generic.
  ----------

  passing an array/struct of funcs for behavior. better than passing specific port/pin info
*/


// perhaps could be a gpio...

typedef struct led_t  led_t ;

struct led_t
{
  void (*setup)(led_t *);
  void (*set)(led_t *, bool val);
};

static inline void led_set( led_t *led, bool val)
{
  led->set(led, val);
}



