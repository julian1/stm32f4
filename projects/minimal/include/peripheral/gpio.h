
#pragma once


/*
  super light abstraction for gpio to organize code
  systick should use
*/


// perhaps could be a gpio...

typedef struct gpio_t  gpio_t;

struct gpio_t
{
  // magic, type, size.

  uint32_t  magic;

  void (*setup)(gpio_t *);

  // can handle masking, shifting etc.
  void (*write)(gpio_t *, uint8_t val);
  // bool (*read)( gpio_t *);
};


static inline void gpio_setup( gpio_t *gpio)
{
  // assert(gpio);
  gpio->setup( gpio);
}


static inline void gpio_write( gpio_t *gpio, uint8_t val)
{
  // assert(gpio);
  gpio->write( gpio, val);
}



/*
  most gpio is simple and can be handled generically
  ie. pass port,pin in constructor in main.c or app.c
  and optionally speed
*/

struct gpio2_t
{
  // anonymous
  gpio_t    ;

  // passed on construction
  uint16_t    port;
  uint16_t    pin;
};




