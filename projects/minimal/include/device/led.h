

#pragma once



/*
  very light abstraction.
  only really useful with multiple leds

  can pass this to critical_error_blink() function easily.

  this cannot really be generic.
  ----------

  passing an array/struct of funcs for behavior. better than passing specific port/pin info
*/



typedef struct led_t  led_t ;

struct led_t
{
  void (*setup)(led_t *);
  void (*set)(led_t *, bool val);
};


// constructors, non opaque, should be called in main()
led_t *led_create(void);
led_t *led_create2(uint32_t port, uint32_t gpios);   // another way...

static inline void led_set( led_t *led, bool val)
{
  led->set(led, val);
}



