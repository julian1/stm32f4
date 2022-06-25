/*
  helper stuff that belongs in separate file, but not in separate library
  because might change

  also. port setup. eg. led blinker. that is very common. but different between stm32 series/part.
*/

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>


#include <libopencm3/stm32/gpio.h>

#include "util.h"
#include "cbuffer.h"
#include "usart.h"
// #include "miniprintf2.h" // internal_vprint


#include <stddef.h> // size_t
#include <stdarg.h> // va_starrt etc
#include <stdio.h>  // vsprintf
#include <string.h>  // strcmp





bool strequal(const char *s1, const char *s2)
{
  return (strcmp(s1, s2) == 0);
}




////////////////////////////////////////////////////////

// DON"T MOVE THIS CODE TO A LIBRARY
// just keep as separate file. because led will change

// stm32f407
// #define LED_PORT      GPIOE
// #define LED_OUT       GPIO0


// stm32f411...
#define LED_PORT  GPIOA
#define LED_OUT   GPIO9 



// move this stuff back to main.c and setup...
// But this code is all identical, except for the led.

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

void systick_setup(uint32_t tick_divider)
{
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






////////////////////////////////////////////////////////



static CBuf *console_out = NULL;



void printf_init(CBuf *output)
{
  console_out = output;
}



void printf(const char *format, ...)
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
  usart1_enable_output_interupt();
}



////////////////////////////








