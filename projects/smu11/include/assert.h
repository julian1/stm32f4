
// the advantage of assert.h in a separate file, is that we use it in library includes.

/*
  add an assert with critical error blink...
  can still try to log to usart.
  probably want a critical_usart_write() 
*/

extern void assert_simple(const char *file, int line, const char *func, const char *expr);

#define ASSERT(expr)    ((expr) ? ((void)0) : assert_simple(__FILE__, __LINE__, __func__, #expr))



