/*
  helper stuff that belongs in separate file, but not in separate library
  because might change

  also. port setup. eg. led blinker. that is very common. but different between stm32 series/part.
*/

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>


#include <libopencm3/stm32/gpio.h>




#include "util.h"
#include "streams.h"

#include "assert.h"  // local


#include <stddef.h> // size_t
#include <stdarg.h> // va_starrt etc
#include <stdio.h>  // vsprintf
#include <string.h>  // strcmp



// stm32f407 ...
#define LED_PORT  GPIOB
#define LED_OUT   GPIO4 




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
  // NOTE. doesn not seem to work without external xtal.
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


void assert_simple(const char *file, int line, const char *func, const char *expr)
{
  // this works by using local "assert.h" with assert() macro definition
  // see assert.h for explanation. works with external libraries/code
  // see, https://stackoverflow.com/questions/50915274/redirecting-assert-fail-messages

  // legacy version
  usart_printf("\nassert simple failed %s: %d: %s: '%s'\n", file, line, func, expr);

  critical_error_blink();
}



void print_stack_pointer()
{
  // https://stackoverflow.com/questions/20059673/print-out-value-of-stack-pointer
  // non-portable.
  void* p = NULL;
  usart_printf("sp %p   %d\n", (void*)&p,  ( (unsigned)(void*)&p)  - 0x20000000   );
  // return &p;
}




////////////////////////////////////////////////////////

#if 0

static CBuf *console_out = NULL;



void usart_printf_init(CBuf *output)
{
  console_out = output;
}


void usart_clear()
{

  cBufClear(console_out);

}




void usart_printf(const char *format, ...)
{
  /*
    if(!console_out)
      critical_error_blink();
  */

#if 0
  // cannot rename to just printf... it's not the responsibiilty of user to know context
  // because different formatting chars, conflict with gcc printf builtins
	va_list args;
	va_start(args, format);
	internal_vprintf((void *)cBufPush, console_out, format, args);
	va_end(args);
#endif
  /*
    see, fopencookie for a better way to do this,
  */

  /*
    - this is not great. but allows using arm-gcc libc sprintf
    if uses 1000 chars. need to report buffer overflow.
    - also could overflow the console buffer.
    - would be better if could write to the console output directly. but we would have to implement the FILE structure.
  */

  /*
      TODO can be reworked to avoid the copy to the circular buffer?
  */
  char buf[1000];
	va_list args;
	va_start(args, format);
	int n = vsnprintf(buf, 1000, format, args);
	va_end(args);

  char *p = buf;
  while(p < buf + n)  {

    if(*p == '\n')
      cBufPush(console_out, '\r');

    cBufPush(console_out, *p);

    ++p;
  }


  // re-enable tx interupt... if needed
  // TODO . rename.  usart_txe_interupt_enable()
  usart_output_update();
}

#endif

////////////////////////////








