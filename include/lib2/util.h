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


#include <stdint.h> // uint32_t



/*
  farm-out led port config state for critical error led blink
  since for assert() cannot take a context
*/

void critical_error_led_setup(uint32_t port_, uint16_t io_ );
void critical_error_led_blink(void);





void systick_setup(uint32_t tick_divider);
void systick_handler_set( void (*pfunc)(void *),  void *ctx);





/*
  - passing system_millis explicitly is good, and makes it clear what the dependency is.
  - could pass sleep dependent object/environment.
  - but it doesn't matter. for single variable.
*/

void msleep(uint32_t delay, volatile uint32_t *system_millis );

// void msleep_with_yield(uint32_t delay, volatile uint32_t *system_millis,  void (*yield)(void *), void *yield_ctx  );
void yield_with_msleep(uint32_t delay, volatile uint32_t *system_millis,  void (*yield)(void *), void *yield_ctx  );


void print_stack_pointer(void);




#define UNUSED(x) (void)(x)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))



#ifdef __cplusplus
}
#endif

