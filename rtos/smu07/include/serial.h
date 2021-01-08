

// high level serial/console/log functions... no reference to freertos functions.


#include <stddef.h>       // size_t


extern void uart_putc_from_isr(char ch) ;  // should remove... used by pvd to try and catch interrupt.

extern int uart_printf(const char *format,...);

extern char *uart_gets( char *buf, size_t len);


extern void serial_prompt_task(void *args __attribute__((unused))) ;



