/*

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


// #include <setjmp.h>
#include <stddef.h> // size_t
//#include <math.h> // nanf
//#include <stdio.h>
// #include <string.h>   // memset
#include <malloc.h> // malloc_stats

// stl variant
#include <variant>


#include "cbuffer.h"
#include "usart.h"
#include "util.h"
#include "streams.h"
#include "cstring.h"

#include "assert.h"
#include "cdcacm.h"


// #include "str.h"  //format_bits

#include "fsmc.h"
#include "ssd1963.h"
#include "xpt2046.h"
#include "rotary.h"
#include "ui_events.h"
#include "curses.h"



#include "value.h"
#include "menu.h"



// put prototypes here to avoid pulling in template c++ headers in c code.

extern "C"
{

int agg_test2( void );
int agg_test3( void );
int agg_test4( void );
int agg_test5( void );
int agg_test6( void );

// extern "C" int agg_test8( void );
}


extern "C" int agg_test7( Curses & a, int arg);

extern int agg_test8(  Curses &a );

// if we want to pass buffers to things...
// Gahhh.
// c initialization, is so different from c++ initialization.
// eg. if want to pass CBuf to jjjjjjj



// #define CMD_BUF_SZ  100



typedef struct app_t
{

  /*
    OK. we want to initialize the console buffers and uart before here
  */


  app_t(
    CBuf & console_in,
    CBuf & console_out,
    CString  &  command,

    usbd_device *usbd_dev,

    Curses & curses, MenuController & menu_controller, Menu & menu,
    Curses & curses2,
    CBuf & ui_events_in
    )
     :
      console_in (console_in),
      console_out (console_out),
      command(command),

      usbd_dev( usbd_dev ),

      curses( curses),
      menu_controller ( menu_controller), menu( menu ),

      curses2( curses2 ),

      ui_events_in(ui_events_in)
  {

  }


  // change name CBuf to CircBuf ... it's more meaningful. C implies class
  // No. c is for character.
  CBuf        & console_in;
  CBuf        & console_out;
  CString     & command;   // initialization?

  unsigned  program;  // test program to run
  unsigned  program_arg;      // argument to programrun

  usbd_device *usbd_dev ;


  ////////

  Curses        & curses;
  MenuController & menu_controller ;
  Menu            & menu;


  Curses        & curses2;

  CBuf          & ui_events_in;

} app_t;





static void update_console_cmd(app_t *app)
{


  while( ! cBufisEmpty(&app->console_in)) {

    // got a character
    int32_t ch = cBufPop(&app->console_in);
    assert(ch >= 0);

    if(ch != '\r' && cStringCount(&app->command) < cStringReserve(&app->command) ) {
      // normal character
      cStringPush(&app->command, ch);
      // echo to output. required for minicom.
      putchar( ch);

    }  else {

      // newline or overflow
      putchar('\n');

      char *cmd = cStringPtr(&app->command);

      printf("cmd is '%s'\n", cmd);

      if(strncmp(cmd , "test", 4) == 0) {

        unsigned x0 = 0;
        unsigned x1 = 0;
        unsigned n = sscanf(cmd + 4,  "%u%u", &x0, &x1);
        if(n == 1) {
          printf("got test %u\n", x0);
          app->program = x0;
          app->program_arg = 0;
        }  else if (n == 2) {
          printf("got test %u %u\n", x0, x1);
          app->program = x0;
          app->program_arg = x1;
        }
        else {
          printf("badly formatted\n");
        }
      }
      else if(strcmp(cmd , "write") == 0) {
        // flash write
        printf("got write - using cString\n");
      }
      else if(strcmp(cmd, "read") == 0) {
        // flash read
        printf("got read\n");
      }
      else {
        // unknown command

        printf( "unknown command '%s'\n", cmd );
      }

      // reset buffer
      cStringClear( &app->command);

      // issue new command prompt
      usart_printf("> ");
    }
  }
}










static void update_ui_events_in(app_t *app)
{


  while(!cBufisEmpty(&app->ui_events_in)) {

    // want to keep consuming rotary inputs until get the most recent.


    int event = cBufPop(&app->ui_events_in) ;

    // consume successive rotary inputs.
    if(event == ui_events_rotary_change
      && !cBufisEmpty(&app->ui_events_in)
      && cBufPeekFirst(&app->ui_events_in) == ui_events_rotary_change)
      continue;

    // usart_printf("main got ui event %d\n", event );

    app->menu_controller.event( event );

  }
}



/*
  rename to lcd_double_buffer_render().
  and have the alternative strategy easily switchable.
*/

