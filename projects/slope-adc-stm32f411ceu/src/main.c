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

#define REG_TEST              15
#define REG_RUNDOWN_DIR       16
#define REG_COUNT_FLIP        17


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








static void report_params(void )
{
  char buf[10];

  usart_printf("-------------\n");

  usart_printf("reg_led           %s\n", format_bits( buf, 4, spi_reg_read(SPI1, REG_LED )) );
  usart_printf("clk_count_init_n  %u\n", spi_reg_read(SPI1, REG_CLK_COUNT_INIT_N ) );

  usart_printf("clk_count_fix_n   %u\n", spi_reg_read(SPI1, REG_CLK_COUNT_FIX_N ) );
  usart_printf("clk_count_var_n   %u\n", spi_reg_read(SPI1, REG_CLK_COUNT_VAR_N ) );

  uint32_t int_lo = spi_reg_read(SPI1, REG_CLK_COUNT_INT_N_LO );
  uint32_t int_hi = spi_reg_read(SPI1, REG_CLK_COUNT_INT_N_HI );
  // usart_printf("clk_count_int_n_lo %u\n", int_lo );
  // usart_printf("clk_count_int_n_hi %u\n", int_hi );

  uint32_t int_n  = int_hi << 24 | int_lo;
  double period = int_n / (double ) 20000000;
  double nplc     = period / (1.0 / 50);

  usart_printf("clk_count_int_n   %u\n", int_n );
  usart_printf("period            %fs\n", period);
  usart_printf("nplc              %.2f\n", nplc);


  usart_printf("use_slow_rundown  %u\n", spi_reg_read(SPI1, REG_USE_SLOW_RUNDOWN) );

  //
  // char buf[100] char * format_bits(char *buf, size_t width, uint32_t value)
  usart_printf("himux_sel         %s\n", format_bits( buf, 4, spi_reg_read(SPI1, REG_HIMUX_SEL )) );
}


/*
  - ok. think we want an intermediate structure...
  so we can use this once

  - could also record the configuration
*/


struct Run
{
  uint32_t count_up;
  uint32_t count_down; 
  uint32_t clk_count_rundown; 


  uint32_t count_trans_up;
  uint32_t count_trans_down;

  // rundown_dir.
  uint32_t count_flip;
};

typedef struct Run  Run;


static void record_run( Run *run )
{
  assert(run); 

  // use separate lines (to make it easier to filter - for plugging into stats).
  run->count_up   = spi_reg_read(SPI1, REG_COUNT_UP );
  run->count_down = spi_reg_read(SPI1, REG_COUNT_DOWN );

  run->clk_count_rundown = spi_reg_read(SPI1, REG_CLK_COUNT_RUNDOWN );

  run->count_trans_up = spi_reg_read(SPI1, REG_COUNT_TRANS_UP ); 
  run->count_trans_down = spi_reg_read(SPI1, REG_COUNT_TRANS_DOWN );

  // rundown_dir.
  run->count_flip = spi_reg_read(SPI1, REG_COUNT_FLIP);
}




static void report_run( Run *run )
{
  assert(run);

  usart_printf("count_up %u, ",   run->count_up );
  usart_printf("count_down %u, ", run->count_down );
  usart_printf("clk_count_rundown %u, ", run->clk_count_rundown);
  // TODO fix this. just use a fixed array and modulo.
  usart_printf("trans_up/down %u %u, ", run->count_trans_up,  run->count_trans_down);
  usart_printf("count_flip %u, ",  run->count_flip);
  usart_printf("\n");
}




/*
  OK. there's an interesting thing. we get the dydr flag.
  But we can still transfer control from one loop - that eg. stores values. to a different loop. etc.
  So we don't necessarily have to make the interupt handler defer to a context, depending on what we want to do.
  eg. we can pass control off from a calibration loop to another loop.
*/

static void configure( uint32_t clk_count_int_n, bool use_slow_rundown, uint8_t himux_sel )
{
  // int configure

  // encapsutate into a function.
  // uint32_t t = 5 * 20000000;

  printf("configure %lu, %u, %u\n", clk_count_int_n,  use_slow_rundown, himux_sel );

  spi_reg_write(SPI1, REG_CLK_COUNT_INT_N_HI, (clk_count_int_n >> 24) & 0xff );
  spi_reg_write(SPI1, REG_CLK_COUNT_INT_N_LO, clk_count_int_n & 0xffffff  );
  spi_reg_write(SPI1, REG_USE_SLOW_RUNDOWN, use_slow_rundown );
  spi_reg_write(SPI1, REG_HIMUX_SEL, himux_sel );
}




static void cal_loop(app_t *app, MAT *x, MAT *y )
{
  // app argument is needed for data ready flag.
  // while loop has to be inner

  // rows x cols 
  unsigned row = 0;
  m_resize( x , 5, 3 );
  m_resize( y , 5, 1 );

  for(unsigned i = 0; i < 3; ++i )
  {
    double target;
    
    // switch integration configuration
    switch(i) {

      case 0:
        configure( 5 * 20000000, 1, HIMUX_SEL_REF_LO );
        target = 0.0;
        break;
      case 1:
        configure( 5 * 20000000, 1, HIMUX_SEL_REF_HI );
        target = 7.1;
        break;

      default:
        // we finished, getting all data
        // return here makes while loop simpler.
        usart_printf("done calibrating\n");
        return;
        break;

      // 2 case exits
    } // switch





    ////

    unsigned obs = 0;

    while(obs < 3) {

      // if we got data handle it.
      if(app->data_ready) {
        // in priority
        app->data_ready = false;

        Run run;
        record_run(&run );
        report_run(&run);

        ++obs;
        // store in matrix.
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


static void loop(app_t *app)
{
  /*
    loop() subsumes update()
  */


  // func();

  assert( HIMUX_SEL_REF_LO ==  0b1011  );


  configure( 5 * 20000000, 1, HIMUX_SEL_REF_LO );



  report_params();


  // TODO move to app_t structure?.
  static uint32_t soft_500ms = 0;

  while(true) {


    if(app->data_ready) {
      // in priority

      Run run;
      record_run(&run );
      report_run(&run);

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

#if 1
  // test ice40 register read/write
  // ok. seems to work.
  usart_printf("whoot\n");
  uint32_t ret;

/*
  spi_reg_xfer_24(SPI1, REG_LED, 0xffffff );
  ret = spi_reg_read(SPI1, REG_LED );
  ASSERT(ret == 0xffffff);
*/

  // fails... with new spi target, and deferred assignment

  spi_reg_write(SPI1, REG_LED , 0xff00ff);
  ret = spi_reg_read(SPI1, REG_LED);
  assert(ret == 0xff00ff);

  // this works... eg. allowing high bit to be off.
  spi_reg_write(SPI1, REG_LED, 0x7f00ff);
  ret = spi_reg_read(SPI1, REG_LED);
  assert(ret == 0x7f00ff);

  ///////////////////
  ret = spi_reg_read(SPI1, REG_TEST);
  usart_printf("reg 15 %u %x\n", ret, ret);
  assert(ret == 0xffffff );


  for(uint32_t i = 0; i < 32; ++i) {

    spi_reg_write(SPI1, REG_LED , i );
    ret = spi_reg_read(SPI1, REG_LED);
    assert(ret == i );
  }
#endif


  // state_change(&app, STATE_FIRST );

  // produces two return values. 
  MAT *x = m_get(1,1);
  MAT *y = m_get(1,1);

  cal_loop(&app, x, y );

  printf("x\n");
  m_foutput(stdout, x );

  printf("y\n");
  m_foutput(stdout, y );



  loop(&app);
}








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




