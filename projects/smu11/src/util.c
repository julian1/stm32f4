/*
  helper stuff that belongs in separate file, but not in separate library
  because might change

  also. port setup. eg. led blinker. that is very common. but different between stm32 series/part.
*/

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>


#include <libopencm3/stm32/gpio.h>

#include "util.h"
#include "buffer.h"
#include "usart2.h"
// #include "miniprintf2.h" // internal_vprint


#include <stddef.h> // size_t
#include <stdarg.h> // va_starrt etc
#include <stdio.h>  // vsprintf





////////////////////////////////////////////////////////

// DON"T MOVE THIS CODE TO A LIBRARY
// just keep as separate file. because led will change

#define LED_PORT      GPIOE
#define LED_OUT       GPIO0



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
		gpio_toggle(LED_PORT,LED_OUT);
		for(uint32_t i = 0; i < 500000; ++i)
       __asm__("nop");
	}
}




////////////////////////////////////////////////////////

/* Common function descriptions */
// #include "clock.h"

/* milliseconds since boot */
volatile uint32_t system_millis;



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
    this is an interupt context. shouldn't do any work in here
  */

  system_millis++;
}


// defined in util.h
void msleep(uint32_t delay)
{
  // TODO. review. don't think this works on wrap around
  uint32_t wake = system_millis + delay;
#if 0
  // this can lock up....
  if(wake < delay) {

    // wait for overflow
    while (system_millis > (UINT32_MAX / 2));
  }
#endif
  while (system_millis < wake);
}









////////////////////////////////////////////////////////



static CBuf *console_out = NULL;



void usart_printf_init( CBuf *output)
{
  console_out = output;
}



void usart_printf( const char *format, ... )
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
	internal_vprintf((void *)cBufPut, console_out, format, args);
	va_end(args);
#endif

  /*
    - this is not great. but allows using arm-gcc libc sprintf
    if uses 1000 chars. need to report buffer overflow.
    - also could overflow the console buffer.
    - would be better if could write to the console output directly. but we would have to implement the FILE structure.
  */

  char buf[1000];
	va_list args;
	va_start(args, format);
	int n = vsnprintf(buf, 1000, format, args);
	va_end(args);

  char *p = buf;
  while(p < buf + n)  {

    if(*p == '\n')
      cBufPut(console_out, '\r');

    cBufPut(console_out, *p);

    ++p;
  }
}


void usart_flush( void)
{
  // this avoids having to pull in usart.h stuff as dependency
  // don't think we need this as a separrate function
  usart_sync_flush();
}


