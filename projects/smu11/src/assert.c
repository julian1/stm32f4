

// the advantage of assert.h in a separate file, is that we use it in library includes.


#include "util.h"   // usart_printf() and critical_error_blink();


#include "assert.h"   // assert_simple()


// we need to be able to call halt to switch things off
static void (*assert_pf)(void *ctx ) = NULL;
static void *assert_ctx  = NULL;



void assert_setup(void (*pf)(void *), void *ctx )
{
  assert_pf  = pf;
  assert_ctx = ctx;

}

void assert_simple(const char *file, int line, const char *func, const char *expr)
{

  usart_printf("\nassert failed %s %d %s '%s'\n", file, line, func, expr);
  // note tx-interupt should continue to work to flush output buffer, even jump to critical_error_blink()

  /*
    For safety, *must* transition to halt condition here.... 
  */
  if(assert_pf) {
    usart_printf("calling assert handler\n");
    assert_pf( assert_ctx);
  } else {
    usart_printf("\nno assert handler. call critical_error_binlk() instead\n");
    critical_error_blink();
  }

}

