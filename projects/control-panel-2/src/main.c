/*
  same interface ... for drawing outlnie text, as span text.
    eg. point size   int(72 / 20.f ) .
    and without having to pass mtx.

    and return x position. so
    -------
    allow us to use interchangeably.

  ncurses/ vt100 terminal.
    - but with some characters. different font size.
    - ability to pass callback
    ----------
  ================
    - to manage
        x,y position (eg. can uniform space)uniform space)
        color
        focus glyph/invert.  blinking (easy).
        maybe veritical/horitzonal lines.
        symbols - can be embedded with codes/indexes like fonts. we have simple svg.
        symbols - can also pre-render span data.
        -----
        hit-testing - becomes easy. just use cursor positon for focus. and
          can then index - the actual text as well.  <- interesting.

        optimisation - avoiding drawing.
          - can test - if last char changes/ to avoid drawing.
          - yes. write into a buffer. then diff the buffer for change. to enable minimal change.
             eg. so every char - should get font-size, and char data.
          - but can still use proportional space fonts.

        - cursor mapping space - very good (
            - for optimizing/computing change delta,
            - for hittesting,
            - for font size / mapping for prominance.
            - for indent left, and indent right.

        write(x,y, "whoot", ).
        cursor(3,4)   not generic move.
        color(green )
        write(3,4, "whoot", ).   moves to pos 3,4, but then text is written with proportional spacing.

        Need same structure repeated - for old/new.  in order to do delta.

        would be good enough for something
          like this https://www.youtube.com/watch?v=SMK4kkf7jmM

        ---------------
        cursor coordinate scheme is a way to carve up space. font-size affects this. so font size is recorded.

        notional_x[ 100* 100 ];   <- note that we can map this how every we want. (eg. a big middle section with larger fonts - could be treated as extra rows at the bottom).
        notional_y[ 100* 100 ];
        fontsize[ 100 * 100 ] ;

        ----
        derived. eg. filled in by the actual drawing commands.
        colorindex[ 100 * 100 ] ;
        color[ 100 * 100 ] ;
        character[ 100 * 100 ] ;
        actual_x[ 100* 100 ];   // for the actual character position. for hittest. and focus. and delta.
        actual_y[ 100* 100 ];   // for the actual character position. for hittest. and focus. and delta.

        ---------------
        for drawing horizontal

        notational concept.  of cursor positioning.
        notational = starting text position. GOOD. first char aligned.

          must loop once to determine.
          eg. if row 3 has larger size font - then row 3 will affect all y positions.
          so must map

        int cursor_to_fontsize [ 100 * 100 ] ; <- use this to generate the cursor_to_screen

        100x100.
        stride== 100.
        int cursor_to_screen_x[ 100 * 100 ] ;  (x + y * stride)
        int cursor_to_screen_y[ 100 * 100 ] ;

        - simple commands are equally good.


        - need indent left/right.

        - pre-determine font size mapping - for the cursor space.  eg. 100x100.
          - so that when displaying text - we know the font size. so for any position.
          - EXTR.    actually  we might make character map the entire cursor space. for the font size.
          - also potentially with whether that space can have


        - function  - cursor mapping / for any cursor pos - need the font-size. and text position.
            to easily - draw characters.
            eg. draw text -  will use this.

        we probably want it,
          for showing large amounts of text numeric data/ regardless.
          for showing general flowing text.

      mcurses for microcontrollers,
        https://github.com/ChrisMicro/mcurses

  ================




  ---------
  simple menu system.
    - when draw something - should potentiallly also add a an element to a hit selection structure. fairly simple. then can easily search for bounds.
    - keep flat. page style.

    - keypad. for number entry.
        or.  select digit. and provide ability to change it.
        single keypad. o
        no. just write the set v & i. underneath. and then cursor. on it.




  ---------------
  - done - dummy text - to test draw time.
  - coroutines. done x86
  - doen - spi slave - test on other board
  - done - do storage for span data.

  ----
  xpt2046
    - got x signal.

  doing work in interups.
    - get tear signal from tft to draw/flip page buffer
    - have to get adc on 50Hz read. needs spi call.
    - need to do spi to check the rails.
    - cannot overlap spi reads/ in interupts / because share the same spi port/state.


  ----------
  - ok tear 1ms hi signal at 76.9Hz.   measured with scope on tear pin ssd1963 (bottom pin15 from rhs).  ssd1963 pin is unconenected on board.
  - works with non-paged agg_test2 , and paged agg_test3 examples.

  ---------

  - maybe tear doesn't work - because we are drrawing/paging faster, than the screen refresh rate
    ? try adding 100ms delay. again.
  - OR - else just use vsync and configure it as interupt.
  - OR - loop with 0x45 getscanline till it gets near the end (depends on page) then repage.
  - OR - don't use double buffering. just draw/undraw  as needed.
  --
  - OR use f429 with sdram, and tft driver. can draw into mcu own sdram memory albeit via 16 bit 8080 bus.

  TODO
    fix agg_test2   without the paging/ double buffering
      see if get tear signal.

  ------------
  spi slave receive is simple.
    eg. just poll/block.
    or interupt driven.
    or dma.
  https://deepbluembedded.com/how-to-receive-spi-with-stm32-dma-interrupt/#STM32_SPI_Slave_Receiver_Polling_Mode_8211_LAB

  probably the only difference in slave - is who drives clk, and nss.

  simple test - is just to try do the block/ receive.  then add the



  ------------
  nix-shell ~/devel/nixos-config/examples/arm.nix
  make

  serial,
  rlwrap -a picocom -b 115200 /dev/ttyUSB0

  usb
  screen /dev/ttyACM0 115200

  screen
  openocd -f openocd.cfg
  rlwrap nc localhost 4444  # in new window

  reset halt ; flash write_image erase unlock /home/me/devel/stm32/stm32f4/projects/control-panel-2/main.elf; sleep 1; reset run

  *********
  with gnu sprintf, and floating point code, this still fits in 27k, tested by editing f410.ld.  good!!
  *********
  -----------------------------
  one mcu or two.

  two two.
    control paenl just issue spi commands and return core structure.
    spi slave - just handle as interupt.
        eg. receive a byte, decide what to do. could be just using spi_xfer()

    - any function in core - can be exposed as spi command. if required.
    - just enable an interupt on CS. then in the isr - handle it.

    - nss pin is set as an interupt input.


*/


