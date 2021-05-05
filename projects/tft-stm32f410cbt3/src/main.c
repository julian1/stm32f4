/*
  serial,
  screen /dev/ttyUSB0 115200

  screen
  openocd -f openocd.cfg
  rlwrap nc localhost 4444  # in new window
  reset halt ; flash write_image erase unlock ./projects/tft-stm32f410cbt3/main.elf; sleep 1000; reset run

*/


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
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>




////////////////////////////////////////////////////////

#define LED_PORT  GPIOA
#define LED_OUT   GPIO15



static void buzzer_enable(void );
static void buzzer_disable(void );


static void led_setup(void)
{
  gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_OUT);
}



////////////////////////////////////////////////////////

/* Common function descriptions */
// #include "clock.h"

/* milliseconds since boot */
static volatile uint32_t system_millis;




// TODO change name systick_setup()
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
	internal_vprintf((void *)cBufPut, &console_out, format, args);
	va_end(args);
}

void flush( void)
{
  usart_sync_flush();
}




////////////////////////////////////

static void drawText(Context *ctx, const char *s)
{
  while(*s) {
    write(ctx, *s);     // This won't work very well with printf if have to pass a context...
    ++s;
  }
}







static void lcd_do_stuff( Context *ctx)
{
/*
  Context   ctx;

  // low level
  initialize(&ctx);

  ILI9341_setRotation(&ctx, 3); // 0 == trhs, 1 == brhs, 2 == blhs,  3 == tlhs
*/

  // gfx
  fillScreen(ctx, ILI9341_WHITE );
  fillRect(ctx, 20, 20, 40, 20, ILI9341_RED );

#if 1
  writeFastHLine(ctx, 50, 40, 100, ILI9341_BLUE);

  writeLine(ctx, 10, 10, 190, 70, ILI9341_RED);
  drawCircle(ctx, 40, 40, 50, ILI9341_BLUE) ;

  drawChar(
    ctx,
    60, 60, '8',                        // int16_t x, int16_t y, unsigned char c,
    ILI9341_BLACK, ILI9341_BLACK,       // uint16_t color, uint16_t bg,
    10, 10);                            // uint8_t size_x, uint8_t size_y);



  setTextColor(ctx, ILI9341_BLUE);
  setCursor(ctx, 50, 50);
  setTextSize(ctx, 0.3, 0.3);

  // ok. this will actually wrap correctly...
  drawText(ctx, "hi there friends all  ");

  drawText(ctx, "77.123");
#endif

}


////////////////////////////////////
#define ROTARY_SW_PORT  GPIOA
#define ROTARY_SW_IN    GPIO10

static void rotary_sw_exti_setup(void)
{
  // rotary switch is PA10
  // *** EXTI10  needs to be macroed... so not much point defining all this


  gpio_mode_setup(ROTARY_SW_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, ROTARY_SW_IN );

  nvic_enable_irq(NVIC_EXTI15_10_IRQ);
  nvic_set_priority(NVIC_EXTI15_10_IRQ, 5 );

  exti_select_source(EXTI10, ROTARY_SW_PORT);

  exti_set_trigger(EXTI10, EXTI_TRIGGER_FALLING);
  exti_enable_request(EXTI10);
}






////////////////////////////////////
#define TACTILE_SW_PORT  GPIOB
#define TACTILE_SW1_IN    GPIO13
#define TACTILE_SW2_IN    GPIO14


static void tactile_sw_exti_setup(void)
{
  // MUST be careful. to keep interupts on different pin numbers - when using more than one port.
  // eg. cannot get interrupts on both PA10 and PB10.

  uint16_t all = TACTILE_SW1_IN | TACTILE_SW2_IN;

  gpio_mode_setup(TACTILE_SW_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, all );

  // WARN. this is already setup/repeated in the rotary decoder exti setup.
  // this needs much better factoring.
  nvic_enable_irq(NVIC_EXTI15_10_IRQ);
  nvic_set_priority(NVIC_EXTI15_10_IRQ, 5 );

  // works.
  exti_select_source(all, TACTILE_SW_PORT);
  exti_set_trigger(all, EXTI_TRIGGER_FALLING);
  exti_enable_request(all);
}



void exti15_10_isr(void)
{
  // need to decode...
  // eg. exti_reset_request(EXTI10 | TACTILE_SW1_IN | TACTILE_SW2_IN);

  // rotary sw.
  exti_reset_request(EXTI10);

  // tactile switches
  exti_reset_request(EXTI14);
  exti_reset_request(EXTI13);

  // should be in super loop, actually it is very light, just enqueues
  usart_printf("interupt button pressed\n");
}








////////////////////////////////////

// buzzer.
// TIM5  ch1.

// OK. drain voltage isn't changing. need to check fet.
// also polarity of buzzer.

static void buzzer_setup(void )
{
  // HMMMMM...

  usart_printf("buzzer setup\n");

  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO0 );
  gpio_set_af(GPIOA, GPIO_AF2, GPIO0 ); // AF1 == timer.
  gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, GPIO0 ); // 50is faster than 100? no. same speed



  rcc_periph_reset_pulse(RST_TIM5);   // is this needed

  timer_set_prescaler(TIM5, 20);  // 1MHz.

  timer_set_mode(TIM5, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_CENTER_1, TIM_CR1_DIR_UP);
  timer_set_oc_mode(TIM5, TIM_OC1, TIM_OCM_PWM2);
  timer_enable_oc_output(TIM5, TIM_OC1);
  timer_enable_break_main_output(TIM5);
  timer_set_oc_value(TIM5, TIM_OC1, 500);
  timer_set_period(TIM5, 1000);
  // timer_enable_counter(TIM5);
}


