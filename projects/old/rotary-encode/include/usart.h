


// extern void usart1_isr(void); // already defined in libopencm3

extern void usart1_setup(void);
extern char *uart_gets( char *buf, size_t len); 
extern int uart_printf(const char *format,...); 

extern void uart_task(void *args __attribute__((unused))) ;

// move?
extern void prompt_task(void *args __attribute__((unused))) ;

