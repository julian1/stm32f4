/*

  required to over-ride behavior for exit/ critical_error_blink().
  the stream handling already works with stderr redirection

  --------
  needs to be put top-level.


  It isn't possible to override libc function at link time, because of strong link specifiers.

  this way it's included for external libraries, so long as -E path is used.

  // see, https://stackoverflow.com/questions/50915274/redirecting-assert-fail-messages

*/



#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h> // uint32_t


void assert_critical_error_led_setup(uint32_t port_, uint16_t io_ );
void assert_critical_error_led_blink(void);


// implement in util.c because project specific
void assert_simple(const char *file, int line, const char *func, const char *expr);

#define assert(expr)    ((expr) ? ((void)0) : assert_simple(__FILE__, __LINE__, __func__, #expr))



#ifdef __cplusplus
}
#endif


