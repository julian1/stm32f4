/*

    oct 3.
    - EXTR. the systick handler should have a callback also.  to api-state.
        rather than code static variables.

        - eg. reaching out into space for system_millis  is horrid.
    -
    ------

// sep 16. 2023. THIS IS really crappy. with led, hanging onto local state like.
    - opposite of dep-inversion.

  -- think it's easier to just define the state in App (.  and do the update in main.
  - issue is critical error blink. called from assert. that has no context.

  Instead - just farm out the revevant state and hold a static reference for this specific function.
       that cannot be called in any other context.
  ----------------

  should be common now.
  ----
  helper stuff that belongs in separate file, but not in separate library
  because might change

  also. port setup. eg. led blinker. that is very common. but different between stm32 series/part.
*/

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>


#include <libopencm3/stm32/gpio.h>

#include "util.h"
#include "streams.h"
#include "assert.h"  // assert simple








//////////////////////////////////

/*
  setup external state for critical error led blink
  because assert() cannot pass a context

  only used - used only for critical_error_led_blink() and assert.
  other things , may also write the led.
  just use static, to hang on to the vars.  no need for struct.
*/


static uint32_t led_port = 0;
static uint32_t led_io = 0;


void critical_error_led_setup(uint32_t port_, uint16_t io_ )   // innit or setup
{
  led_port = port_;
  led_io   = io_;

}

void critical_error_led_blink(void)
{
  // needs the led port config.
  // avoid passing arguments, consuming stack.
  for (;;) {
		// gpio_toggle(led.port, led.io );
    // led_toggle();

    gpio_toggle(led_port, led_io);

		for(uint32_t i = 0; i < 500000; ++i)
       __asm__("nop");
  }
}




////////////////////////////////////////////////////////

/* Common function descriptions */
// #include "clock.h"

/* milliseconds since boot */
// volatile uint32_t system_millis = 0;
// volatile uint64_t system_millis;

/*
  just use a uint64_t ?
*/



static void (*sys_tick_interupt)(void *ctx) = NULL;
static void *sys_tick_ctx = NULL;


// static void systick_setup(void)
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



void assert_simple(const char *file, int line, const char *func, const char *expr)
{
  // this works by using local "assert.h" with assert() macro definition
  // see assert.h for explanation. works with external libraries/code
  // see, https://stackoverflow.com/questions/50915274/redirecting-assert-fail-messages

  // legacy version
  usart1_printf("\nassert simple failed %s: %d: %s: '%s'\n", file, line, func, expr);

  critical_error_led_blink();
}


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
  usart1_printf("sp %p   %d\n", (void*)&p,  ( (unsigned)(void*)&p)  - 0x20000000   );
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

