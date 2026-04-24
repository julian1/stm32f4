
#pragma once

/*

  peripheral interface is abstract, and not associated with any device instance.

*/

typedef struct interrupt_t  interrupt_t;



typedef void (*interrupt_handler_t)( void *ctx, void *arg);


struct interrupt_t
{
  uint32_t magic;

  // mar 2026. simplify. these fields used to be opaque. using _new() and composition
  interrupt_handler_t  handler;
  void *ctx;

  void (*port_configure)( interrupt_t *);
  // void (*handler_set)( interrupt_t *, void *ctx, interrupt_handler_t);
};



/*
  consider rename port_configure() to just configure()
  for interrupt.

*/

static inline void interrupt_port_configure( interrupt_t *i)
{
  assert(i);
  i->port_configure( i);
}


static inline  void interrupt_handler_set( interrupt_t *i, void *ctx, interrupt_handler_t h)
{
  // simple accessor
  assert(i);
  //  handler_set( i, ctx, h );  no point to delegate, when fields are not opaque
  i->ctx      = ctx;
  i->handler  = h;
}

