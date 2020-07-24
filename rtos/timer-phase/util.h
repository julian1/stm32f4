


extern void usart_setup(void);


extern void led_setup(void) ;

// extern void usart1_isr(void); // already defined in libopencm3


extern char *uart_gets( char *buf, size_t len); 

extern int uart_printf(const char *format,...); 

extern void task1(void *args __attribute((unused))) ;

extern void uart_task(void *args __attribute__((unused))) ;

extern void demo_task(void *args __attribute__((unused))) ;

