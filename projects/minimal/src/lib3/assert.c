

/*
  see
  ../../lib/libopencm3/lib/cm3/assert.c
  ../../lib/libopencm3/include/libopencm3/cm3/assert.h

  should just be able to override, the weak linkage specifier.

     cm3_assert_failed_verbose
      andjjjjjjjjj

  ------

  In C/C++: A failed assert prints an error message to stderr and then calls abort(), which terminates the program.
  so consider leave assert as is.
  and just override abort() to add the led blink function,


  the reason to avoid abort()/exit() handler- is the extra call stack overhead.
  ie. control is already problematic, and we may have run out of stack.

*/



#include <assert.h>  // assert_simple()
#include <stdio.h>  // printf


#include <libopencm3/stm32/gpio.h>


/*
  state for which led/gpio to blink
*/
static uint32_t led_port = 0;
static uint32_t led_no = 0;


void assert_critical_error_led_setup( uint32_t port_, uint16_t led_no_ )
{
  led_port = port_;
  led_no   = led_no_;

}

void assert_critical_error_led_blink()
{

  // ensure port/pin configured.
  gpio_mode_setup( led_port, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, led_no);
  gpio_set_output_options( led_port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, led_no);


  // non static, to support other direct callers
  // needs the led port config.
  // avoid passing arguments, consuming stack.
  for (;;) {

    gpio_toggle( led_port, led_no);

		for(uint32_t i = 0; i < 500000; ++i)
       __asm__("nop");
  }
}



void assert_simple( const char *file, int line, const char *func, const char *expr)
{
  /*
    overide macro definition in local <assert.h> that will get included in priority
    works well with external libraries/code
    but only if placed in top-level path for those Makefiles. eg.
    consider move back to
    ../../include/assert.h
  */

  // see, https://stackoverflow.com/questions/50915274/redirecting-assert-fail-messages

  printf("\nassert simple failed %s: %d: %s: '%s'\n", file, line, func, expr);

  // loop
  assert_critical_error_led_blink();
}



