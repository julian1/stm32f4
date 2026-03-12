

#include <stddef.h>  // NULL

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>

#include <device/systick.h>






static void (*systick)( void *ctx) = NULL;
static void *ctx = NULL;


void sys_tick_handler( void) // called by runtime
{
  // prototype declared in cm3/systick.
  if( systick) {
    systick( ctx);
  }
}




void systick_handler_set( void (*systick_)(void *),  void *ctx_)
{
  systick = systick_;
  ctx     = ctx_;
}


void systick_setup( uint32_t tick_divider)
{

  /* clock rate / 168000 to get 1mS interrupt rate */
  // systick_set_reload(168000);
  systick_set_reload( tick_divider);
  systick_set_clocksource( STK_CSR_CLKSOURCE_AHB);
  systick_counter_enable();

  /* done last */
  systick_interrupt_enable();
}



