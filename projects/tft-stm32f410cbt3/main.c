


#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>


#include <libopencm3/stm32/timer.h>

// #include <libopencm3/cm3/scb.h>

#include <stddef.h> // size_t
//#include <math.h> // nanf
//#include <stdio.h>


#include "buffer.h"
#include "miniprintf2.h"
#include "usart2.h"
#include "util.h"


#include "lcd_spi.h"
#include "context.h"
#include "gfx.h"
#include "Adafruit_ILI9341.h"


#include "rotary.h"


////////////////////////////////////////////////////////

#define LED_PORT  GPIOA
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



// static void clock_setup(void)
static void clock_setup(uint32_t tick_divider)
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


////////////////////////////////////////////////////////

// implement critical_error_blink() msleep() and usart_printf()
// eg. rather than ioc/ dependency injection, just implement

void critical_error_blink(void)
{
  // avoid passing arguments, consuming stack.
	for (;;) {
		gpio_toggle(LED_PORT,LED_OUT);
		for(uint32_t i = 0; i < 500000; ++i)
       __asm__("nop");
	}
}

// defined in util.h
void msleep(uint32_t delay)
{
  // TODO. review. don't think this works on wrap around
  uint32_t wake = system_millis + delay;
  if(wake < delay) {

    // wait for overflow
    while (system_millis > (UINT32_MAX / 2));
  }
  while (system_millis < wake);
}



static char buf1[1000];
static char buf2[1000];

static CBuf console_in;
static CBuf console_out;




void usart_printf( const char *format, ... )
{
  // TODO rename to just printf... it's not the responsibiilty of user to know context
  // can override the write, to do flush etc.
	va_list args;

	va_start(args,format);
	internal_vprintf((void *)cBufPut, &console_out, format,args);
	va_end(args);
}

void flush( void)
{
  usart_sync_flush();
}





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


  // 100ms.
  if( system_millis % 100 == 0) {
      // print rotary encoder
      int count = timer_get_counter(TIM1);
      usart_printf("rotary count.. %d\n", count);
  }


  // 500ms.
  if( system_millis % 500 == 0) {
    // blink led
    gpio_toggle(LED_PORT, LED_OUT);
  }

}



static void drawText(Context *ctx, const char *s)
{
  while(*s) {
    write(ctx, *s);     // This won't work very well with printf if have to pass a context...
    ++s;
  }
}




static void lcd_do_stuff(void)
{
  Context   ctx;

  // low level
  initialize(&ctx);

  ILI9341_setRotation(&ctx, 3); // 0 == trhs, 1 == brhs, 2 == blhs,  3 == tlhs


  // gfx
  fillScreen(&ctx, ILI9341_WHITE );
  fillRect(&ctx, 20, 20, 40, 20, ILI9341_RED );

#if 1
  writeFastHLine(&ctx, 50, 40, 100, ILI9341_BLUE);

  writeLine(&ctx, 10, 10, 190, 70, ILI9341_RED);
  drawCircle(&ctx, 40, 40, 50, ILI9341_BLUE) ;

  drawChar(
    &ctx,
    60, 60, '8',                        // int16_t x, int16_t y, unsigned char c,
    ILI9341_BLACK, ILI9341_BLACK,       // uint16_t color, uint16_t bg,
    10, 10);                            // uint8_t size_x, uint8_t size_y);



  setTextColor(&ctx, ILI9341_BLUE);
  setCursor(&ctx, 50, 50);
  setTextSize(&ctx, 0.3, 0.3);

  // ok. this will actually wrap correctly...
  drawText(&ctx, "hi there friends all  ");

  drawText(&ctx, "77.123");
#endif

}




int main(void)
{
  // high speed internal!!!
  // this appears to work... 1Hz clock goes to 5Hz.
  // although should be able to go to 100MHz.
  // rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_3V3_84MHZ ]);

  clock_setup(16000);

  // LED
  rcc_periph_clock_enable(RCC_GPIOA);

  // TFT
  rcc_periph_clock_enable(RCC_SPI1);

  // USART
  // rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOB); // F410
  rcc_periph_clock_enable(RCC_USART1);


  cBufInit(&console_in, buf1, sizeof(buf1));
  cBufInit(&console_out, buf2, sizeof(buf2));


  led_setup();
  usart_setup_gpio_portB();
  usart_setup(&console_in, &console_out);

  ///////////////////////
  // rotary
  // PA8,PA9 TIM1 (advanced)   AF1.
  rcc_periph_clock_enable(RCC_TIM1);
 
  // PA6 and PA7
  // initRotaryEncoderTimer(TIM3, GPIOA, GPIO6, GPIO_AF2, GPIOA, GPIO7, GPIO_AF2) ;
  initRotaryEncoderTimer(TIM1, GPIOA, GPIO8, GPIO_AF1, GPIOA, GPIO9, GPIO_AF1) ;

  ///////////////////////




  usart_printf("\n--------\n");
  usart_printf("starting\n");
  // usart_printf("size %d\n", sizeof(fbuf) / sizeof(float));


  // lcd...
  usart_printf("doing lcd_spi_setup()\n");
  lcd_spi_setup();

  usart_printf("doing lcd_do_stuff()\n");
  lcd_do_stuff();

  while(true) {

    // EXTREME - can actually call update at any time, in a yield()...
    // so long as we wrap calls with a mechanism to avoid stack reentrancy
    // led_update(); in systick.

    usart_input_update();
    usart_output_update();
  }


	for (;;);
	return 0;
}




