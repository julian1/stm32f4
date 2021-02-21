


#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>

// #include <libopencm3/cm3/scb.h>

#include <stddef.h> // size_t
#include <math.h> // nanf

#include <stdio.h>



#include "miniprintf2.h"
#include "usart2.h"
#include "util.h"

#include "ads131a04.h"
#include "stats.h"






////////////////////////////////////////////////////////

#define LED_PORT  GPIOE
#define LED_OUT   GPIO15



static void led_setup(void)
{
  gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_OUT);
}



////////////////////////////////////////////////////////

/* Common function descriptions */
// #include "clock.h"

/* milliseconds since boot */
static volatile uint32_t system_millis;



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

// implement critical_error_blink() msleep() and usart_printf()
// eg. rather than ioc/ dependency injection, just implement

void critical_error_blink(void)
{
	for (;;) {
		gpio_toggle(LED_PORT,LED_OUT);
		for(uint32_t i = 0; i < 500000; ++i)
       __asm__("nop");
	}
}


void msleep(uint32_t delay)
{
  // TODO. review. don't think this works on wrap around
  uint32_t wake = system_millis + delay;
  while (wake > system_millis);
}



static char buf1[1000];
static char buf2[1000];

static CBuf console_out;
static CBuf console_in;


static float fbuf[1000];
static FBuf ffbuf;



void usart_printf( const char *format, ... )
{
  // TODO rename to just printf... it's not the responsibiilty of user to know context
  // can override the write, to do flush etc.
	va_list args;

	va_start(args,format);
	internal_vprintf((void *)cBufWrite, &console_out, format,args);
	va_end(args);
}

void flush( void)
{
  usart_sync_flush();
}



///////////////////////////

void sys_tick_handler(void)
{
  /*
    this is an interupt. not sure how much work should do here.
    albeit maybe its ok as dispatch point.

    it will interrupt other stuff...

  */
  // equivalent to a rtos software timer
  // we are in an interupt  context here... so don't do anything
  // NOTE. we could actually set a flag.  or a vector of functions to call..
  // and then process in main loop.
  // eg. where things can be properly sequenced

  system_millis++;


  // 500ms.
  if( system_millis % 500 == 0) {

    gpio_toggle(LED_PORT, LED_OUT);
  }

  float buf[1000];
  uint32_t sz = 0;

  // OK. stddev is much the same. whether 100ms or 1000ms...
  // interesting

  // one sec timer
  if( system_millis % 100 == 0) {

    // usart_printf(".");

    // clear the circular buffer
    while(!fBufEmpty(&ffbuf)) {
      float val = fBufRead(&ffbuf);
      // buf don't overflow local buffer
      if(sz < 1000) {
        buf[sz++] = val;
      } else {
        usart_printf("overflow\n");
      }
    }

    // maybe float is not enabled???
    // is our character output buffer wrapping?

    usart_printf("---------\n");

  // need sample freq... eg. 100 / sz

    usart_printf("n %d, mean %7f, stddev2*e6 %6f\n", sz, mean( buf, sz), stddev2(buf, sz) * powf(10,6));

#if 0
    usart_printf("rms    %f\n", rms( buf, sz) );
#endif
  }
}






static void adc_update(FBuf *float_buffer)
{
  static uint32_t state = 0;
  switch(state) {
    case 0: adc_setup_spi(); ++state; return;
    case 1: adc_reset(); ++state; return; // TODO check erroro
    case 2: adc_exti_setup( float_buffer); ++state; return;
    default:
      break;
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

  // adc02
  rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_SPI2);


  rcc_periph_clock_enable(RCC_SYSCFG); // exti gpio interupts.


  cBufInit(&console_out, buf1, sizeof(buf1));
  cBufInit(&console_in, buf2, sizeof(buf2));
  // fBufInit(&ffbuf, fbuf, 1000 );
  fBufInit(&ffbuf, fbuf, sizeof(fbuf) / sizeof(float) );



  led_setup();
  usart_setup(&console_out, &console_in);
  clock_setup();

  usart_printf("\n--------\n");
  usart_printf("starting\n");
  // usart_printf("size %d\n", sizeof(fbuf) / sizeof(float));

  while(true) {

    // EXTREME - can actually call update at any time, in a yield()...
    // so long as we wrap calls with a mechanism to avoid stack reentrancy
    // led_update(); in systick.

    usart_input_update();
    usart_output_update();
    adc_update(&ffbuf);
  }


	for (;;);
	return 0;
}