#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>

// #include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>

#include <libopencm3/stm32/timer.h> // rotary


#include <libopencm3/usb/usbd.h>


#include <setjmp.h>
#include <stddef.h> // size_t
//#include <math.h> // nanf
//#include <stdio.h>
#include <string.h>   // memset


#include "cbuffer.h"
#include "usart2.h"
#include "util.h"
#include "assert.h"
#include "cdcacm.h"


// #include "str.h"  //format_bits

#include "fsmc.h"
#include "ssd1963.h"
#include "xpt2046.h"
#include "rotary.h"


// put prototypes here to avoid pulling in template c++ headers in c code.
int agg_test2( void );
int agg_test3( void );
int agg_test4( void );
int agg_test5( void );
int agg_test6( void );
int agg_test7( void );




typedef struct app_t
{
  CBuf console_in;
  CBuf console_out;

  usbd_device *usbd_dev ;

} app_t;


static void update_console_cmd(app_t *app)
{

  if( !cBufisEmpty(&app->console_in) && cBufPeekLast(&app->console_in) == '\r') {

    // usart_printf("got CR\n");

    // we got a carriage return
    static char tmp[1000];

    size_t nn = cBufCount(&app->console_in);
    size_t n = cBufCopyString(&app->console_in, tmp, ARRAY_SIZE(tmp));
    ASSERT(n <= sizeof(tmp));
    ASSERT(tmp[n - 1] == 0);
    ASSERT( nn == n - 1);

    // chop off the CR to make easier to print
    ASSERT(((int) n) - 2 >= 0);
    tmp[n - 2] = 0;

    // TODO first char 'g' gets omitted/chopped here, why? CR handling?
    usart_printf("got command '%s'\n", tmp);

    // process_cmd(app, tmp);
  }
}







