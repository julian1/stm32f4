

// the advantage of assert.h in a separate file, is that we use it in library includes.


#include "util.h"   // usart_printf() and critical_error_blink();


#include "assert.h"   // assert_simple()


/*
  TOO MUCH setup COMPLEXITY?

  If using a handler - then we should pass the arguments
*/

#if 0
// we need to be able to call halt to switch things off
static void (*assert_f)(void *ctx ) = NULL;
static void *assert_ctx  = NULL;



void assert_setup(void (*pf)(void *), void *ctx )
{
  assert_f  = pf;
  assert_ctx = ctx;

}

void assert_simple(const char *file, int line, const char *func, const char *expr)
{

  usart_printf("\nassert failed %s %d %s '%s'\n", file, line, func, expr);
  // note tx-interupt should continue to work to flush output buffer, even jump to critical_error_blink()

  /*
    For safety, *must* transition to halt condition here....
  */
  if(assert_f) {
    usart_printf("calling assert handler\n");
    assert_f( assert_ctx);
  } else {
    usart_printf("\nno assert handler. call critical_error_binlk() instead\n");
    critical_error_blink();
  }

}

#endif


// typedef void assert_t(void *ctx, const char *file, int line, const char *func, const char *expr);


static void assert_default(void *ctx, const char *file, int line, const char *func, const char *expr)
{
  UNUSED(ctx);
  usart_printf("\nassert failed %s: %d: %s: '%s'\n", file, line, func, expr);
}



static void (*assert_f)(void *ctx, const char *file, int line, const char *func, const char *expr) = assert_default;
static void *assert_ctx  = NULL;



void assert_setup(void (*pf)(void *ctx, const char *file, int line, const char *func, const char *expr ), void *ctx )
{
  assert_f  = pf;
  assert_ctx = ctx;
}


void assert_simple(const char *file, int line, const char *func, const char *expr)
{
  assert_f( assert_ctx, file, line, func, expr);
}



