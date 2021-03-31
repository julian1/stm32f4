


extern void led_setup(void);
extern void led_toggle(void);
extern void critical_error_blink(void);




/////////////////////


extern void systick_setup(uint32_t tick_divider);
extern volatile uint32_t system_millis;
extern void msleep(uint32_t delay);

///////////////

extern void usart_printf( const char *format, ... );
extern void flush( void);
extern void usart_setup_( void );







