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











static void app_loop_dispatcher(app_t *app);




static int reg_read_write_test(void)
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
  // printf("ret is %x\n", ret); // value is completely wrong.

  spi_reg_write(SPI1, REG_LED , 0xff00ff);
  msleep(1);
  ret = spi_reg_read(SPI1, REG_LED);
  // printf("ret is %x\n", ret); // value is completely wrong.
  // ret value is completely wrong....
  if(ret != 0xff00ff)
    return -123;

  // this works... eg. allowing high bit to be off.
  spi_reg_write(SPI1, REG_LED, 0x7f00ff);
  ret = spi_reg_read(SPI1, REG_LED);
  if(ret != 0x7f00ff)
    return -123;

  for(uint32_t i = 0; i < 32; ++i) {
    spi_reg_write(SPI1, REG_LED , i );
    ret = spi_reg_read(SPI1, REG_LED);
    if(ret != i )
      return -123;
  }

  return 0;
}



void app_update_console_cmd(app_t *app)
{

  uint32_t u32;
  // int32_t i32;
  double d;


  while( ! cBufisEmpty(&app->console_in)) {

    // got a character
    int32_t ch = cBufPop(&app->console_in);
    assert(ch >= 0);

    if(! ( ch == ';' || ch == '\r')) {
      // character other than newline
      // push onto a vector? or array?

      // ignore leading whitespace
      if( ch == ' ' && app->cmd_buf_i == 0)  {
        putchar( ch);
        return;
      }

      if( app->cmd_buf_i < CMD_BUF_SZ - 1 )
        app->cmd_buf[ app->cmd_buf_i++ ] = ch;

      // echo to output. required for minicom.
      putchar( ch);

    }  else {

      // code should be CString. but this kind of works well enough...
      // we got a command
      app->cmd_buf[ app->cmd_buf_i ]  = 0;

      putchar('\n');
      // printf("got command '%s'\n", app->cmd_buf );


      if(strcmp(app->cmd_buf , "test comm") == 0) {
        /*
          avoid doing this on initializtion is useful
          permits test development for code that is not reliant fpga.
        */
        long ret = reg_read_write_test();
        if(ret == 0)
          printf("ok\n");
        else
          printf("fail\n");
      }

      // flash erase
      else if(strcmp(app->cmd_buf , "flash erase") == 0) {

          printf("flash erasing sector\n");
          usart1_flush();

          flash_erase_sector_();

          printf("done erase\n");

      }

      // flash write cal
      else if(strcmp(app->cmd_buf , "flash write cal") == 0) {

        if(!app->b) {

          printf("no cal to save\n");
        } else {

          printf("flash unlock\n");
          flash_unlock();

          FILE *f = open_flash_file();
          c_skip_to_end( f);

          // write cal matrix
          m_write_flash ( app->b, f );
          fclose(f);

          printf("flash lock\n");
          flash_lock();
          printf("done\n");
        }
      }

      // flash write test
      else if(strcmp(app->cmd_buf , "flash write test") == 0) {

        // TODO check if need to unlock to write. or only for erase.
        printf("flash unlock\n");
        flash_unlock();

        FILE *f = open_flash_file();
        c_skip_to_end( f);

        MAT *m = m_get(10, 2);
        // would be cool , to have some variable
        m_set_val( m, 2, 0, 123.456 )  ;
        m_write_flash ( m, f );
        fclose(f);

        printf("flash lock\n");
        flash_lock();
        printf("done\n");
      }

      // flash read test. doesn't load cal.
      // but might be useful to revert
      else if(strcmp(app->cmd_buf , "flash read") == 0) {

        printf("flash read\n");

        FILE *f = open_flash_file();
        if(c_skip_to_last_valid(  f) != 0) {
          printf("no valid config found\n" );
        }
        else {
          MAT *u  = m_read_flash( MNULL, f );
          m_foutput( stdout, u );
          usart1_flush();
        }

        fclose(f);
      }

      else if(sscanf(app->cmd_buf, "sleep %lu", &u32 ) == 1) {

        printf("sleep %lums\n", u32);

        // main looop keeps updating
        app_simple_sleep( app, u32 );
        printf("sleep done\n");
      }
  
      /*
      // change name source ? or test-source ?
      else if(sscanf(app->cmd_buf, "vs %ld", &i32 ) == 1) {
        printf("voltage source %ld!\n", i32);
        voltage_source_set( i32 );
      }
      */

      else if(sscanf(app->cmd_buf, "vs %lf", &d ) == 1) {
        // set the voltage
        printf("voltage source set %lf!\n", d);
        app_voltage_source_set( app, d );
      }


      else if(strcmp(app->cmd_buf , "mux ref-lo") == 0 )  {
        printf("setting mux ref-lo\n");
        ctrl_reset_enable(app->spi);
        ctrl_set_mux( app->spi, HIMUX_SEL_REF_LO );
        ctrl_reset_disable(app->spi);
      }
      else if(strcmp(app->cmd_buf , "mux ref-hi") == 0) {
        printf("setting mux ref-hi\n");
        ctrl_reset_enable(app->spi);
        ctrl_set_mux( app->spi, HIMUX_SEL_REF_HI );
        ctrl_reset_disable(app->spi);
      }
      else if(strcmp(app->cmd_buf , "mux sig") == 0) {
        printf("setting mux sig\n");
        ctrl_reset_enable(app->spi);
        ctrl_set_mux( app->spi, HIMUX_SEL_SIG_HI );
        ctrl_reset_disable(app->spi );
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
        ctrl_reset_enable(app->spi);
        msleep( u32 * 1000);
        ctrl_reset_disable(app->spi);
      }


      // should read as a double
      // actually an aperture setting.


      else if(sscanf(app->cmd_buf, "buffer %lu", &u32 ) == 1) {
        if(u32 > 100) {
          u32 = 100;
        }
        printf("set buffer %lu\n", u32 );
        app->buffer = m_resize( app->buffer, u32, 1 );
        app->buffer_i = 0;
      }


      else if(sscanf(app->cmd_buf, "nplc %lf", &d ) == 1) {
        printf("setting nplc %lf\n", d );
        uint32_t aper = nplc_to_aper_n( d );
        ctrl_reset_enable(app->spi);
        ctrl_set_aperture( app->spi, aper );
        ctrl_reset_disable(app->spi);
      }

#if 0
      // else if(strncmp(app->cmd_buf , "clk_count_var_pos_n", 19) == 0) {
      else if(sscanf(app->cmd_buf, "clk_count_var_pos_n %lu", &u32 ) == 1) {

          printf("setting clk_count_var_pos_n %lu\n", u32);

      }
#endif

      else if(strcmp(app->cmd_buf , "h") == 0 || strcmp(app->cmd_buf , "halt") == 0) {
        // exit the current loop program
        app->continuation_f = (void (*)(void *)) app_loop_dispatcher;
        app->continuation_ctx = app;
      }
      else if(strcmp(app->cmd_buf , "app_loop1") == 0) {
        // start app_loop1.
        app->continuation_f = (void (*)(void *)) app_loop1;
        app->continuation_ctx = app;
      }

      else if(strcmp(app->cmd_buf , "app_loop2") == 0) {  // cal loop.
        app->continuation_f = (void (*)(void *)) app_loop2;
        app->continuation_ctx = app;
      }


      else if(strcmp(app->cmd_buf , "app_loop3") == 0) {
        app->continuation_f = (void (*)(void *)) app_loop3;
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
      printf("> ");

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





void app_simple_sleep( app_t * app, uint32_t period )
{
  // not static
  uint32_t soft_timer = system_millis;

  while(true) {
    // keep pumping messages
    app_update_console_cmd(app);
    app_update_led(app);

    if( (system_millis - soft_timer ) > period ) {
      return;
    }
  }
}





void app_update_led(app_t *app)
{
  assert(app);

  // 500ms soft timer. should handle wrap around
  if( (system_millis - app->soft_500ms) > 500) {
    app->soft_500ms += 500;
    led_toggle();
  }

}



static void app_loop_dispatcher(app_t *app)
{
  printf("=========\n");
  printf("continuation dispatcher\n");
  printf("> ");

  while(true) {

    app_update_console_cmd(app);
    app_update_led( app);


    if(app->continuation_f) {
      printf("jump to continuation\n");
      void (*tmppf)(void *) = app->continuation_f;
      app->continuation_f = NULL;
      tmppf( app->continuation_ctx );

      printf("continuation done\n");
      printf("> ");
    }
  }

}





static void app_spi1_interupt(app_t *app )
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
  // usart1_setup_gpio_portA();
  usart1_setup_gpio_portB();

  usart1_set_buffers(&app.console_in, &app.console_out);

  // standard streams for printf, fprintf, putc.
  cbuf_init_std_streams(  &app.console_out );


  ////////////////
  // spi1/ice40
  spi1_port_cs1_setup();

  app.spi = SPI1;

  spi_ice40_setup(SPI1);

  // adc interupt...
  spi1_interupt_gpio_setup( (void (*) (void *))app_spi1_interupt, &app);


  // needs a context...
  voltage_source_setup( ) ;



  /////////////

  printf("\n--------\n");
  printf("starting loop\n");
  printf("sizeof bool   %u\n", sizeof(bool));
  printf("sizeof float  %u\n", sizeof(float));
  printf("sizeof double %u\n", sizeof(double));

  assert(sizeof(long) == 4);
  assert(sizeof(signed) == 4);
  assert(sizeof(void *) == 4);

  // assert(1 == 2);

  printf("a float formatted %g\n", 123.456f );


  printf("\n--------\n");
  printf("addr main() %p\n", main );

  usart1_flush();

  // read main params from device - as starting point. should perhaps be flash
  // params_read( &app.params );

  printf("==========\n");


  /////////////////////
  // try to load cal
  FILE *f = open_flash_file();

  if(c_skip_to_last_valid(  f) != 0) {
    printf("no valid config found\n" );
  } else {

    app.b = m_read_flash( MNULL, f );
    printf("loadeded cal\n" );
    m_foutput( stdout, app.b  );
    usart1_flush();
  }
  fclose(f);

  /////////////////////



  // set the buffer
  app.buffer = m_resize( app.buffer, 1, 1 );
  app.buffer_i = 0;

  // stats buffer for reporting
  app.stats_buffer = m_resize( app.stats_buffer, 10, 1 );
  app.stats_buffer_i = 0;



  app_loop_dispatcher( &app);

}



/***************
// TODO should be using CString for cmd_buf ? see voltage-source-2 code for this.
// ALL this code relying on strcmp is dangerous. because it's not relying on null termination.
// alternatively should bzero(), memcpy( 0 ) nulls.
*/
// we may want to be able to read/store multiple calibrations. eg. array.
// but this is sufficient for the moment.



