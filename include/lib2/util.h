/*
  helper stuff that belongs in separate file, but not in separate library
  because might change
	--------

  TODO better suffix name.   _init() instead of _setup()
*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#if 0
#include <stdint.h> // uint32_t
#endif

#if 0

void systick_setup(uint32_t tick_divider);
void systick_handler_set( void (*pfunc)(void *),  void *ctx);

#endif



/*
  - passing system_millis explicitly is good, and makes it clear what the dependency is.
  - could pass sleep dependent object/environment.
  - but it doesn't matter. for single variable.
*/


/*
void msleep(uint32_t delay, volatile uint32_t *system_millis );

void yield_with_msleep(uint32_t delay, volatile uint32_t *system_millis,  void (*yield)(void *), void *yield_ctx  );
*/

#if 0
void print_stack_pointer(void);

#endif


#define UNUSED(x) ((void)(x))

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))



#ifdef __cplusplus
}
#endif

