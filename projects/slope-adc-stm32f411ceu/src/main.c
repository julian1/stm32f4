/*
  serial,
  rlwrap -a picocom -b 115200 /dev/ttyUSB0

  screen
  openocd -f openocd.cfg
  rlwrap nc localhost 4444  # in new window

   reset halt ; flash write_image erase unlock ../blinky-stm32f410cbt3/main.elf; sleep 1500; reset run

  *********
  with gnu sprintf, and floating point code, this still fits in 27k, tested by editing f410.ld.  good!!
  *********

*/


#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

// #include <libopencm3/cm3/nvic.h>
// #include <libopencm3/cm3/systick.h>

#include <libopencm3/stm32/spi.h>   // SPI1


#include <stddef.h> // size_t
//#include <math.h> // nanf
//#include <stdio.h>
#include <string.h>   // memset
#include <assert.h>





#include "cbuffer.h"
#include "usart.h"
#include "streams.h"
#include "util.h"
// #include "assert.h"
#include "fbuffer.h"
#include "stats.h"
#include "format.h" // format_bits

#include "spi1.h"
#include "ice40.h"


#include <matrix.h>
#include "regression.h"


#include "app.h"



static void loop_dispatcher(app_t *app);

/*
  we want the parameters for
    ordinary running . to be same  as cal.
    but also to be able to vary.
    -------

  point is that params for calibration and for running want to be the same.
  OR. we want to run with permutations. but the same counts should work.

  so we just need an extra set of parameters.

  YAGNI.

    - just try use app_t data structure instead of something separate.
    - that means putting b. in app.

  permutate.

  mux hi
  mux lo/com
  mux sig

  cal       - using current values
  reset cal  - cal using default etc.
  set fix
  set var.    etc.

  reset default.   eg. same as programmed.

  its all

*/

/*
  so the main need - is to be able to switch programs/tests from commands.
  then can prototype more easily.
*/


void update_console_cmd(app_t *app)
{


  while( ! cBufisEmpty(&app->console_in)) {

    // got a character
    int32_t ch = cBufPop(&app->console_in);
    assert(ch >= 0);

    if(ch != '\r' && app->cmd_buf_i < CMD_BUF_SZ - 1) {
      // character other than newline
      // push onto a vector? or array?
      app->cmd_buf[ app->cmd_buf_i++ ] = ch;

      // echo to output. required for minicom.
      putchar( ch);

    }  else {
      // we got a command

      app->cmd_buf[ app->cmd_buf_i ]  = 0;

      putchar('\n');
      // usart_printf("got command '%s'\n", app->cmd_buf );

      // flash write
      if(strcmp(app->cmd_buf , "flash write") == 0) {
        // flash_write();
      }
      // flash read
      else if(strcmp(app->cmd_buf , "flash read") == 0) {
        // flash_read();
      }

      /*
        - OK. it is extremely confusing that we have to call write after doing this.
        spent 20 mins trying to figure it out.

      */
      /*
        We need to add azero. to get a proper sense .
          - subtract the azero term. or subtract a moving average.

      */

      else if(strcmp(app->cmd_buf , "mux ref-lo") == 0 || strcmp(app->cmd_buf , "mux com") == 0)  {

        usart_printf("make sure to write value!\n");
        app->params.himux_sel = HIMUX_SEL_REF_LO;
      }// ref-lo/com...
      else if(strcmp(app->cmd_buf , "mux ref-hi") == 0) {
        usart_printf("make sure to write value!\n");
        app->params.himux_sel = HIMUX_SEL_REF_HI;
      }
     else if(strcmp(app->cmd_buf , "mux sig\n") == 0) {
        usart_printf("make sure to write value!");
        app->params.himux_sel = HIMUX_SEL_SIG_HI;
      }

      // we may want to be able to read/store multiple calibrations. eg. array.
      // but this is sufficient for the moment.

      /*
        - calibration b -  is mostly going to be invariant.
        - basic steps.
            - on bootup. - read the device characteristics.   or load from flash.
            - can then run.  or calibrate.

      */

      else if(strcmp(app->cmd_buf , "show") == 0) {
        // report params.
        params_report( &app->params);
      }

      // think we need to rename thesei. distinct from the flash operations.

      else if(strcmp(app->cmd_buf , "read") == 0) {
        // read params from the device. actually a reset to default.
        params_read( &app->params );
      }
      else if(strcmp(app->cmd_buf , "write") == 0) {
        // write params to device
        // should we do this on every value change?
        params_write( &app->params );
      }
      else if(strcmp(app->cmd_buf , "exit") == 0 || strcmp(app->cmd_buf , "halt") == 0) {
        // exit the current loop program
        app->continuation_f = (void (*)(void *)) loop_dispatcher;
        app->continuation_ctx = app;
      }
      else if(strcmp(app->cmd_buf , "loop1") == 0) {
        // start loop1.
        app->continuation_f = (void (*)(void *)) loop1;
        app->continuation_ctx = app;
      }
      else if(strcmp(app->cmd_buf , "loop2") == 0) {
        app->continuation_f = (void (*)(void *)) loop2;
        app->continuation_ctx = app;
      }

      // unknown command
      else {
        printf( "unknown command '%s'\n", app->cmd_buf );
      }

      // reset buffer
      app->cmd_buf_i = 0;
      app->cmd_buf[ app->cmd_buf_i ]  = 0;

      // issue new command prompt
      usart_printf("> ");

    }
  }
}




