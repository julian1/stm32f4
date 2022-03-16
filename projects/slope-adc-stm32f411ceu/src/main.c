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


#include <libopencm3/stm32/flash.h>


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
#include "voltage-source.h"

// bad conflicts with lib2/include/flash.h
#include "flash.h"
#include "config.h"


#include <matrix.h>
#include "regression.h"


#include "app.h"











static void loop_dispatcher(app_t *app);




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
  ret = spi_reg_read(SPI1, REG_LED);
  usart_printf("ret is %x\n", ret); // value is completely wrong.

  spi_reg_write(SPI1, REG_LED , 0xff00ff);
  msleep(1);
  ret = spi_reg_read(SPI1, REG_LED);
  usart_printf("ret is %x\n", ret); // value is completely wrong.
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



void update_console_cmd(app_t *app)
{

  uint32_t u32;
  int32_t i32;
  double d;

  while( ! cBufisEmpty(&app->console_in)) {

    // got a character
    int32_t ch = cBufPop(&app->console_in);
    assert(ch >= 0);

    if(! ( ch == ';' || ch == '\r')) {
      // character other than newline
      // push onto a vector? or array?

      if( app->cmd_buf_i < CMD_BUF_SZ - 1 )
        app->cmd_buf[ app->cmd_buf_i++ ] = ch;

      // echo to output. required for minicom.
      putchar( ch);

    }  else {

      // code should be CString. but this kind of works well enough...
      // we got a command
      app->cmd_buf[ app->cmd_buf_i ]  = 0;

      putchar('\n');
      // usart_printf("got command '%s'\n", app->cmd_buf );


      if(strcmp(app->cmd_buf , "test") == 0) {

        /* not haulting mcu, if fpga is not powered up, runnng is useful to test mcu code
          test device read/write
        */
        reg_read_write_test();
      }



      // flash write
      else if(strcmp(app->cmd_buf , "flash erase") == 0) {

          usart_printf("flash erasing sector\n");
          usart_flush();

          flash_erase_sector_();

          usart_printf("done erase\n");

      }


      // flash write
      else if(strcmp(app->cmd_buf , "flash write") == 0) {

        if(false && !app->b) {

          printf("no cal to save\n");
        } else {

          // put in a command
          usart_printf("flash unlock\n");
          flash_unlock();

          MAT *m = m_get(10, 2);

          FILE *f = open_flash_file();

          c_skip_to_last( f);

          m_write_flash ( m, f );
          fclose(f);

          // flash_program(FLASH_SECT_ADDR, buf, sizeof(buf) );

          usart_printf("flash lock\n");
          flash_lock();
          usart_printf("done\n");
        }
      }


      // flash read
      else if(strcmp(app->cmd_buf , "flash read") == 0) {


        usart_printf("flash reading \n");

        FILE *f = open_flash_file();



        MAT *u  = m_read_flash( MNULL, f );
        m_foutput( stdout, u );
        usart_flush();

        /* OK. ftell after a read is not correct because of intermediate buffering, by libc
          but it should be correct after a fseek() and should be correct after a write operation fwrite, fput etc. 
        */

        printf("ftell  %ld\n", ftell( f)  );


        // printf("****seek beginning \n" );
        fseek( f, 0 , SEEK_SET ) ;   // should be at start now


        MAT *uu  = m_read_flash( MNULL, f );
        m_foutput( stdout, uu );
        usart_flush();

        fclose(f);


        // flash_read();
      }



      else if(sscanf(app->cmd_buf, "vs %ld", &i32 ) == 1) {
        printf("setting value for voltage source %ld!\n", i32);
        voltage_source_set( i32 );
      }


      else if(strcmp(app->cmd_buf , "mux ref-lo") == 0 )  {
        ctrl_reset_enable();
        ctrl_set_mux( HIMUX_SEL_REF_LO );
        ctrl_reset_disable();
      }
      else if(strcmp(app->cmd_buf , "mux ref-hi") == 0) {
        ctrl_reset_enable();
        ctrl_set_mux( HIMUX_SEL_REF_HI );
        ctrl_reset_disable();
      }
      else if(strcmp(app->cmd_buf , "mux sig") == 0) {
        ctrl_reset_enable();
        ctrl_set_mux( HIMUX_SEL_SIG_HI );
        ctrl_reset_disable();
      }
    /*
      else if(strcmp(app->cmd_buf , "mux ang") == 0) {
        ctrl_reset_enable();
        ctrl_set_mux( HIMUX_SEL_ANG);
        ctrl_reset_disable();
      }
    */



/*
      else if(sscanf(app->cmd_buf, "pattern %lu", &u32 ) == 1) {

        printf("setting pattern %lu\n", u32);

        ctrl_reset_enable();
        ctrl_set_pattern( u32 );
        ctrl_reset_disable();

        /////////////////
        // when we set variables - we should also set the interupt handler - to determine if should update for next run
        /////////////////
      }
*/


      else if(sscanf(app->cmd_buf, "reset_test %lu", &u32 ) == 1) {

        printf("reset test %lu seconds\n", u32 );

        // TODO change name ctrl_reset_enable()
        ctrl_reset_enable();
        msleep( u32 * 1000);
        ctrl_reset_disable();
      }


      // should read as a double
      // actually an aperture setting.

      else if(sscanf(app->cmd_buf, "nplc %lf", &d ) == 1) {

        printf("setting nplc %lf\n", d );

        uint32_t aper = nplc_to_aper_n( d );

        ctrl_reset_enable();
        ctrl_set_aperture( aper );
        ctrl_reset_disable();
      }

#if 0
      // else if(strncmp(app->cmd_buf , "clk_count_var_pos_n", 19) == 0) {
      else if(sscanf(app->cmd_buf, "clk_count_var_pos_n %lu", &u32 ) == 1) {

          printf("setting clk_count_var_pos_n %lu\n", u32);

      }
#endif

      else if(strcmp(app->cmd_buf , "h") == 0 || strcmp(app->cmd_buf , "halt") == 0) {
        // exit the current loop program
        app->continuation_f = (void (*)(void *)) loop_dispatcher;
        app->continuation_ctx = app;
      }
      else if(strcmp(app->cmd_buf , "loop1") == 0) {
        // start loop1.
        app->continuation_f = (void (*)(void *)) loop1;
        app->continuation_ctx = app;
      }
      else if(strcmp(app->cmd_buf , "loop2") == 0) {  // cal loop.
        app->continuation_f = (void (*)(void *)) loop2;
        app->continuation_ctx = app;
      }
      else if(strcmp(app->cmd_buf , "loop3") == 0) {
        app->continuation_f = (void (*)(void *)) loop3;
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






int main(void)
{
  // hsi setup high speed internal!!!
  // TODO. not using.

  rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ] );  // stm32f411  upto 100MHz.


  // clocks
  rcc_periph_clock_enable(RCC_SYSCFG); // maybe required for external interupts?

  // LED
  rcc_periph_clock_enable(RCC_GPIOA); // led.

  // USART
  rcc_periph_clock_enable(RCC_GPIOB); // usart, voltage-source
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


  //
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


  //
  voltage_source_setup( ) ;


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

  // read main params from device - as starting point. should perhaps be flash
  // params_read( &app.params );

  printf("==========\n");


  loop_dispatcher( &app);

}




/*
  - OK. it is extremely confusing that we have to call write after doing this.
  spent 20 mins trying to figure it out.

*/
/*
  We need to add azero. to get a proper sense .
    - subtract the azero term. or subtract a moving average.
    --------------

    all this dispatch stuff would be nicer.

*/

/***************
// TODO should be using CString for cmd_buf ? see voltage-source-2 code for this.
// ALL this code relying on strcmp is dangerous. because it's not relying on null termination.
// alternatively should bzero(), memcpy( 0 ) nulls.
*/
// we may want to be able to read/store multiple calibrations. eg. array.
// but this is sufficient for the moment.

/*
  - calibration b -  is mostly going to be invariant.
  - basic steps.
      - on bootup. - read the device characteristics.   or load from flash.
      - can then run.  or calibrate.

*/



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


