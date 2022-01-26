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
#include "usart2.h"
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

// spi
#define REG_LED  7





typedef struct app_t
{
  CBuf console_in;
  CBuf console_out;

  bool data_ready ;


  // FBuf      measure_rundown;



} app_t;


static void update_console_cmd(app_t *app)
{

  if( !cBufisEmpty(&app->console_in) && cBufPeekLast(&app->console_in) == '\r') {

    // usart_printf("got CR\n");

    // we got a carriage return
    static char tmp[1000];

    size_t nn = cBufCount(&app->console_in);
    size_t n = cBufCopyString(&app->console_in, tmp, ARRAY_SIZE(tmp));
    assert(n <= sizeof(tmp));
    assert(tmp[n - 1] == 0);
    assert( nn == n - 1);

    // chop off the CR to make easier to print
    assert(((int) n) - 2 >= 0);
    tmp[n - 2] = 0;

    // TODO first char 'g' gets omitted/chopped here, why? CR handling?
    usart_printf("got command '%s'\n", tmp);

    // process_cmd(app, tmp);
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






#define REG_LED               7

#define REG_COUNT_UP          9
#define REG_COUNT_DOWN        10
#define REG_CLK_COUNT_RUNDOWN 11
#define REG_COUNT_TRANS_UP    12
#define REG_COUNT_TRANS_DOWN  14
#define REG_COUNT_FIX_UP      26
#define REG_COUNT_FIX_DOWN    27
#define REG_COUNT_FLIP        17


#define REG_TEST              15
#define REG_RUNDOWN_DIR       16


// control parameters
#define REG_CLK_COUNT_INIT_N  18
#define REG_CLK_COUNT_FIX_N   20
#define REG_CLK_COUNT_VAR_N   21
#define REG_CLK_COUNT_INT_N_LO   22
#define REG_CLK_COUNT_INT_N_HI 23


#define REG_USE_SLOW_RUNDOWN  24
#define REG_HIMUX_SEL         25

// eg. bitwise, active lo.  avoid turning on more than one.
// although switch has 1.5k impedance so should not break
#define HIMUX_SEL_SIG_HI      (0xf & ~(1 << 0))
#define HIMUX_SEL_REF_HI      (0xf &~(1 << 1))
#define HIMUX_SEL_REF_LO      (0xf &~(1 << 2))
#define HIMUX_SEL_ANG         (0xf &~(1 << 3))





struct Params
{
  // fix counts. are setup and written in
  uint32_t reg_led ;
  uint32_t clk_count_int_n;

  uint32_t clk_count_init_n ;
  uint32_t clk_count_fix_n ;
  uint32_t clk_count_var_n ;
  uint32_t use_slow_rundown;
  uint32_t himux_sel;

};

typedef struct Params Params;


// initialize. first read. then overwrite. then write again.

static void params_read( Params * params )
{
  params->reg_led           = spi_reg_read(SPI1, REG_LED);

  uint32_t int_lo = spi_reg_read(SPI1, REG_CLK_COUNT_INT_N_LO );
  uint32_t int_hi = spi_reg_read(SPI1, REG_CLK_COUNT_INT_N_HI );
  params->clk_count_int_n   = int_hi << 24 | int_lo;

  params->clk_count_init_n  = spi_reg_read(SPI1, REG_CLK_COUNT_INIT_N);
  params->clk_count_fix_n   = spi_reg_read(SPI1, REG_CLK_COUNT_FIX_N);
  params->clk_count_var_n   = spi_reg_read(SPI1, REG_CLK_COUNT_VAR_N);

  params->use_slow_rundown  = spi_reg_read(SPI1, REG_USE_SLOW_RUNDOWN);
  params->himux_sel         = spi_reg_read(SPI1, REG_HIMUX_SEL);
}



static void params_report(Params * params )
{
  char buf[10];

  usart_printf("-------------\n");
  usart_printf("reg_led           %s\n", format_bits( buf, 4, params->reg_led ) );

  uint32_t int_n  = params->clk_count_int_n ;
  double period = int_n / (double ) 20000000;
  double nplc     = period / (1.0 / 50);

  usart_printf("clk_count_int_n   %u\n", int_n );
  usart_printf("period            %fs\n", period);
  usart_printf("nplc              %.2f\n", nplc);

  usart_printf("clk_count_init_n  %u\n", params->clk_count_init_n);
  usart_printf("clk_count_fix_n   %u\n", params->clk_count_fix_n);
  usart_printf("clk_count_var_n   %u\n", params->clk_count_var_n);

  usart_printf("use_slow_rundown  %u\n", params->use_slow_rundown);

  //
  // char buf[100] char * format_bits(char *buf, size_t width, uint32_t value)
  usart_printf("himux_sel         %s\n", format_bits( buf, 4, params->himux_sel));
}





static void params_write_main( Params *params ) // uint32_t clk_count_int_n, bool use_slow_rundown, uint8_t himux_sel )
{
  // write the main parameter to device

  // encapsutate into a function.
  // uint32_t t = 5 * 20000000;
  // printf("params_set %lu, %u, %u\n", clk_count_int_n,  use_slow_rundown, himux_sel );

  spi_reg_write(SPI1, REG_CLK_COUNT_INT_N_HI, (params->clk_count_int_n >> 24) & 0xff );
  spi_reg_write(SPI1, REG_CLK_COUNT_INT_N_LO, params->clk_count_int_n & 0xffffff  );
  spi_reg_write(SPI1, REG_USE_SLOW_RUNDOWN, params->use_slow_rundown );
  spi_reg_write(SPI1, REG_HIMUX_SEL, params->himux_sel );
}



#if 0
static void params_set( uint32_t clk_count_int_n, bool use_slow_rundown, uint8_t himux_sel )
{
  // int params_set

  // encapsutate into a function.
  // uint32_t t = 5 * 20000000;

  printf("params_set %lu, %u, %u\n", clk_count_int_n,  use_slow_rundown, himux_sel );

  spi_reg_write(SPI1, REG_CLK_COUNT_INT_N_HI, (clk_count_int_n >> 24) & 0xff );
  spi_reg_write(SPI1, REG_CLK_COUNT_INT_N_LO, clk_count_int_n & 0xffffff  );
  spi_reg_write(SPI1, REG_USE_SLOW_RUNDOWN, use_slow_rundown );
  spi_reg_write(SPI1, REG_HIMUX_SEL, himux_sel );
}
#endif



/*
  - ok. think we want an intermediate structure...
  so we can use this once

  - could also record the configuration
*/


struct Run
{
  uint32_t count_up;
  uint32_t count_down;
  uint32_t count_trans_up;
  uint32_t count_trans_down;
  uint32_t count_fix_up;
  uint32_t count_fix_down;
  uint32_t count_flip;

  // rundown_dir.

  uint32_t clk_count_rundown;

};

typedef struct Run  Run;


static void run_read( Run *run )
{
  assert(run);

  // use separate lines (to make it easier to filter - for plugging into stats).
  run->count_up         = spi_reg_read(SPI1, REG_COUNT_UP );
  run->count_down       = spi_reg_read(SPI1, REG_COUNT_DOWN );

  // run->count_trans_up     = spi_reg_read(SPI1, REG_COUNT_TRANS_UP );
  // run->count_trans_down   = spi_reg_read(SPI1, REG_COUNT_TRANS_DOWN );

  run->count_fix_up     = spi_reg_read(SPI1, REG_COUNT_FIX_UP);
  run->count_fix_down   = spi_reg_read(SPI1, REG_COUNT_FIX_DOWN);

  // run->count_flip    = spi_reg_read(SPI1, REG_COUNT_FLIP);


  // WE could record slow_rundown separate to normal rundown.
  run->clk_count_rundown = spi_reg_read(SPI1, REG_CLK_COUNT_RUNDOWN );

}




static void run_report( Run *run )
{
  assert(run);

  // usart_printf("count_up %u, ",         run->count_up );
  // usart_printf("count_down %u, ",       run->count_down );

  usart_printf("count_up/down %u %u, ", run->count_up, run->count_down );
  // usart_printf("trans_up/down %u %u, ", run->count_trans_up,  run->count_trans_down);
  usart_printf("fix_up/down %u %u, ",   run->count_fix_up,  run->count_fix_down);
  // usart_printf("count_flip %u, ",       run->count_flip);

  usart_printf("clk_count_rundown %u, ", run->clk_count_rundown);

  usart_printf("\n");
}


static MAT * run_to_matrix( Params *params, Run *run, MAT * out )
{
  /*
    EXTR. IMPORTANT.
    1. must calculate the estimator values before average rather than average raw inputs (rundown count etc) then cal estimated.

    - because the modulation could flutter around the hi count values. so that an average
    does not accurately capture the combination with slow rundown count.
    ------------

    EXTR IMPORTANT
    *******
    2. rather than represent the slow rundown as an independent field .
    it could be represented - as a clk count of *both* pos and neg.
    likewise the fast rundown

    eg. total current =
    (fix pos + var pos + rundown pos) + (fix neg + var neg  + rundown neg )
    = c + a * pos + b * neg

    rather than 3 independent var linear regression, it collapses to a 2 var regression.

    So that the calculation collpases to just a two independent variable linear regression.
    And if fast runddown is used then it is just 0 clk.

    =======
  */

  UNUSED(params);

  if(out == MNULL)
    out = m_get(1,1);

#if 0
  // compute value
  m_resize(out, 1, 3);
  m_set_val( out, 0, 0,  run->count_up );
  m_set_val( out, 0, 1,  run->count_down );
  m_set_val( out, 0, 2,  run->count_fix_up );
#endif


  // slow rundown uses both

  // negative current / slope up
  double x1 = (run->count_up * params->clk_count_var_n) + (run->count_fix_up * params->clk_count_fix_n)  + run->clk_count_rundown;

  // positive current. slope down.
  double x2 = (run->count_down * params->clk_count_var_n) + (run->count_fix_down * params->clk_count_fix_n) + run->clk_count_rundown;


  m_resize(out, 1, 2);
  m_set_val( out, 0, 0,  x1  );
  m_set_val( out, 0, 1,  x2  );


  return out;
}







static void cal_loop(app_t *app, MAT *x, MAT *y )
{
  // app argument is needed for data ready flag.
  // while loop has to be inner
  // might be easier to overside. and then resize.

  usart_printf("=========\n");
  usart_printf("cal loop\n");

  // rows x cols
  unsigned row = 0;

  #define MAX_OBS  30
  #define X_COLS   2

  m_resize( x , MAX_OBS, X_COLS );      // constant + pos clk + neg clk.
  m_resize( y , MAX_OBS, 1 );



  Params  params;
  params_read( &params );   // change name read_from_device ?

  for(unsigned i = 0; i < 10; ++i )
  {
    double target;


    // switch integration configuration
    switch(i) {

      case 0:
        // params_set( 1 * 20000000, 1, HIMUX_SEL_REF_LO ); target = 0.0; break;
        params.clk_count_int_n  = 1 * 20000000;
        params.use_slow_rundown = 1;
        params.himux_sel = HIMUX_SEL_REF_LO;
        target = 0.0;

        params_report(&params);
        params_write_main(&params);
        break;

      case 1:
        // same except mux lo.
        params.himux_sel = HIMUX_SEL_REF_HI;
        target = 7.1;

        params_report(&params);
        params_write_main(& params);
        break;

      default:
        // we finished, getting all data
        // return here makes while loop simpler.
        usart_printf("done calibrating\n");

        // shrink matrixes for the data
        m_resize( x , row, X_COLS   );
        m_resize( y , row, 1 );

        // we could do the cal here...

        return;
        break;

      // 2 case exits
    } // switch


    ////

    // obs per current configuration
    unsigned obs = 0;

    while(obs < 5) {

      // if we got data handle it.
      if(app->data_ready) {
        // in priority
        app->data_ready = false;

        // get run details
        Run run;
        run_read(&run );
        run_report(&run);

        // ignore first obs
        if(obs >= 1) {

          MAT *whoot = run_to_matrix( &params, &run, MNULL );
          assert(whoot);
          // m_foutput(stdout, whoot );
          m_row_set( x, row, whoot );
          M_FREE(whoot);

          // do y
          assert(row < y->m); // < or <= ????
          m_set_val( y, row, 0,  target );

          ++row;
        } else {
          usart_printf("discard\n");

        }

        ++obs;
      }

      // update_console_cmd(app);
      // usart_output_update(); // shouldn't be necessary, now pumped by interupts.


      // 250ms
      static uint32_t soft_250ms = 0;
      if( (system_millis - soft_250ms) > 250) {
        soft_250ms += 250;
        led_toggle();
      }


    } // while



  } // state for

}


// hmmm weights are all off...


static void loop(app_t *app, MAT *b)
{
  usart_printf("=========\n");
  usart_printf("main loop\n");


  /*
    loop() subsumes update()
  */

  assert( HIMUX_SEL_REF_LO ==  0b1011  );


 // params_set( 5 * 20000000, 1, HIMUX_SEL_REF_LO );


  Params  params;
  params_read( &params );

  usart_printf("overwriting params\n");
  // overwrite
  params.clk_count_int_n  = 1 * 20000000;
  params.use_slow_rundown = 1;
  // params.himux_sel = HIMUX_SEL_REF_LO;
  // params.himux_sel = HIMUX_SEL_REF_HI;
  params.himux_sel = HIMUX_SEL_SIG_HI;
  params_write_main(&params);

  params_report( &params);


  // TODO move to app_t structure?.
  static uint32_t soft_500ms = 0;

  while(true) {


    if(app->data_ready) {
      // in priority

      Run run;
      run_read(&run );
      run_report(&run);


      MAT *x = run_to_matrix( &params, &run, MNULL );
      assert(x );


      MAT *predicted = m_mlt(x, b, MNULL );
      printf("predicted \n");
      m_foutput(stdout, predicted );
      usart_flush();

      M_FREE(x);

#if 0
      // compute value
      MAT *x = m_get(1, 4);
      m_set_val( x, 0, 0,  1.f );   // should be 1...
      m_set_val( x, 0, 1,  run.count_up );
      m_set_val( x, 0, 2,  run.count_down );
      m_set_val( x, 0, 3,  run.clk_count_rundown );

      MAT *predicted = m_mlt(x, b, MNULL );
      printf("predicted \n");
      m_foutput(stdout, predicted );

      M_FREE(predicted);
#endif

      app->data_ready = false;
    }


    update_console_cmd(app);
    // usart_output_update(); // shouldn't be necessary, now pumped by interupts.

    // 500ms soft timer. should handle wrap around
    if( (system_millis - soft_500ms) > 500) {
      soft_500ms += 500;

      //
      led_toggle();

#if 0
      static int count = 0;
      uint32_t ret = spi_reg_xfer_24(SPI1, 7, count );
      usart_printf("here %u  %u\n", count ,  ret);
      ++count;
#endif

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
  spi1_port_setup();

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


// test writing reg led.
#if 1
  // test ice40 register read/write
  // ok. seems to work.
  uint32_t ret;

  usart_flush();

/*
  OK. i think these spi calls may fail when speed of design falls below 32MHz.
  because

  IMPORTANT.
  OK. we removed reg_led from the verilog initial block.
  and now we the values are correct.
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
#endif


  // state_change(&app, STATE_FIRST );

  printf("==========\n");
  ////////////////////////////////////
  // produces two return values.
  // mnull for both args fails ...
  // MAT *x_ = MNULL;
  // MAT *y = MNULL;

  MAT *x = m_get(1,1);
  MAT *y = m_get(1,1);

  cal_loop(&app, x, y );

  printf("x\n");
  m_foutput(stdout, x);
  usart_flush();

  printf("y\n");
  m_foutput(stdout, y);
  usart_flush();



#if 1
  MAT *b =  regression( x, y, MNULL );
  printf("b\n");
  m_foutput(stdout, b);

  usart_flush();

  MAT *predicted = m_mlt(x, b, MNULL );
  printf("predicted \n");
  m_foutput(stdout, predicted );
  usart_flush();
#endif

  // TODO clean up mem.
  // TODO. our circular buffer does not handle overflow very nicely. - the result is truncated.

/*
  M_FREE(x);
  M_FREE(x_);
  M_FREE(y);
  M_FREE(b);
  M_FREE(predicted);

*/


  loop(&app, b );
}




    /*
      EXTR
        fixed pos == fixed neg. so only record once - and it becomes a constant.
      -------
        8****
        - i think as soon as we hit int period. eg. 1PLC. or 200ms. we must turn of input immediately.
        - not wait until we we come to the end of a phase (var) . and then test whether we finished.
        - eg. we must have the time of the input signal - to be absolutely constant between measurements.
           - regardless we use fast rundown or slow rundown.

          - this means. being able to switch ref currents and signal independently.

        ***

        =========================
        - should try to get the two parameter thing working. with rundown.   (non slow).
          eg.  fix pos + var pos + rundown.   and fix neg + var neg.
          ----------
          no. because in the rundown the input is turned off. so it must be a different variable. but maybe should be left on.
        =========================

        - combine fix and var.  eg. so have total pos total neg in raw clock counts, and then add the raw rundown.
        - OR get the fpga - to count it up - in raw counts.
        - would need to 0
        - IMPORTNAT - then we have just two variables. if not using slow rundown. and three if are.

        - our flip_count is wrong. and confusing things. with fixed amount.

        - change names count_up , count_down.   count_fix_up,  count_var_up etc.  or cout_fix_pos.

        - make sure not including rundown in counts.

        - include the fix pos and neg counts
            even though they are equal.
            they are equal time.
            but they are *not* equal current.
            ---------

            for a certain integration time/period.    they will be constant.
            but we cannot create permutations with different integration times - if do not include.

        - for a certain time. the pos + neg should be a constant.

        - OR. instead of using counts. multiply by the times.
            then we could generate permutations.
            and then include the count * the limit.

        - perhaps we need three points. and generating extra values by running  at multiple of nplc is insufficient.
            for degrees freedom.

        - could record slow_rundown as separate var to rundown. and thus
          handle both options in the same calibration data.

          - actually this is quite interesting - because it would generate the 0 data points.

        - can/should  add a dummy observation.
            eg. count_up 0 , count_down 0, rundown 0 == 0

        - or perhaps better. without the slow slope.
            eg. just plug in 0 for the rundown.

        - think should probably not have constant.

        - we need a function run -> x_ vector.

        - perhaps try entire calibration without slow slope. as a first test.
        - and then secondary calibration. using the predicted values as input.

    */















  //MAT *x = concat_ones( x_, MNULL );

/*
  printf("x\n");
  m_foutput(stdout, x_ );

  printf("y\n");
  m_foutput(stdout, y );
*/

  // IMIPOORTANT -  perhaps the issue is cbuffer is overflowing????
  // yes seems ok.

  // printf("m is %u n",  x_->m );

/*
  // ths ones code looks buggy.
  MAT *ones = m_ones( m_copy( x_, MNULL  ));
  printf("ones\n");
  m_foutput(stdout, ones );
*/

/*
  MAT *j = m_get( x_-> m, 1 );
  MAT *ones = m_ones( j );
  printf("ones\n");
  m_foutput(stdout, ones );
*/

/*
  MAT *j = m_get( x_-> m, 1 );
  MAT *ones = m_ones( j );

  printf("ones\n");
  m_foutput(stdout, ones );
*/



// data is wrong. until the buffers are full.
#if 0
  // computed via octave
  // double v = (-6.0000e+00 * 1) + (4.6875e-02 * count_up) + ( -3.1250e-02 * count_down) + (-4.5475e-12 * clk_count_rundown);
  double v = (-6.0000e+00 * 1) + (4.6875e-02 * count_up) + ( -3.1250e-02 * count_down) + (-4.5475e-7 * clk_count_rundown);
  usart_printf("v %.7f, ", v );
#endif


#if 0
  static float clk_count_rundown_ar[ 10 ] ;
  size_t n = 5;
  // static int i = 0;

  float mean_;
  UNUSED(mean_);
  ////////////////////////
  ///////// stats

  // usart_printf("imodn %u ", i % n);

  {
  assert(n <= ARRAY_SIZE(clk_count_rundown_ar));

  clk_count_rundown_ar[ i % n ] =  clk_count_rundown;
  usart_printf("stddev_rundown(%u) %.2f, ", n, stddev(clk_count_rundown_ar, n) );


  mean_ = mean(clk_count_rundown_ar, n);
  usart_printf("mean (%u) %.2f, ", n, mean_ );
  }

#endif
#if 0
  {
  static float means[ 10 ];
  assert(n <= ARRAY_SIZE(means));
  means[ i % n  ] = mean_;
  usart_printf("stddev_means(%u) %.2f ", n, stddev(means, n));
  }


  double v2 = (-6.0000e+00 * 1) + (4.6875e-02 * count_up) + ( -3.1250e-02 * count_down) + (-4.5475e-7 * mean_ );
  usart_printf("v %.7f, ", v2 );
#endif






#if 0

  char buf[10];
  usart_printf("whoot %s\n", format_bits( buf, 10,  (0xf & ~(1 << 3))  ));
  usart_printf("whoot %s\n", format_bits( buf, 10, 0xf ));
#endif


#if 0
  // encapsutate into a function.
  uint32_t t = 5 * 20000000;
  spi_reg_write(SPI1, REG_CLK_COUNT_INT_N_HI, (t >> 24) & 0xff );
  spi_reg_write(SPI1, REG_CLK_COUNT_INT_N_LO, t & 0xffffff  );
  spi_reg_write(SPI1, REG_USE_SLOW_RUNDOWN, 0 );
  // spi_reg_write(SPI1, REG_HIMUX_SEL, HIMUX_SEL_REF_LO );
  spi_reg_write(SPI1, REG_HIMUX_SEL, HIMUX_SEL_REF_HI );
#endif


#if 0
  uint32_t ret;

  ///////////////////////////////////////////
  // write the mux select
  // himux_sel = 4'b1101;     // ref i
  spi_reg_write(SPI1, REG_HIMUX_SEL , 0b1101 ); // doesn't work to set reg_himux_sel
  ret = spi_reg_read(SPI1, REG_HIMUX_SEL);
  assert(ret == 0b1101 );

  spi_reg_write(SPI1, REG_CLK_COUNT_INIT_N, 20000 ); // doesn't work to set reg_himux_sel
  ret = spi_reg_read(SPI1, REG_CLK_COUNT_INIT_N );
  assert(ret == 20000);

/*
  spi_reg_write(SPI1, REG_CLK_COUNT_INIT_N, 20000 ); // doesn't work to set reg_himux_sel
  ret = spi_reg_read(SPI1, REG_CLK_COUNT_INIT_N );
  assert(ret == 20000);
*/
#endif




