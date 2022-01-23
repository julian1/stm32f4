
/*
  NOT deprecated. although assert.c is.

  We cannot over-ride libc version at link time, because of strong link specifiers.

  but we can include this file in the path, so it gets included with
  #include <assert.h>
    not just
  #include "assert.h"

  this way it's included for external libraries, so long as -E path is used.

  // see, https://stackoverflow.com/questions/50915274/redirecting-assert-fail-messages

*/



#pragma once

#ifdef __cplusplus
extern "C" {
#endif


// implement in util.c because project specific
extern void assert_simple(const char *file, int line, const char *func, const char *expr);

#define assert(expr)    ((expr) ? ((void)0) : assert_simple(__FILE__, __LINE__, __func__, #expr))



#ifdef __cplusplus
}
#endif


