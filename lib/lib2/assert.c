/*
  override assert() function, for the led blink and because no appropriate exit()
  not because we need to redirect the stderr stream


  Might be easier to use, on_exit(),  or override exit() instead?

         on_exit - register a function to be called at normal process termination

       #include <stdlib.h>
       int on_exit(void (*function)(int, void *), void *arg);
  ----

  reason not to use exist handler- is extra stack overhead. when may have run out of mem.

*/


#include <libopencm3/stm32/gpio.h>

#include <assert.h>  // assert_simple()
#include <stdio.h>  // printf




/*
  state for which led/gpio to blink
*/
static uint32_t led_port = 0;
static uint32_t led_io = 0;


void assert_critical_error_led_setup(uint32_t port_, uint16_t io_ )
{
  led_port = port_;
  led_io   = io_;

}

void assert_critical_error_led_blink(void)
{
  // non static, to support other direct callers
  // needs the led port config.
  // avoid passing arguments, consuming stack.
  for (;;) {

    gpio_toggle(led_port, led_io);

		for(uint32_t i = 0; i < 500000; ++i)
       __asm__("nop");
  }
}



void assert_simple(const char *file, int line, const char *func, const char *expr)
{
  // this works by using local "assert.h" with assert() macro definition
  // see assert.h for explanation. works with external libraries/code
  // see, https://stackoverflow.com/questions/50915274/redirecting-assert-fail-messages

  // legacy version
  // usart1_printf("\nassert simple failed %s: %d: %s: '%s'\n", file, line, func, expr);
  printf("\nassert simple failed %s: %d: %s: '%s'\n", file, line, func, expr);

  // go to endless loop
  assert_critical_error_led_blink();
}



