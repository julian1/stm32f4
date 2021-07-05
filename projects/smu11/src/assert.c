

// the advantage of assert.h in a separate file, is that we use it in library includes.


#include "util.h"   // usart_printf() and critical_error_blink();


#include "assert.h"   // assert_simple() 



void assert_simple(const char *file, int line, const char *func, const char *expr)
{

  usart_printf("\nassert failed %s %d %s '%s'\n", file, line, func, expr);
  // note tx-interupt should continue to work to flush output buffer, even jump to critical_error_blink()
  critical_error_blink();
/*
  either,
  critical_error_blink()...
  TODO - for prod, go to halt state
*/
}

