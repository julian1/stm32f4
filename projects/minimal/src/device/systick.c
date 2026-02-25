

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/gpio.h>

#include <stddef.h>  // NULL
// #include <assert.h>

#include "device/systick.h"



////////////////////////////////////////////////////////



static void (*systick_interupt)(void *ctx) = NULL;
static void *systick_ctx = NULL;


void systick_handler_set( void (*pfunc)(void *),  void *ctx)
{
  systick_interupt = pfunc;
  systick_ctx = ctx;
}



void sys_tick_handler(void) // called by runtime
{
  // prototype declared in cm3/systick.
  if(systick_interupt) {
    systick_interupt(systick_ctx);
  }
}



void systick_setup(uint32_t tick_divider)
{

  /* clock rate / 168000 to get 1mS interrupt rate */
  // systick_set_reload(168000);
  systick_set_reload(tick_divider);
  systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
  systick_counter_enable();

  /* this done last */
  systick_interrupt_enable();
}



