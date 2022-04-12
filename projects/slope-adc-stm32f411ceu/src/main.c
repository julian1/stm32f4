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

  **************
  get the list of commands,

  egrep  'strcmp|sscan' ./src/main.c | sed 's/[^"]*\("[^"]*"\).*$/\1/'


"test comm"
"test reset %lu"
"flash erase"
"cal rescan"
"cal switch %lu"
"cal switch mod %lu"
"cal save %lu"
"cal show %lu"
"cal show"
"var_n show"
"fix_n show"
"var_n %lu"
"fix_n %lu"
"sleep %lu"
"voltage source dir %ld"
"voltage source set %lf"
"mux ref-lo"
"mux ref-hi"
"mux sig"
"mux ang"
"pattern %lu"
"temp show"
"last show"
"buffer show"
"buffer %lu"
"nplc show"
"nplc %lf"
"clk_count_var_pos_n %lu"
"h"
"loop1"
"loop2"
"loop3"
"loop4"
"loop22"


  **************
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
#include <malloc.h>   // malloc_stats





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
// #include "voltage-source.h"   // setup()

// bad conflicts with lib2/include/flash.h
#include "flash.h"
#include "cal.h"
#include "temp.h"


#include <meminfo.h>
// #include <matrix.h>
#include "regression.h"



#include "voltage-source-2/voltage-source.h"

#include "app.h"



// TODO change name spi_ice40_

static int spi_ice40_read_write_test(uint32_t spi )
{
  /*
    would be better to move into spi.c/ice40.c .
    but requires REG_LED define.
    pass reg_led...
  */
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
  ret = spi_ice40_reg_read(spi, REG_LED);
  // printf("ret is %x\n", ret); // value is completely wrong.

  spi_ice40_reg_write(spi, REG_LED , 0xff00ff);
  msleep(1);
  ret = spi_ice40_reg_read(spi, REG_LED);
  // printf("ret is %x\n", ret); // value is completely wrong.
  // ret value is completely wrong....
  if(ret != 0xff00ff)
    return -123;

  // this works... eg. allowing high bit to be off.
  spi_ice40_reg_write(spi, REG_LED, 0x7f00ff);
  ret = spi_ice40_reg_read(spi, REG_LED);
  if(ret != 0x7f00ff)
    return -123;

  for(uint32_t i = 0; i < 32; ++i) {
    spi_ice40_reg_write(spi, REG_LED , i );
    ret = spi_ice40_reg_read(spi, REG_LED);
    if(ret != i )
      return -123;
  }

  return 0;
}



/*
  OK. hang on. when we do a calibration.
  should always be stored in memory in cal_0 ?

  then we can save it - in any slot.

  when save. should always be cal0.

*/



