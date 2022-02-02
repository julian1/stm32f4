/*
  generalized helper stuff that is too specific to push to library code

*/


#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>


#include <libopencm3/stm32/gpio.h>

#include "util.h"
#include "cbuffer.h"
#include "usart2.h"
#include "assert.h"
#include "streams.h"
// #include "miniprintf2.h" // internal_vprint

#include <string.h>  // strcmp




bool strequal(const char *s1, const char *s2)
{
  return (strcmp(s1, s2) == 0);
}




////////////////////////////////////////////////////////

// DON"T MOVE THIS CODE TO A LIBRARY
// just keep as separate file. because led will change

// stm32f410cbt3

// #define LED_PORT  GPIOA
// #define LED_OUT   GPIO15
// #define LED_OUT   GPIO9 // f411

// f407
#define LED_PORT      GPIOE
#define LED_OUT       GPIO0





// move this stuff back to main.c and setup...

void led_setup(void)
{
  gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_OUT);
}



void led_toggle(void)
{
  gpio_toggle(LED_PORT, LED_OUT);
}



void critical_error_blink(void)
{
  // needs the led port config.
  // avoid passing arguments, consuming stack.
  // kind of depends on what clk is configured.
	for (;;) {
		gpio_toggle(LED_PORT, LED_OUT);
		for(uint32_t i = 0; i < 500000; ++i)
       __asm__("nop");
	}
}




////////////////////////////////////////////////////////

/* Common function descriptions */
// #include "clock.h"

/* milliseconds since boot */
volatile uint32_t system_millis = 0;
// volatile uint64_t system_millis;

/*
  just use a uint64_t ?
*/

// static void systick_setup(void)
void systick_setup(uint32_t tick_divider)
{
  // TODO change name systick_setup().
  // TODO pass clock reload divider as argument, to localize.

  /* clock rate / 168000 to get 1mS interrupt rate */
  // systick_set_reload(168000);
  systick_set_reload(tick_divider);
  systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
  systick_counter_enable();

  /* this done last */
  systick_interrupt_enable();
}



void sys_tick_handler(void)
{
  /*
    interupt context. avoid doing work here
  */

  system_millis++;

  /*
    if(system_millis % 1000 == 0) {
      app->soft_timer_1sec = true; ...
    }
  */
}



/*
  https://arduino.stackexchange.com/questions/12587/how-can-i-handle-the-millis-rollover
  https://hsel.co.uk/2014/06/20/stm32f0-tutorial-2-systick/
*/



void msleep(uint32_t delay)
{
  // works for system_millis integer wrap around
  // could be a do/while block.
  uint32_t start = system_millis ;
  while (true) {
    uint32_t elapsed = system_millis - start;
    if(elapsed > delay)
      break;
  };
}






void assert_simple(const char *file, int line, const char *func, const char *expr)
{
  // this works by using local "assert.h" with assert() macro definition
  // see assert.h for explanation. works with external libraries/code
  // see, https://stackoverflow.com/questions/50915274/redirecting-assert-fail-messages

  // legacy version
  usart_printf("\nassert simple failed %s: %d: %s: '%s'\n", file, line, func, expr);

  critical_error_blink();
}



















/*
// test code for integer wrap around

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

volatile uint32_t  system_millis = 0;

static void msleep(uint32_t delay)
{
  uint32_t start = system_millis ;
  printf("start %u\n" , start );
  while (true) {
    uint32_t elapsed = system_millis - start;
    printf("system_millis %u,   elapsed %u\n" ,   system_millis,  elapsed );
    if(elapsed > delay)
      break;
    ++system_millis;
  };
}

int main()
{
  system_millis = UINT32_MAX  - 10;
  msleep(5);
  printf("\n");
  system_millis = UINT32_MAX  - 10;
  msleep(15);
}
*/









// #include <stdio.h>
// #include <assert.h>

// extern void __assert_fail(const char * assertion, const char * file, unsigned int line, const char * function);

/*
  // this is *NOT* being called now????

  - because libc version is not marked weak.
  - the problem with a local assert.h other library code we don't control where assert is used.

*/
#if 0
extern void __assert_fail (const char *__assertion, const char *__file,
               unsigned int __line, const char *__function)
__THROW __attribute__ ((__noreturn__));

void __assert_fail(const char * assertion, const char * file, unsigned int line, const char * function)
{
  // this is *NOT* being called.
  // SHOULD get linked in priority to libc version.
  //
  // fprintf(stderr, "My custom message\n");
  usart_printf("\nwhoot assert failed %s: %d: %s: '%s'\n", file, line, function, assertion );
  critical_error_blink();
}

void exit( int jjkx )
{

}
#endif





////////////////////////////////////////////////////////







#if 0

static CBuf *console_out = NULL;

/*
  TODO.
    should remove the global. and instead
    just install the cookie.
  as the cookie.
*/

void usart_printf_init(CBuf *output)
{
  // for usart_printf
  console_out = output;
}

#endif


////////////////////////////////////////////////////////






#if 0
static void output_char( int ch)
{
    if(ch  == '\n') {
      cBufPush(console_out, '\r');
    }

    cBufPush(console_out, ch);

}

static int printf_base( const char *format, va_list args)
{

  char buf[1000];
	int n = vsnprintf(buf, 1000, format, args);

  for( char *p = buf;  p < buf + n; ++p)  {
    output_char( *p);
  }

  // re-enable tx interupt... if needed
  // could do line buffering here, if wanted.
  usart_output_update();

  return n;
}

void usart_printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
  int ret = printf_base(format, args);
	va_end(args);
  UNUSED(ret);
}


int printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
  int ret = printf_base(format, args);
	va_end(args);
  return ret;
}


int fprintf(FILE *stream, const char *format, ...)
{
  UNUSED(stream);

	va_list args;
	va_start(args, format);
  int ret = printf_base(format, args);
	va_end(args);
  return ret;
}


#if 0
// trying to override this just creates link errors, with libc version.

// BETTER WAY. see what libc putc calls. and then over-ride that.

eg. see
  https://github.com/lattera/glibc/blob/master/libio/putc.c

  gsl matrix.  looks very lightweight.
            code uses two.
  https://github.com/AlisterH/gwc/blob/master/declick.c.new

int putc(int c, FILE *stream)
{
}
#endif



#if 0
// none of this works.
#undef _IO_putc

int
_IO_putc (int c, FILE *fp)
{
  UNUSED(fp);

  output_char( c);
  usart_output_update();
}

#undef putc

putc (int c, FILE *fp)
{
  UNUSED(fp);

  output_char( c);
  usart_output_update();
}
#endif



int mesch_putc(int c, void *stream)
{
  UNUSED(stream);

  output_char( c);
  usart_output_update();

  return c;
}


////////////////////////////
#endif






