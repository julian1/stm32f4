/*
  helper stuff that belongs in separate file, but not in separate library
  because might change
*/

#include <stddef.h> // size_t

extern char * uint_to_bits(char *buf, size_t width, uint32_t value);
extern bool strequal(const char *s1, const char *s2);




extern void led_setup(void);
extern void led_toggle(void);
extern void critical_error_blink(void);


/////////////////////


extern void systick_setup(uint32_t tick_divider);
extern volatile uint32_t system_millis;
extern void msleep(uint32_t delay);

///////////////

typedef struct CBuf CBuf;

extern void usart_printf_init(CBuf *output);
extern void usart_printf(const char *format, ... );
extern void usart_flush(void);

#define UNUSED(x) (void)(x)
