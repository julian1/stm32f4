/*
  should be common now.
  ----
  helper stuff that belongs in separate file, but not in separate library
  because might change

  also. port setup. eg. led blinker. that is very common. but different between stm32 series/part.
*/

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>


#include <libopencm3/stm32/gpio.h>

#include "util.h"
#include "streams.h"
#include "assert.h"  // assert simple



/*
  issue here - is maintaining led state so that critical_error_bllink can work.

  void  gpio_mode_setup (uint32_t gpioport, uint8_t mode, uint8_t pull_up_down, uint16_t gpios)
*/
typedef struct  Led
{
  uint32_t  port;
  uint32_t  io;
} Led;

static Led   led;


/*
// DON"T MOVE THIS CODE TO A LIBRARY
// just keep as separate file. because led will change
// stm32f407 ...
// cjmcu
// #define LED_PORT  GPIOE
// #define LED_OUT   GPIO0

#define LED_PORT  GPIOA
#define LED_OUT   GPIO15
*/





/*
  May 19 2022.
    really think should just use a structure for the led blink.  To init once in main, and encapsulate
    And then pass by reference. like we do  for spi, etc.

    eg
    led_setup( &app->led, GPIOA,  GPIO15 );
    led_toggle( &app->led );

    - issue is having to pass arg/or initialize for critical_error_blink()

*/

// move this stuff back to main.c and setup...


// change name led_blink_setup()?

void led_setup(uint32_t port_, uint16_t io_ )   // innit or setup
{
  led.port = port_;
  led.io   = io_;

  gpio_mode_setup(led.port, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, led.io );
}

/*
void led_setup(void)
{
  gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_OUT);
}
*/



void led_toggle(void)
{
  gpio_toggle(led.port, led.io);
}


void led_set(bool val )
{
  // is this 
  // val ? gpio_set(led.port, led.io) : gpio_clear (led.port, led.io);

  // clear turns it on???? 
  val ? gpio_clear(led.port, led.io) : gpio_set(led.port, led.io);
}






void critical_error_blink(void)
{
  // needs the led port config.
  // avoid passing arguments, consuming stack.
	for (;;) {
		// gpio_toggle(led.port, led.io );
    led_toggle();

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
  usart1_printf("\nassert simple failed %s: %d: %s: '%s'\n", file, line, func, expr);

  critical_error_blink();
}







