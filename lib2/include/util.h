/*
  helper stuff that belongs in separate file, but not in separate library
  because might change
*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#include <stddef.h> // size_t
#include <stdbool.h> // bool
#include <stdint.h> // uint32_t

// extern bool strequal(const char *s1, const char *s2);



// change name led_blink_setup()? 
extern void led_setup(uint32_t port_, uint16_t io_ );

// extern void led_setup(void);
extern void led_toggle(void);
extern void critical_error_blink(void);


/////////////////////


extern volatile uint32_t system_millis;

extern void systick_setup(uint32_t tick_divider);

extern void msleep(uint32_t delay);


void print_stack_pointer(void);




///////////////


#define UNUSED(x) (void)(x)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))



#ifdef __cplusplus
}
#endif

