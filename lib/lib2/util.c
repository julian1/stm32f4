/*

  helper functions
  perhaps rename to systick.c or just sleep...
  ---
  TODO move void print_stack_pointer()
  and rename to systtick . or soft-timer. etc.
*/


#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/gpio.h>


#include <stdio.h>  // printf

#include "util.h"



////////////////////////////////////////////////////////

/*
  just use a uint64_t for overflow wrap-around?
*/



static void (*sys_tick_interupt)(void *ctx) = NULL;
static void *sys_tick_ctx = NULL;


void systick_setup(uint32_t tick_divider, void (*pfunc)(void *),  void *ctx)
{

  sys_tick_interupt = pfunc;
  sys_tick_ctx = ctx;

    ///////////////////////////
  // NOTE. doesn not seem to work without external xtal.
  // TODO change name systick_setup().
  // TODO pass clock reload divider as argument, to localize.

  /* clock rate / 168000 to get 1mS interrupt rate */
  // systick_set_reload(168000);
  systick_set_reload(tick_divider);
  systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
  systick_counter_enable();

  /* this done last */
  systick_interrupt_enable();
}



void sys_tick_handler(void)
{

  // rather than the callback function.
  // could pass the volatile variable to increment.
  // more lightweight

  if(sys_tick_interupt) {
    sys_tick_interupt(sys_tick_ctx);
  }

}



/*
  https://arduino.stackexchange.com/questions/12587/how-can-i-handle-the-millis-rollover
  https://hsel.co.uk/2014/06/20/stm32f0-tutorial-2-systick/
*/



// void msleep(uint32_t delay)
void msleep(uint32_t delay, volatile uint32_t *system_millis )
{
  // works for system_millis integer wrap around
  // could be a do/while block.
  uint32_t start = *system_millis ;
  while (true) {
    uint32_t elapsed = *system_millis - start;
    if(elapsed > delay)
      break;
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