static void lcd_render( Curses & curses, Curses & curses2)
{
  /*
    - maybe change name to agg_render
      very high level function. because coordinates the paging.

    -  funny that the lcd does not have a context variable.
        instead its talking to single fsmc controller.
  */

  // persist the page that we need to draw
  static int page = 0; // page to use
  page = ! page;

  // set up our buffer
  pixfmt_t  pixf(  page *  272 );
  rb_t    rb(pixf);

  // rb.clear(agg::rgba(1,1,1));     // bg white .
  rb.clear(agg::rgba(0,0,0));       // bg black

  // uint32_t start = system_millis;
  // ok, this works. so maybe there is memory corruption somewhere.

  ////////////////////////////////////
  int blink = (system_millis / 500) % 2;
  // usart_printf("blink %u\n", blink );

  render( curses , rb,  blink );
  render( curses2 , rb,  blink );

  // lcd synchronization, wait until not in vertical blanking mode
  while( getTear() ) {
    // usart_printf("tear hi\n" );
  };

  // flip the newly drawn page in
  setScrollStart( page *  272 );

}




static void loop(app_t *app)  // TODO change to a c++ reference
{

  usart_printf("=========\n");
  usart_printf("loop\n");
  usart_printf("> ");


  // TODO move to app_t structure?.
  static uint32_t soft_500ms = 0;

  while(true) {

		usbd_poll(app->usbd_dev);

    update_console_cmd(app);

    update_ui_events_in(app);

    // usart_output_update(); // shouldn't be necessary


    // 500ms soft timer. should handle wrap around
    if( (system_millis - soft_500ms) > 500) {
      soft_500ms += 500;
      led_toggle();
      // usart_printf("here\n");
      // LCD_Read_DDB();


      // int count = timer_get_counter(TIM1);
      // usart_printf("timer count %u\n", count);


    }


    // curses2.render( Curses &a, rb_t &rb, bool blink );

    // render( curses2, rb_t &rb, bool blink );

    // do the curses menu draw
    // app->menu.draw();
    app->menu.draw( app->curses  ); // TODO should pass the curses here....

    lcd_render( app->curses, app->curses2  );




#if 0
  // memory issues somewhere
    switch(app->program)
    {

      case 2: agg_test2(); break;
      case 3: agg_test3(); break;
      case 4: agg_test4(); break;   // ok

      case 5: agg_test5(); break;   // ok.
      case 6: agg_test6(); break; // ok.
      case 7: agg_test7( app->curses, app->program_arg ); break;  // just linking breaks stuff. because of the size of static Curses  structure
      case 8: agg_test8( app->curses ); break;  // ok
      case 9: app->menu_controller.draw(); break;

      // todo get working.
      // case 10: draw_test1(app->curses );

      // default:
      //  printf("unrecognized test\n");
    }
#endif


    // xpt2046_read();

  }
}


// just put all this on the stack.

static char buf_console_in[1000];
static char buf_console_out[2000]; // setting to 10000. and it fails??? werid.


static char buf_ui_events_in [100];


static char buf_command[100];



// TODO should be initialized, instantiated in main. on stack.
// in order for c++ constructors. to be called



/*
  We need. feb 2022.
    - agg tests should be separate loops. so we can switch at runtime with continuation functions.
    - CVec structure. for buffers.
    - menu structure - gui drawing.    SHOULD consider.
        - only using an effect for the focus cursor. this means don't have to have if(focus) drawthis() else drwathat()
        - should be able to separate out drawing. I think.

      - menu should be high level structure. that FSM operates over.


*/



/*
  Ok, vertical is ok.   but we want to flip the horizontal origin.
  to draw from top left. that shoudl be good for fillRect, and for agg letter.
*/