static void loop(app_t *app)
{
  /*
    loop() subsumes update()
  */

  // TODO move to app_t structure?.
  static uint32_t soft_500ms = 0;

  while(true) {


		usbd_poll(app->usbd_dev);

    update_console_cmd(app);

    // usart_output_update(); // shouldn't be necessary


    // 500ms soft timer. should handle wrap around
    if( (system_millis - soft_500ms) > 500) {
      soft_500ms += 500;
      led_toggle();
      // usart_printf("here\n");
      // LCD_Read_DDB();

      int count = timer_get_counter(TIM1);
      usart_printf("timer count %u\n", count);


    }

    // agg_test2();
    // agg_test3();
    // agg_test4();
    // agg_test5();
    // agg_test6();
    agg_test7();




    // xpt2046_read();

  }
}




static char buf_console_in[1000];
static char buf_console_out[2000]; // setting to 10000. and it fails??? werid.


static app_t app;









/*
  Ok, vertical is ok.   but we want to flip the horizontal origin.
  to draw from top left. that shoudl be good for fillRect, and for agg letter.
*/

int main(void)
{

  // required for usb
	// rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ] );  // stm32f411  upto 100MHz. works stm32f407 too.
	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ] );  // stm32f407

  /*
  // http://libopencm3.org/docs/latest/stm32f4/html/f4_2rcc_8h.html

    see here for rcc.c example, defining 100MHz, using 25MHz xtal.
      https://github.com/insane-adding-machines/unicore-mx/blob/master/lib/stm32/f4/rcc.c
  */

  // clocks
  rcc_periph_clock_enable(RCC_SYSCFG); // maybe required for external interupts?

  // LED
  rcc_periph_clock_enable(RCC_GPIOA); // rotary/buttongs
  rcc_periph_clock_enable(RCC_GPIOB);

  // USART
  // rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_USART1);


  // USB
	// rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_OTGFS);


  // TFT
  // SHOULD PUT ALL TFT stuff in header... or at least predeclare.
  // parallel tft / ssd1963
  rcc_periph_clock_enable(RCC_GPIOD);
  rcc_periph_clock_enable(RCC_GPIOE);
  rcc_periph_clock_enable(RCC_GPIOB); // TEAR_PORT/TEAR gpio. on PB9.

  rcc_periph_clock_enable(RCC_FSMC);


  // spi / ice40
  // rcc_periph_clock_enable(RCC_SPI1);

  // xpt2046
  rcc_periph_clock_enable(RCC_SPI2);


  //////////////////////
  // setup


  // 16MHz. from hsi datasheet.
  // systick_setup(16000);
  // systick_setup(16000);
  // systick_setup(84000);  // 84MHz.
  systick_setup(168000);


  // led
  led_setup();


  memset(&app, 0, sizeof(app_t));

  // uart/console
  cBufInit(&app.console_in,  buf_console_in, sizeof(buf_console_in));
  cBufInit(&app.console_out, buf_console_out, sizeof(buf_console_out));

  // usart_setup_gpio_portA();
  usart_setup_gpio_portB();

  usart_set_buffers(&app.console_in, &app.console_out);

  // setup print
  // usart_printf_set_buffer()
  usart_printf_init(&app.console_out);


  ////////////////////////////
  // usb
  // might be better to pass as handler?
	app.usbd_dev = usb_setup();
  ASSERT(app.usbd_dev);




  fsmc_gpio_setup();

  fsmc_setup(1);
  tft_reset();

  LCD_Init();
  LCD_SetTearOn();
  // LCD_TestFill();


  xpt2046_gpio_setup();
  xpt2046_spi_port_setup();
  xpt2046_spi_setup( XPT2046_SPI );

  xpt2046_reset( XPT2046_SPI);


  // tim1
  rcc_periph_clock_enable(RCC_TIM1);
  initRotaryEncoderTimer(TIM1, GPIOA, GPIO8, GPIO_AF1, GPIOA, GPIO9, GPIO_AF1) ;



  usart_printf("\n--------");
  usart_printf("\nstarting\n");



  usart_printf("\n--------\n");
  usart_printf("starting loop\n");
  usart_printf("sizeof bool   %u\n", sizeof(bool));
  usart_printf("sizeof float  %u\n", sizeof(float));
  usart_printf("sizeof double %u\n", sizeof(double));

  usart_printf("sizeof setjmp %u\n", sizeof(setjmp)); // 1.

  // test assert failure
  // ASSERT(1 == 2);
  usart_printf("a float formatted %g\n", 123.456f );


  loop(&app);
}

// put the code after the function. to prevent inlining.



