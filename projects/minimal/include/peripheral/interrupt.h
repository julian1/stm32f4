
#pragma once

/* peripheral interface/abstraction
// basic interrupt should be abstract, and un-associated with any device.


// should try to put the systick interupt on this structure also
*/

typedef struct interrupt_t  interrupt_t;



typedef void (*interupt_handler_t)(void *ctx, void *arg);


struct interrupt_t
{
  /* user settable - better to use a setter func to hide state
    but ok for now */

  uint32_t magic;   // not sure if place here, 

  void (*setup)( interrupt_t *);
  void (*set_handler)( interrupt_t *, void *ctx, interupt_handler_t);
};



static inline void interrupt_setup( interrupt_t *i)
{
  assert(i);
  i->setup( i);
}



static inline  void interrupt_set_handler( interrupt_t *i, void *ctx, interupt_handler_t h)
{
  assert(i);
  i->set_handler( i, ctx, h );
}

