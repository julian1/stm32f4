/*


  needs to be put top-level. to work with compilation of local libraries.


  It isn't possible to override libc function at link time, because of strong link specifiers.

  this way it's included for external libraries, so long as -E path is used.

  // see, https://stackoverflow.com/questions/50915274/redirecting-assert-fail-messages

*/



#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h> // uint32_t


void assert_critical_error_led_setup( uint32_t port, uint16_t io);
void assert_critical_error_led_blink( void);
void assert_simple( const char *file, int line, const char *func, const char *expr);

#define assert(expr)    ((expr) ? ((void)0) : assert_simple(__FILE__, __LINE__, __func__, #expr))



#ifdef __cplusplus
}
#endif