void app_update_console_cmd(app_t *app)
{

  uint32_t u32;   // long unsigned int.
  // int32_t i32;
  double d;

  /* TODO.
      consider using non-blocking fread() for unified interface.

      or fgetc()  in non blocking mode, return EOF/-1 when no data.
  */


  int ch;
  clearerr(stdin);

  // using FILE, EOF is the normal case, on buffer empty.

  while( (ch = fgetc( stdin)) != EOF ) { // && -1 /EOF for error

    assert(ch >= 0);

/*
  while( ! cBufisEmpty(&app->console_in)) {

    // got a character
    int32_t ch = cBufPop(&app->console_in);
    assert(ch >= 0);
*/

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
        long ret = spi_ice40_read_write_test( app->spi );
        if(ret == 0)
          printf("ok\n");
        else
          printf("fail\n");
      }
      else if(sscanf(app->cmd_buf, "test reset %lu", &u32 ) == 1) {

        printf("reset test %lu seconds\n", u32 );

        // TODO change name ctrl_reset_enable()
        ctrl_reset_enable(app->spi);
        msleep( u32 * 1000);
        ctrl_reset_disable(app->spi);
      }



      // flash erase
      else if(strcmp(app->cmd_buf , "flash erase") == 0) {

          printf("flash erasing sector\n");
          usart1_flush();

          flash_erase_sector_();

          printf("done erase\n");

      }

      else if(sscanf(app->cmd_buf, "cal model %lu", &u32 ) == 1) {

        // set the current cal slot
        if(!( u32 == 3 || u32 == 4 || u32 == 5)) {
          printf("cal model not 3,4, 5\n");
        } else {
          app->cal_model = u32;
        }
      }
      else if(strcmp(app->cmd_buf , "cal model show") == 0) {

        printf("cal model %u\n", app->cal_model );
      }


      else if(strcmp(app->cmd_buf , "cal rescan") == 0) {
        // perhaps better as rescan. since scanned on startup.
        // might be better to have separate read
        printf("cal scan\n");

        FILE *f = flash_open_file();

        file_scan_cal( f, app->cal, ARRAY_SIZE(app->cal) );
        // file_scan_cal( f);

        fclose(f);
      }

      else if(sscanf(app->cmd_buf, "cal switch %lu", &u32 ) == 1) {

        // set the current cal slot
        if(!( u32 < ARRAY_SIZE(app->cal))) {
          printf("cal slot out of range\n");
        } else {

          // issue is that we are using null pointer to indcate if valid.
          Cal *b = app->cal[ u32 ];
          if(!b) {
            printf("no cal saved at slot %lu\n", u32);
          } else {
            printf("ok from %u to %lu\n", app->cal_idx, u32 );
            app->cal_idx = u32;
          }
        }
      }

      else if(sscanf(app->cmd_buf, "cal switch mod %lu", &u32 ) == 1) {

        // set the current cal slot
        if(!( u32 < ARRAY_SIZE(app->cal))) {
          printf("cal slot out of range\n");
        } else {

          // issue is that we are using null pointer to indcate if valid.
          Cal *b = app->cal[ u32 ];
          if(!b) {
            printf("cal slot %lu missing cal config\n", u32);
          } else {
            printf("ok from %u to %lu\n", app->cal_idx, u32 );

            // switch cal. and use the modulation parameters. associated with cal
            app->cal_idx = u32;
            Cal *cal = app->cal[  app->cal_idx ];

            ctrl_set_aperture( app->spi,  cal->param.clk_count_aper_n);
            ctrl_set_var_n( app->spi,     cal->param.clk_count_var_n);
            ctrl_set_fix_n( app->spi,     cal->param.clk_count_fix_n);

          }
        }
      }




      else if(sscanf(app->cmd_buf, "cal save %lu", &u32 ) == 1) {
        /*
          save the current cal. to slot.
          when we save. think we need to save to current memory for the slot also.
          to keep mem in sync with flash.
          otherwise need to rescan.
        */
        // get current matrix
        assert( app->cal_idx < ARRAY_SIZE(app->cal));
        Cal *cal_current = app->cal[ app->cal_idx ];

        // think we really want slots.
        if(!cal_current) {
          printf("no current cal to save\n");
        } else {

          // note cal at new position.
          Cal *old = app->cal[ u32 ];

          // copy into the old position
          app->cal[ u32 ] = cal_copy( cal_current );
          app->cal[ u32 ]->slot = u32;


          // now free old.
          // do in this order in case old==new.
          if( old) {
            cal_free( old);
          }



          // now save to flash
          printf("flash unlock\n");
          flash_unlock();

          FILE *f = flash_open_file();
          file_skip_to_end( f);

          file_write_cal ( app->cal[ u32 ] , f );
          fclose(f);

          printf("flash lock\n");
          flash_lock();
          printf("done\n");
          }
      }

      // would be useful to have 'cal show' and 'cal show idx'

      else if(sscanf(app->cmd_buf, "cal show %lu", &u32 ) == 1) {

        if( !( u32 < ARRAY_SIZE(app->cal))) {
          printf("out of range\n");
        } else {

          Cal *b = app->cal[ u32 ];
          if(!b) {
            printf("no cal\n");
          }
          else {
            cal_report( b );
            usart1_flush();
          }
        }
      }

      else if(strcmp(app->cmd_buf , "cal show") == 0 )  {

        if( !(app->cal_idx < ARRAY_SIZE(app->cal))) {

          printf("out of range\n");
        } else {
          Cal *b = app->cal[ app->cal_idx ];
          if(!b) {
            printf("no cal\n");
          }
          else {
            cal_report( b );
            usart1_flush();
          }
        }
      }

      else if(strcmp(app->cmd_buf , "cal free") == 0) {
        // free memory associated with cal load
        for(unsigned i = 0; i < ARRAY_SIZE(app->cal); ++i) {
          if(app->cal[i] ) {
            cal_free( app->cal[i] );
            app->cal[i] = NULL;
          }
        }
      }





      // OK. saving the parameters that were used. for cal. would be useful.
      // In fact. then we could drive the difference loop using them.
      // eg. changing nplc. or changing var_n, fix_n

      else if(strcmp(app->cmd_buf , "var_n show") == 0 )  {
        printf("var_n %lu\n", ctrl_get_var_n( app->spi));
      }
      else if(strcmp(app->cmd_buf , "fix_n show") == 0 )  {
        printf("fix_n %lu\n", ctrl_get_fix_n( app->spi));
      }


      else if(sscanf(app->cmd_buf, "var_n %lu", &u32 ) == 1) {
        printf("var_n %lu\n", u32);
        ctrl_set_var_n( app->spi, u32);
      }
      else if(sscanf(app->cmd_buf, "fix_n %lu", &u32 ) == 1) {
        printf("fix_n %lu\n", u32);
        ctrl_set_fix_n( app->spi, u32);
      }

      else if(sscanf(app->cmd_buf, "sleep %lu", &u32 ) == 1) {

        printf("sleep %lums\n", u32);
        // main looop keeps updating
        app_simple_sleep( app, u32 );
        printf("sleep done\n");
      }

/*
      else if(sscanf(app->cmd_buf, "voltage source dir %ld", &i32 ) == 1) {
        if( i32 == 0 || i32 == 1 || i32 == -1) {
          printf("voltage source dir %ld!\n", i32);
          voltage_source_set_dir( i32 );
        } else {
          printf("bad value\n");
        }
      }

      else if(sscanf(app->cmd_buf, "voltage source set %lf", &d ) == 1) {
        // set the voltage
        printf("voltage source set %lf!\n", d);
        app_voltage_source_set( app, d );
      }
*/


      else if(strcmp(app->cmd_buf , "voltage source setup") == 0 )  {

        spi_voltage_source_2_setup( app->spi_voltage_source, &app->spi_4094_reg);
      }

      else if(strcmp(app->cmd_buf , "voltage source powerdown") == 0 )  {

        spi_voltage_source_2_powerdown( app->spi_voltage_source, &app->spi_4094_reg);
      }

      else if( sscanf(app->cmd_buf, "voltage source set %lu %lf", &u32, &d )  == 2)  {

        // we need a break/continue/goto.

        if( !spi_voltage_source_2_in_on(&app->spi_4094_reg)) {
          usart1_printf("spi_voltage_source_2 not on\n");

        } else {

          printf("%lu %f\n", u32, d);
          if(u32 > 3)  {
              printf("bad dac channel, should be 0-3\n");

          } else {
            assert( app->spi_voltage_source == SPI2);

            printf("set channel %lu to output %fV\n", u32 , d );
            spi_voltage_source_2_set_val(app->spi_voltage_source, u32, d );
          }
        }
      }


      else if(strcmp(app->cmd_buf , "mux ref-lo") == 0 )  {   // fixme
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


      else if(sscanf(app->cmd_buf, "led blink %lu", &u32 ) == 1) {
        // set the voltage
        app->led_blink_interval = u32;
      }

      else if(strcmp(app->cmd_buf , "mem show") == 0) {

          printf("-------\n");
          printf("malloc\n");
          // Note that not all allocations are visible to mallinfo(); see BUGS and consider using malloc_info(3) instead.
          // mallinfo(stdout );
          //  The malloc_info() function is designed to address deficiencies in
          // malloc_stats(3) and mall
          malloc_stats();

          printf("-------\n");
          printf("mesch\n");

          printf("mesch mem_info_is_on() %u\n", mem_info_is_on());
          mem_dump_list(stdout, 0 );

      }


      // temp show.
      else if(strcmp(app->cmd_buf , "temp show") == 0) {

          double val = adc_temp_read10();
          printf("temp %.1fC\n", val);
      }




      else if(strcmp(app->cmd_buf , "last show") == 0 )  {   // fixme
        // set to flush
        if(!app->last) {
          printf("no last\n");
        } else {
          ffnctl( stdout, ffnctl( stdout, 0) | FILE_SYNC_ON_NEWLINE );
          m_octave_foutput( stdout, NULL, app->last);
          ffnctl( stdout, ffnctl( stdout, 0) & ~FILE_SYNC_ON_NEWLINE );
        }
      }

      else if(strcmp(app->cmd_buf , "last free") == 0) {
        // free associated memory with last long running loop operation
        if(app->last)
          M_FREE(app->last);
      }



      else if(strcmp(app->cmd_buf , "buffer show") == 0 )  {   // fixme
        printf("buffer %u\n",    m_rows(app->buffer));
      }
      else if(sscanf(app->cmd_buf, "buffer %lu", &u32 ) == 1) {
        if(u32 > 100) {
          u32 = 100;
        }
        printf("set buffer %lu\n", u32 );
        app->buffer = m_resize( app->buffer, u32, 1 );
        app->buffer_i = 0;
      }


      else if(strcmp(app->cmd_buf , "nplc show") == 0 )  {   // fixme
        int aperture = ctrl_get_aperture(app->spi); // in clk counts
        printf("nplc   %.2lf\n",  aper_n_to_nplc( aperture ));
        printf("period %.2lfs\n", aper_n_to_period( aperture ));
        // printf("buffer %u\n",    m_rows(app->buffer));
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
      else if(strcmp(app->cmd_buf , "loop1") == 0) {
        // start app_loop1.
        app->continuation_f = (void (*)(void *)) app_loop1;
        app->continuation_ctx = app;
      }
      else if(strcmp(app->cmd_buf , "loop2") == 0) {  // cal loop.
        app->continuation_f = (void (*)(void *)) app_loop2;
        app->continuation_ctx = app;
      }
      else if(strcmp(app->cmd_buf , "loop3") == 0) {
        app->continuation_f = (void (*)(void *)) app_loop3;
        app->continuation_ctx = app;
      }
      else if(strcmp(app->cmd_buf , "loop4") == 0) {
        app->continuation_f = (void (*)(void *)) app_loop4;
        app->continuation_ctx = app;
      }
      else if(strcmp(app->cmd_buf , "loop22") == 0) {
        app->continuation_f = (void (*)(void *)) app_loop22;
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
  rcc_periph_clock_enable(RCC_SPI2);  // voltage-source-2


  // adc/temp
	rcc_periph_clock_enable(RCC_ADC1);



  //////////////////////
  // setup

  // 16MHz. from hsi datasheet.
  // systick_setup(16000);
  systick_setup(84000);

  // led
  led_setup();



  //
  memset(&app, 0, sizeof(app_t));

  app.led_blink_interval  = 500;
  app.cal_model     = 3;

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
  cbuf_init_stdout_streams(  &app.console_out );
  // for fread, fgetch etc
  cbuf_init_stdin_streams( &app.console_in );


  ////////////////
  // spi1/ice40
  spi1_port_cs1_setup();

  app.spi = SPI1;

  spi_ice40_setup(SPI1);

  // adc interupt...
  spi1_interupt_gpio_setup( (void (*) (void *))app_spi1_interupt, &app);


  // good context.
  app.spi_4094_reg = 0;
  app.spi_voltage_source = SPI2;

#if 0
  // needs a context...
  voltage_source_setup( /*ctx */ );
#endif


  // for temp
  adc_setup();
  // read some values to clear
  adc_temp_read10();


  /////////////

  assert(sizeof(bool) == 1);
  assert(sizeof(long) == 4);
  assert(sizeof(signed) == 4);
  assert(sizeof(void *) == 4);
  assert(sizeof(double) == 8);
  // assert(1 == 2);

  printf("==========\n");

  printf("addr main() %p\n", main );

  usart1_flush();


  // mesch turn on mem tracking
  mem_info_on(1 );



  // try to load cal
  FILE *f = flash_open_file();
  assert(f);
  file_scan_cal( f, app.cal, ARRAY_SIZE(app.cal) );

  fclose(f);


  /////////////////////



  // set the buffer
  app.buffer = m_resize( app.buffer, 1, 1 );
  app.buffer_i = 0;

  // stats buffer for reporting
  app.stats_buffer = m_resize( app.stats_buffer, 10, 1 );
  app.stats_buffer_i = 0;



  app.cal_comment = "330pF/45kHz/250k2";

  app_loop_dispatcher( &app);

}



/***************
// TODO should be using CString for cmd_buf ? see voltage-source-2 code for this.
// ALL this code relying on strcmp is dangerous. because it's not relying on null termination.
// alternatively should bzero(), memcpy( 0 ) nulls.
*/
// we may want to be able to read/store multiple calibrations. eg. array.
// but this is sufficient for the moment.


#if 0
  /////////////////////
  // try to load cal
  FILE *f = flash_open_file();

  if(file_skip_to_last_valid(  f) != 0) {
    printf("no valid config found\n" );
  } else {

    app.b = m_read_flash( MNULL, f );
    printf("loadeded cal\n" );
    m_foutput( stdout, app.b  );
    usart1_flush();
  }
  fclose(f);

#endif

#if 0
      // flash write test
      else if(strcmp(app->cmd_buf , "flash write test") == 0) {

        // TODO check if need to unlock to write. or only for erase.
        printf("flash unlock\n");
        flash_unlock();

        FILE *f = flash_open_file();
        file_skip_to_end( f);

        MAT *m = m_get(10, 2);
        // would be cool , to have some variable
        m_set_val( m, 2, 0, 123.456 )  ;
        file_write_cal ( m, f );
        fclose(f);

        printf("flash lock\n");
        flash_lock();
        printf("done\n");
      }
#endif


      // flash write cal
      // should be called 'cal save', 'cal load'  etc?

#if 0
      else if(sscanf(app->cmd_buf, "cal save %lu", &u32 ) == 1) {

        // think we really want slots.
        if(!app->cal) {

          printf("no cal to writesave\n");
        } else {

          printf("flash unlock\n");
          flash_unlock();

          FILE *f = flash_open_file();
          file_skip_to_end( f);

          // write cal matrix
          unsigned slot = u32;
          file_write_cal ( app->cal, slot, f );
          fclose(f);

          printf("flash lock\n");
          flash_lock();
          printf("done\n");
        }
      }

      // flash read test. doesn't load cal.
      // but might be useful to revert
      else if(strcmp(app->cmd_buf , "cal read") == 0) {

        // might be better to have separate read
        printf("cal read from flash\n");

        FILE *f = flash_open_file();
        if(file_skip_to_last_valid(  f) != 0) {
          printf("no valid config found\n" );
        }
        else {
          app->cal = m_read_flash( MNULL, f );
          m_foutput( stdout, app->cal );
          usart1_flush();
        }

        fclose(f);
      }

#endif

