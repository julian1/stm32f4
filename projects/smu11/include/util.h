/*
  helper stuff that belongs in separate file, but not in separate library
  because might change
*/

#include <stddef.h> // size_t

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

extern size_t usart_mark(void);



#define UNUSED(x) (void)(x)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))



/*
  add an assert with critical error blink...
  can still try to log to usart.
  probably want a critical_usart_write() 
*/

extern void assert_simple(const char *file, int line, const char *func, const char *expr);

#define ASSERT(expr)    ((expr) ? ((void)0) : assert_simple(__FILE__, __LINE__, __func__, #expr))




