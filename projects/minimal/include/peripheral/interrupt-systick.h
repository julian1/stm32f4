
// we need this to carry the tick-divider state.

#include <peripheral/interrupt.h>

struct interrupt_systick_t
{

  interrupt_t  ;   // C11. anonymous.  for composition.

  uint32_t tick_divider;
};



