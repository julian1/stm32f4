


#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>

// #include <libopencm3/cm3/scb.h>

#include <stddef.h> // size_t



#include "miniprintf2.h"
#include "usart2.h"

#define LED_PORT  GPIOE
#define LED_OUT   GPIO15


static void critical_error_blink(void)
{
	for (;;) {
		gpio_toggle(LED_PORT,LED_OUT);
		for(uint32_t i = 0; i < 500000; ++i)
       __asm__("nop");
	}
}




////////////////////////////////////////////////////////


static void led_setup(void)
{
  gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_OUT);
}



////////////////////////////////////////////////////////

/* Common function descriptions */
// #include "clock.h"

/* milliseconds since boot */
static volatile uint32_t system_millis;



/* simple sleep for delay milliseconds */
static void msleep(uint32_t delay)
{
  // TODO. review. don't think this works on wrap around
  uint32_t wake = system_millis + delay;
  while (wake > system_millis);
}


static void clock_setup(void)
{
  /* clock rate / 168000 to get 1mS interrupt rate */
  // systick_set_reload(168000);
  systick_set_reload(16000);
  systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
  systick_counter_enable();

  /* this done last */
  systick_interrupt_enable();
}


////////////////////////////////////////////////////////




static A *console_out = NULL;
static A *console_in = NULL;


void sys_tick_handler(void)
{
  // equivalent to a software timer
  // we are in an interupt  context here... so don't do anything
  // NOTE. we could actually set a flag.  or a vector of functions to call..
  // and then process in main loop.
  // eg. where things can be properly sequenced

  system_millis++;

  // 500ms.
  if( system_millis % 500 == 0) {

    gpio_toggle(LED_PORT, LED_OUT);
  }


  // one sec timer
  if( system_millis % 1000 == 0) {

    mini_snprintf( (void*)write, console_out, "whoot %d\r\n", system_millis);
  }

}


/*
  OK. EXTREME.
    in a long func,
    we need to wait for control to return to superloop to flush console output...
    can call at anytime
*/

int main(void)
{
  // LED
  rcc_periph_clock_enable(RCC_GPIOE);

  // USART
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_USART1);


  // rcc_periph_clock_enable(RCC_SYSCFG); // should only be needed for external interupts.


  // initialize buffers
  char buf1[1000];
  struct A out;
  console_out = &out;
  init(console_out, buf1, sizeof(buf1));


  char buf2[1000];
  struct A in;
  console_in = &in;
  init(console_in, buf2, sizeof(buf2));



  led_setup();
  usart_setup( console_out, console_in );
  clock_setup();

  mini_snprintf((void *)write, console_out, "starting\r\n");

  while(true) {

    // led_update(); in systick.

    usart_input_update();
    usart_output_update();
  }


	for (;;);
	return 0;
}




#if 0

static void led_update(void)
{
  bool state = (system_millis / 500)  % 2 == 0;
  // TODO, check last state... to avoid resetting.
  // bool state = (system_millis % 1000) > 500;
  if(state) {
    gpio_set(LED_PORT, LED_OUT);
  } else {
    gpio_clear(LED_PORT, LED_OUT);
  }
}
#endif

#if 0
/* Getter function for the current time */
static uint32_t mtime(void)
{
  return system_millis;
}
#endif