/*
  after a 5 seecond integration at 20MHz. quite good.
    - disconnecting scope leads, meter leads. helps.
    - can turn off the leds.
    - can provide a space to read values. before start next integration
    - can short the integrator
    - noise ...

  - IMPORTANT. 5/1 gain. good for same speed ops. but is useful way to trim the output range without adjusting current or integrator cap.

  - very preliminary tests - using same high-current for rundown are encouraging.
      takes some 30sec to settle - DA or TC influence somewhere, that need to investigate.

    - strengths
      - 20MHz canned cmos oscillator
      - 74hc175 syncronization and 74hc4053 switch (nexperia). - 5V supplies derived from ref voltage/ for consistent discrete cmos voltage levels.
      - lt1016 10ns comparator. datasheet says has GHz GBW. running from +-5V supplies.
      - opa2140 compound gain integrator. with 10k/2k divider.
      - lt1358 slope amp. 25MHz/600V/uS. no gain, just diode range limit. use comparator gain instead.
      - 2x lt5400 10k for ladder, and current.
      - lm399 and opa2777 for ref, current source.
      - separate agnd/dgnd/int current gnd.
      - stm32/adum/ice40 for isolation/control.

    - limitation/weaknesses - used for initial tests
      - sprayed isopropul around comparator integrator/ and no longer stable. hmmm
      - 30sec to settle, could just be the ref.
      - mlcc 100nF cap. for itnegrator.
      - slow 2kHz waveform.
      - not using slow rundown current - eg. stretch integration to 5 sec.
      - no initial reset/shorting of integrator at start. instead start after prior integration after zero-cross.
      - high current 1.3mA due to ltc5400 10k resistors. resistor heating.
      - compound integrator - divider 10k/2k. susumu rr.   TC. not as good as could be.
      - need comparator latch code. to avoid some output bouncing
      - spi reading values at end/start coincides with actual integration period. want pause.
      - plc not multiple of mains freq. albeit 5sec. is.
      - soic 4053 choice. limiting not ad633.
*/






// initialize. first read. then overwrite. then write again.












static void loop_dispatcher(app_t *app)
{
  usart_printf("=========\n");
  usart_printf("continuation dispatcher\n");
  usart_printf("> ");

 static uint32_t soft_500ms = 0;

  while(true) {

    update_console_cmd(app);

    // 500ms soft timer. should handle wrap around
    if( (system_millis - soft_500ms) > 500) {
      soft_500ms += 500;

      //
      led_toggle();
    }

    if(app->continuation_f) {
      printf("jump to continuation\n");
      void (*tmppf)(void *) = app->continuation_f;
      app->continuation_f = NULL;
      tmppf( app->continuation_ctx );

      printf("continuation done\n");
      usart_printf("> ");
    }

  }
}





static void spi1_interupt(app_t *app )
{
  UNUSED(app);
  app->data_ready = true;

}

static char buf_console_in[1000];
static char buf_console_out[1000];

// static float buf_rundown[6];

static app_t app;




static void reg_read_write_test(void)
{
  // test ice40 register read/write
  // ok. seems to work.
  uint32_t ret;


  /*
  OK. i think these spi calls may fail when speed of design falls below 32MHz.
  because

  IMPORTANT.
  - Also. removing reg_led from the verilog initial block.
  indicating timing conflict.
  and values are correct.
  - but this could have just been due to timing.
  - but no longer seems to have affect. now that speed is better.
  */

  spi_reg_write(SPI1, REG_LED , 0xff00ff);
  msleep(1);
  ret = spi_reg_read(SPI1, REG_LED);
  usart_printf("ret is %x\n", ret);
  // ret value is completely wrong....
  assert(ret == 0xff00ff);

  // this works... eg. allowing high bit to be off.
  spi_reg_write(SPI1, REG_LED, 0x7f00ff);
  ret = spi_reg_read(SPI1, REG_LED);
  assert(ret == 0x7f00ff);

  for(uint32_t i = 0; i < 32; ++i) {
    spi_reg_write(SPI1, REG_LED , i );
    ret = spi_reg_read(SPI1, REG_LED);
    assert(ret == i );
  }

}


