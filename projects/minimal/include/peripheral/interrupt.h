
#pragma once

/*

  peripheral interface/abstraction
  basic interrupt should be abstract, and un-associated with any device.

  just a simple structure - that adds a user-context, to the callback function

  systick interrupt should probably use this structure also
*/

typedef struct interrupt_t  interrupt_t;



typedef void (*interrupt_handler_t)(void *ctx, void *arg);


struct interrupt_t
{
  uint32_t magic;

  // mar 2026. simplify. these fields used to be opaque. using _new() and composition
  interrupt_handler_t  handler;
  void *ctx;

  void (*setup)( interrupt_t *);
  // void (*handler_set)( interrupt_t *, void *ctx, interrupt_handler_t);
};



static inline void interrupt_setup( interrupt_t *i)
{
  assert(i);
  i->setup( i);
}


static inline  void interrupt_handler_set( interrupt_t *i, void *ctx, interrupt_handler_t h)
{
  // simple accessor
  assert(i);
  //  handler_set( i, ctx, h );  no point to delegate, when fields are not opaque
  i->ctx = ctx;
  i->handler = h;
}

