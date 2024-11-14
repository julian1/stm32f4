
#pragma once

// peripheral interface/abstraction
// basic interrupt should be abstract, and un-associated with any device.

typedef struct interrupt_t  interrupt_t;



typedef   void (*interupt_handler_t)(void *ctx, interrupt_t *)   ;


struct interrupt_t
{

  void (*setup)( interrupt_t *);

  /* user settable - better to use a setter func to hide state
    but ok for now */

  interupt_handler_t  handler;
  void *ctx;
};




