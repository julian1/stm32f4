
#pragma once


/*
  very light abstraction for gpio
*/


// perhaps could be a gpio...

typedef struct gpio_t  gpio_t;

struct gpio_t
{
  // magic, type, size.

  void (*setup)(gpio_t *);

  // can handle masking, shifting etc.
  void (*write)(gpio_t *, uint8_t val);


};


static inline void gpio_setup( gpio_t *io)
{
  // assert(io);
  io->setup( io);
}



static inline void gpio_write( gpio_t *io, uint8_t val)
{
  // assert(io);
  io->write(io, val);
}



