

// high level serial/console/log functions... no reference to freertos functions.


#include <stddef.h>       // size_t


extern void usart_putc_from_isr(char ch) ;  // should remove... used by pvd to try and catch interrupt.

extern int usart_printf(const char *format,...);

extern char *usart_gets( char *buf, size_t len);


extern void serial_prompt_task(void *args __attribute__((unused))) ;



