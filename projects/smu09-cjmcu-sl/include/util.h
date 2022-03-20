/*
  helper stuff that belongs in separate file, but not in separate library
  because might change
*/


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
extern void usart1_flush(void);

#define UNUSED(x) (void)(x)