static void buzzer_enable(void )
{
  timer_enable_counter(TIM5);
}

static void buzzer_disable(void )
{
  timer_disable_counter(TIM5);
}

/////////////////////////////



void sys_tick_handler(void)
{
  /*
    this is an interupt. not sure how much work should do here.
    albeit maybe its ok as dispatch point.

    it will interrupt other stuff...

    should be flipping a flag, so can handle in the main update loop()
  */
  // equivalent to a rtos software timer
  // we are in an interupt  context here... so don't do anything
  // NOTE. we could actually set a flag.  or a vector of functions to call..
  // and then process in main loop.
  // eg. where things can be properly sequenced

  system_millis++;


  // 100ms.
  if( system_millis % 100 == 0) {
  }


  // 500ms.
  if( system_millis % 500 == 0) {
    // blink led
    gpio_toggle(LED_PORT, LED_OUT);
  }

}


static void loop( Context *ctx)
{
  static uint32_t buzzer_stop_millis = 0;

  //static uint16_t last_sw = 0;

  static int last_rotary_count = 0;

  while(true) {

    // EXTREME - can actually call update at any time, in a yield()...
    // so long as we wrap calls with a mechanism to avoid stack reentrancy
    // led_update(); in systick.

    // stop buzzer?
    if(system_millis > buzzer_stop_millis)    // wrap around
      buzzer_disable();

    /*
    // tactile switch
    uint16_t sw = gpio_get( TACTILE_SW_PORT, TACTILE_SW1_IN | TACTILE_SW2_IN) ;
    if(sw != last_sw) {
      last_sw = sw;
      usart_printf("sw1 or sw2 changed .. %d\n", sw);
    }
    */

    // print rotary encoder
    // IMPORTANT - should be able to sign extend
    // to get negative values... get_counter returns 16 or 32 bit?
    int count = timer_get_counter(TIM1);
    if(count != last_rotary_count) {
      last_rotary_count = count ;
      usart_printf("rotary count.. %d\n", count);

      // set buzzer
      buzzer_enable();
      buzzer_stop_millis = system_millis + 20;
    }

    // pump usart queues
    // usart_input_update();
    usart_output_update();

    /////////////////////
    setTextColor(ctx, ILI9341_BLUE);
    setCursor(ctx, 50, 50);
    setTextSize(ctx, 5, 5);

    char buf[100];
    snprintf( 100, buf, "%d", (int)system_millis );
    // ok. this will actually wrap correctly...
    // drawText(ctx, "hi there friends all  ");
    drawText(ctx, buf ); //"hi there friends all  ");


    /////////////////////

  }

}



void agg_test(void);

int main(void)
{
  // high speed internal!!!
  // this appears to work... 1Hz clock goes to 5Hz.
  // although should be able to go to 100MHz.
  // rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_3V3_84MHZ ]);

  clock_setup(16000);



  rcc_periph_clock_enable(RCC_SYSCFG); // maybe required for external interupts?


  // LED
  rcc_periph_clock_enable(RCC_GPIOA);

  // tactile switches
  rcc_periph_clock_enable(RCC_GPIOB);

  // TFT
  rcc_periph_clock_enable(RCC_SPI1);

  // USART
  // rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOB); // F410
  rcc_periph_clock_enable(RCC_USART1);


  cBufInit(&console_in, buf1, sizeof(buf1));
  cBufInit(&console_out, buf2, sizeof(buf2));

  /////////////////
  // tft
  led_setup();

  // TODO move these up, under usart.
  usart_setup_gpio_portB();
  usart_setup(&console_in, &console_out);

  ///////////////////////
  // rotary
  rcc_periph_clock_enable(RCC_TIM1);

  initRotaryEncoderTimer(TIM1, GPIOA, GPIO8, GPIO_AF1, GPIOA, GPIO9, GPIO_AF1) ;

  ///////////////////////
  // rotary sw
  rotary_sw_exti_setup();

  // tactile sw
  tactile_sw_exti_setup();


  ///////////////////////
  // buzzer

  rcc_periph_clock_enable(RCC_TIM5);
  buzzer_setup();

  ////////////////////


  usart_printf("\n--------\n");
  usart_printf("starting\n");
  // usart_printf("size %d\n", sizeof(fbuf) / sizeof(float));


  // lcd...
  usart_printf("doing lcd_spi_setup()\n");
  lcd_spi_setup();


  /////////////////////////////
  Context   ctx;

  // low level
  initialize(&ctx);

  ILI9341_setRotation(&ctx, 3); // 0 == trhs, 1 == brhs, 2 == blhs,  3 == tlhs

  usart_printf("doing lcd_do_stuff()\n");
  lcd_do_stuff( &ctx);

   // agg
  // usart_printf("agg test\n");
  // agg_test();

 

  loop( &ctx);

	for (;;);
	return 0;
}




