/*
  helper stuff that is project/hardware specific and belongs in separate file, but not in separate library
*/

#include <stddef.h> // size_t
#include <stdbool.h> // bool
#include <stdint.h> // uint32_t


// extern bool strequal(const char *s1, const char *s2);


extern void led_setup(void);
extern void led_toggle(void);
extern void critical_error_blink(void);


/////////////////////


extern volatile uint32_t system_millis;

extern void systick_setup(uint32_t tick_divider);

extern void msleep(uint32_t delay);

///////////////


#define UNUSED(x) (void)(x)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define MIN(A, B) ((A) < (B) ? (A) : (B))


