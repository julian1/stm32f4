




extern void led_setup(void) ;
extern void led_blink_task(void *args __attribute((unused))) ;

// extern void usart1_isr(void); // already defined in libopencm3


extern void usart_setup(void);
extern char *uart_gets( char *buf, size_t len); 
extern int uart_printf(const char *format,...); 


extern void uart_task(void *args __attribute__((unused))) ;

extern void prompt_task(void *args __attribute__((unused))) ;

