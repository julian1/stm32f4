

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>



#include <string.h> // memset
#include <assert.h>


#include <peripheral/interrupt-systick.h>
#include <device/interrupt-systick.h>


#define UNUSED(x) ((void)(x))


#define INT_SYSTICK_MAGIC   43112389



static interrupt_t   *x = NULL;



void sys_tick_handler( void) // called by runtime
{
  assert(x);
  assert(x->magic == INT_SYSTICK_MAGIC);

  if( x->handler) {

    // printf("x       is %p\n", x );
    // printf("x->ctx  is %p\n", x->ctx);
    // printf("f       is %p\n", x->handler);

    x->handler( x->ctx, NULL );
  }
}


static void port_configure( interrupt_t *i)
{
  assert(i && i->magic == INT_SYSTICK_MAGIC);

  // downcast
  interrupt_systick_t *i_  =  i_;


  /* clock rate / 168000 to get 1mS interrupt rate */
  // systick_set_reload(168000);
  systick_set_reload( i_->tick_divider);
  systick_set_clocksource( STK_CSR_CLKSOURCE_AHB);
  systick_counter_enable();

  /* done last */
  systick_interrupt_enable();
}


void interrupt_systick_init( interrupt_systick_t *i, /* nvic_ctx_table * */ uint32_t tick_divider)
{
  assert( i);
  memset( i, 0, sizeof( interrupt_systick_t));

  i->magic        = INT_SYSTICK_MAGIC;
  i->port_configure =  port_configure;

  i->tick_divider = tick_divider;

  x = i;
}