int main(void)
{
  // hsi setup high speed internal!!!
  // TODO. not using.

  rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ] );  // stm32f411  upto 100MHz.


  // clocks
  rcc_periph_clock_enable(RCC_SYSCFG); // maybe required for external interupts?

  // LED
  rcc_periph_clock_enable(RCC_GPIOA); // f410 led.

  // USART
  // rcc_periph_clock_enable(RCC_GPIOA);     // f407
  rcc_periph_clock_enable(RCC_GPIOB); // F410
  rcc_periph_clock_enable(RCC_USART1);

  // spi / ice40
  rcc_periph_clock_enable(RCC_SPI1);



  //////////////////////
  // setup

  // 16MHz. from hsi datasheet.
  // systick_setup(16000);
  systick_setup(84000);

  // led
  led_setup();


  memset(&app, 0, sizeof(app_t));

  ///////
  // uart/console
  cBufInit(&app.console_in,  buf_console_in, sizeof(buf_console_in));
  cBufInit(&app.console_out, buf_console_out, sizeof(buf_console_out));


  // buffer of measurements.
  // fBufInit(&app.measure_rundown, buf_rundown, ARRAY_SIZE(buf_rundown));


  //////////////
  // uart
  // usart_setup_gpio_portA();
  usart_setup_gpio_portB();

  usart_set_buffers(&app.console_in, &app.console_out);

  // standard streams for printf, fprintf, putc.
  init_std_streams(  &app.console_out );

  printf("hi\n");
  // assert( 0);


  ////////////////
  // spi1/ice40
  spi1_port_cs1_setup();

  spi_ice40_setup(SPI1);

  // adc interupt...
  spi1_interupt_gpio_setup( (void (*) (void *))spi1_interupt, &app);




  usart_printf("\n--------\n");
  usart_printf("starting loop\n");
  usart_printf("sizeof bool   %u\n", sizeof(bool));
  usart_printf("sizeof float  %u\n", sizeof(float));
  usart_printf("sizeof double %u\n", sizeof(double));

  // ASSERT(1 == 2);

  usart_printf("a float formatted %g\n", 123.456f );


  usart_printf("\n--------\n");
  usart_printf("addr main() %p\n", main );

  usart_flush();


  // test device read/write
  reg_read_write_test();

  // read main params from device - as starting point. should perhaps be flash
  params_read( &app.params );

  printf("==========\n");



#if 0
  ////////////////////////////////////
  MAT *b = calibrate( &app );
  permute(&app, b);
#endif

  // loop1(&app, b );

  loop_dispatcher( &app);

}






#if 0
void update_console_cmd(app_t *app)
{

  /* using peekLast() like this wont work
     since it could miss a character.
    we kind of need to transfer all chars to another buffer. and test for '\n'.
    -----
    No. the easiest way is to handle the interupt character. directly...
    actually no. better to handle in main loop..

  */


  while( ! cBufisEmpty(&app->console_in)) {

    // got a character
    int32_t ch = cBufPop(&app->console_in);
    assert(ch >= 0);

    if(ch != '\r' && app->cmd_buf_i < CMD_BUF_SZ - 1) {


      // push_char(app->cmd_buf, &app->cmd_buf_i, ch );

      // push onto a vector? or array?
      app->cmd_buf[ app->cmd_buf_i++ ] = ch;
      // app->cmd_buf[ app->cmd_buf_i ] = 0;

    }  else {
      // we got a command

      app->cmd_buf[ app->cmd_buf_i ]  = 0;

      usart_printf("got command '%s'\n", app->cmd_buf );


      if(strcmp(app->cmd_buf , "whoot") == 0) {
        // So.  how do we handle changing modes????

        // if we are in separate loops for calibration, permutation , etc.
        // how do we cancel, break out. and start another?
        // coroutines. not really an answer.

        // this function can be tested and be used to return early.
        // or set a flag. like cancel current command/action.

        // also - sometimes we want to change something - without setting the continuation.

        app->continuation_ctx = 0;

      }
      app->cmd_buf_i = 0;
    }
  }
}
#endif








