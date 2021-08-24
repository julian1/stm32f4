/*
  - The handler, with ctx, is needed
    eg. to pass app ctx, in order to do stuff like hardware shutdown/ power off for safety.
    or call critical_error_blink() to halt etc

*/


#include "assert.h"   // assert_simple()
#include "util.h"     // usart_printf()




static void assert_default(void *ctx, const char *file, int line, const char *func, const char *expr)
{
  // cannot use assert_pf_t here... because parameters will be unnamed...
  UNUSED(ctx);
  usart_printf("\nassert failed %s: %d: %s: '%s'\n", file, line, func, expr);
}

static assert_pf_t *assert_pf = assert_default;
static void *assert_ctx  = NULL;



void assert_set_handler( assert_pf_t *pf, void *ctx )
{
  assert_pf  = pf;
  assert_ctx = ctx;
}


void assert_simple(const char *file, int line, const char *func, const char *expr)
{
  assert_pf( assert_ctx, file, line, func, expr);
}



