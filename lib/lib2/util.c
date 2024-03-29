/*

  some helper functions

  TODO maybe move void print_stack_pointer()
  and rename to systick, or soft-timer. etc.
  or rename to control.c  because of sleeping yield/continuation like functions?
*/


#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/gpio.h>


#include <assert.h>
#include <stdio.h>  // printf

#include "util.h"



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







/*
  https://arduino.stackexchange.com/questions/12587/how-can-i-handle-the-millis-rollover
  https://hsel.co.uk/2014/06/20/stm32f0-tutorial-2-systick/
*/

#if 0
void msleep(uint32_t delay, volatile uint32_t *system_millis /* void (*yield)(void *) */ )
{
  assert(system_millis);

  // works for system_millis integer wrap around
  // could be a do/while block.
  uint32_t start = *system_millis;
  while (true) {
    uint32_t elapsed = *system_millis - start;
    if(elapsed > delay)
      break;
  };
}
#endif

void msleep(uint32_t delay, volatile uint32_t *system_millis)
{

  yield_with_msleep(delay, system_millis,  NULL, NULL);

}



/*
  msleep() is blocking.
  but can pass a yield() function as a general update handler.
  that is called until the elapsed time has passed.
  this can service soft-timers, comms/usart network queues, polling etc.
  and using the same stack.  avoids complication of co-routines, context switch, and need to manage multiple stacks.
  --
  can also pass null to ignore.

  rename  msleep_with_continuation() ? ,
  no because a continuation, is the next function to be sequenced, not an intermediate yielding  function.
*/

void yield_with_msleep(uint32_t delay, volatile uint32_t *system_millis,  void (*yield)(void *), void *yield_ctx  )
{
  assert(system_millis);

  // works for system_millis integer wrap around
  // could be a do/while block.
  uint32_t start = *system_millis;
  while (true) {
    uint32_t elapsed = *system_millis - start;
    if(elapsed > delay)
      break;

    if(yield)
      yield( yield_ctx);
  };
}






/*
// test code for integer wrap around

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

volatile uint32_t  system_millis = 0;

static void msleep(uint32_t delay)
{
  uint32_t start = system_millis ;
  printf("start %u\n" , start );
  while (true) {
    uint32_t elapsed = system_millis - start;
    printf("system_millis %u,   elapsed %u\n" ,   system_millis,  elapsed );
    if(elapsed > delay)
      break;
    ++system_millis;
  };
}

int main()
{
  system_millis = UINT32_MAX  - 10;
  msleep(5);
  printf("\n");
  system_millis = UINT32_MAX  - 10;
  msleep(15);
}
*/



/*
  2023. substraction looks wrong.  stack start - is stack_end - current .
  because stack grows down.



The stack does not grow from the RAM-Start (0x20000000) to the end.
  The Stack starts at RAM-End (0x20001800) and grows downwards. You define _estack in the Linker script.
  This value is used in the startup code and put into the vector table (Assembler file startup_stm32f …s).
  The processor initializes his internal stack pointer register with this.
  Global and or static variables which are available from the begin of the program are placed at the RAM-Start (0x20000000) and after that comes the heap.
  You can see this in the linker Script. “.data”, “.bss”, “.heap…” go into the RAM in this order.
  Starting at “RAM” which has the RAM-start address (0x20000000).



  // for f411ceu.
	// ram (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
  p  -  (ORIGIN + (LENGTH * 1024) )

  we should be able to get the stack start, from the vector table.
*/

#if 0
static void f()
{
  SCB->VTOR;
};
#endif



void print_stack_pointer()
{
  // https://stackoverflow.com/questions/20059673/print-out-value-of-stack-pointer
  // non-portable.
  void* p = NULL;
  // usart1_printf("sp %p   %d\n", (void*)&p,  ( (unsigned)(void*)&p)  - 0x20000000   );


  printf("sp %p   %d\n", (void*)&p,  ( (unsigned)(void*)&p)  - 0x20000000   );


  // return &p;
  // uint32_t x = _stack;

}




/*
  https://developer.arm.com/documentation/dui0475/j/the-arm-c-and-c---libraries/defining---initial-sp----heap-base-and---heap-limit?lang=en

  used to define, not retrieve,

  // extern void * __initial_sp ;
  // usart1_printf("p %p \n",  __initial_sp);


__attribute__((naked)) void dummy_function(void)
{
   __asm(".global __initial_sp\n\t"
         ".global __heap_base\n\t"
//         ".global __heap_limit\n\t"
         ".equ __initial_sp, STACK_BASE\n\t"
         ".equ __heap_base, HEAP_BASE\n\t"
 //        ".equ __heap_limit, (HEAP_BASE+HEAP_SIZE)\n\t"
   );
}

*/