int main(int arg0)
{

  // required for usb
	// rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ] );  // stm32f411  upto 100MHz. works stm32f407 too.
	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ] );  // stm32f407



  //////////////////////



  /*
  // http://libopencm3.org/docs/latest/stm32f4/html/f4_2rcc_8h.html

    see here for rcc.c example, defining 100MHz, using 25MHz xtal.
      https://github.com/insane-adding-machines/unicore-mx/blob/master/lib/stm32/f4/rcc.c
  */

  // clocks
  rcc_periph_clock_enable(RCC_SYSCFG); // maybe required for external interupts?

  rcc_periph_clock_enable(RCC_GPIOA); // rotary/buttongs
  rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_GPIOC); // buttons

  // USART
  // rcc_periph_clock_enable(RCC_GPIOA);
  // rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_USART1);


  // USB
	// rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_OTGFS);


  // TFT
  // SHOULD PUT ALL TFT stuff in header... or at least predeclare.
  // parallel tft / ssd1963
  rcc_periph_clock_enable(RCC_GPIOD);
  rcc_periph_clock_enable(RCC_GPIOE);
  // rcc_periph_clock_enable(RCC_GPIOB); // TEAR_PORT/TEAR gpio. on PB9.

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


  // ***********
  // IMPORTANT - we need to set up memory structures before interrupts get enabled, that might call functions.
  // that use those strucctures.
  // that suggests the app structure should be done first.

  ///////
  // uart/console
  // should be done in the constructor...

  // Might be better to initialize console_in and console out here.
  // and leave on stack.

  CBuf console_in;
  CBuf console_out;

  cBufInit(&console_in,  buf_console_in, sizeof(buf_console_in));
  cBufInit(&console_out, buf_console_out, sizeof(buf_console_out));


  //////////////
  // initialize usart before start all the app constructors, so that can print.
  // uart
  // usart_setup_gpio_portA();
  usart_setup_gpio_portB();

  usart_set_buffers(&console_in, &console_out);

  // standard streams for printf, fprintf, putc.
  init_std_streams( &console_out );


  usart_printf("\n--------\n");
  usart_printf("addr main() %p\n", main );

  // ram growing up.
  printf("arg0 %u \n", ((unsigned )(void *) &arg0 )  );
  printf("arg0 diff %uk\n", (((unsigned )(void *) &arg0 )  - 0x20000000 ) / 1024 );

  printf("-----------\n" );
  printf("malloc stats\n" );
  malloc_stats();


  // print_stack_pointer();


  ////////////////////////////
  // usb
  // might be better to pass as handler?

  // should be being done before or after app construction.
  // need buffers. up ...



  usbd_device *usbd_dev = usb_setup();
  assert(usbd_dev);


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

  rotary_setup_gpio_portA();
  rotary_init_timer( TIM1 );
  // rotary_setup_interupt();




  /////////////////////////////////


  // command buffer - should/could be done in app constructor I think.
  CString  command;

  cStringInit(&command, buf_command, buf_command + sizeof( buf_command));
  assert(cStringReserve(&command) == sizeof( buf_command));
  assert(cStringCount(&command) == 1); // null terminator


  Curses curses( 33, 17, 14, 16 );

  printf("sizeof(Curses) %u\n", sizeof(Curses) );

  /////////////

  Curses curses2( 12, 3, 43, 55 );
  clear( curses2 ); // eg. remove text. from last draw
  font(curses2, &arial_span_72 ); // font
  color_pair_idx(curses2, 2);
  to(curses2, 0, 1);
  text(curses2, "12.3456mV");   // arial72 - not being able to draw text.... and crashing is bad....



  int32_t    element_idx = 0; // first digit, need negative to support after float

  ElementController  element_controller(element_idx);

  DigitController digit_controller(element_idx );


  ListController  list_controller( digit_controller);

  PageController  page_controller( list_controller);

  // menucontroller is the controller controller
  // rename
  MenuController  menu_controller( page_controller, list_controller, element_controller, digit_controller);

  // draw. could probably just put the draw on the menu_controller.
  // but separate for separation of concerns.
  // else use function draw()..
  Menu menu( page_controller, list_controller, element_controller, digit_controller );       // MenuView

  // int x = 0b1001;

  char *name = "settings 1";
  char *keys[]     = { "whoot", "apple", "blue", "foo" } ;


  // arrays of functions perhaps heasier
  // not sure. if virtual functions would be easier

  double xx[]  = { 14.12, 256, 399.123 } ;

  bool result = false;

  Value  values[] = {
      Value( &xx[0], (edit_t) value_float_edit, (copy_t) value_float_copy, (format_t) value_float_format2, NULL),
      Value( &xx[1], (edit_t) value_float_edit, (copy_t) value_float_copy, (format_t) value_float_format, NULL) ,
      Value( &xx[2], (edit_t) value_float_edit, (copy_t) value_float_copy, (format_t) value_float_format, NULL),

      Value( &result, (edit_t) value_bool_edit, (copy_t) value_bool_copy, (format_t) value_bool_format, NULL)
  };


  assert( ARRAY_SIZE(keys) == ARRAY_SIZE(values) );
  Item item1( name, keys, values , ARRAY_SIZE(values) ); // ARRAY_SIZEsizeof(values

/*
  double values[]  = { 14.12, 256, 399.123 } ;

  Item item1( name, keys, values , 3 );


  char *name2 = "settings 2";
  char *keys2[]     = { "fred", "bananna", "green", "blue" } ;
  double values2[]  = { 9.12, 5, 123.456, 44 } ;

  Item item2( name2, keys2, values2, 4 ) ;
*/
  Item *items[] = { &item1 /*, &item2 */ } ;

  // populate the initial value.
  page_controller.set_value( items, 1 );


  // c structure stuff
  CBuf ui_events_in;

  // ui events
  cBufInit(&ui_events_in, buf_ui_events_in, sizeof(buf_ui_events_in));

  ui_events_init( &ui_events_in);



  app_t app( console_in,
            console_out,
            command,
            usbd_dev,
            curses, menu_controller, menu,
            curses2,
            ui_events_in
            ) ; // not sure that app needs curses.




  usart_printf("\n--------\n");
/*
  usart_printf("starting loop\n");
  usart_printf("sizeof bool   %u\n", sizeof(bool));
  usart_printf("sizeof float  %u\n", sizeof(float));
  usart_printf("sizeof double %u\n", sizeof(double));
*/

  // usart_printf("sizeof setjmp %u\n", sizeof(setjmp)); // 1.

  // test assert failure
  // assert(1 == 2);
  usart_printf("a float formatted %g\n", 123.456f );

  usart_flush();

  // set program to run
  app.program = 9;

  loop(&app);
}

// put the code after the function. to prevent inlining.




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



*/
