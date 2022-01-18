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


#include "cbuffer.h"
#include "usart2.h"
#include "util.h"
#include "assert.h"


#include "fbuffer.h"
#include "stats.h"




#include "spi1.h"
#include "ice40.h"


// spi
#define REG_LED  7





typedef struct app_t
{
  CBuf console_in;
  CBuf console_out;

  bool data_ready ;


  FBuf      measure_rundown;



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


static void loop(app_t *app)
{
  /*
    loop() subsumes update()
  */

  // TODO move to app_t structure?.
  static uint32_t soft_500ms = 0;

  while(true) {


    if(app->data_ready) {
      // in priority

      usart_printf("count_up/down %u %u, ", spi_reg_read(SPI1, 9 ), spi_reg_read(SPI1, 10 ));
      // usart_printf("count_down %u  ", spi_reg_read(SPI1, 10 ));

      uint32_t count_rundown = spi_reg_read(SPI1, 11 );
      usart_printf("clk_count_rundown %u, ", count_rundown);

      // TODO fix this. just use a fixed array and modulo.

      // push value onto circ buffer. taking care pop last value off...
      size_t sz  = fBufCount(&app->measure_rundown);
      // usart_printf("size %u ", sz);
      if(sz == fBufReserve(&app->measure_rundown) - 1)
        fBufPop(&app->measure_rundown);
      fBufPush(&app->measure_rundown, (float)count_rundown);



      usart_printf("trans_up/down %u %u, ", spi_reg_read(SPI1, 12 ),  spi_reg_read(SPI1, 14 ));
      // usart_printf("trans_down %u  ", spi_reg_read(SPI1, 14 ));

      usart_printf("rundown_dir %u, ", spi_reg_read(SPI1, 16 ));
      usart_printf("flip %u, ",        spi_reg_read(SPI1, 17 ));

      ////////////////////////
      ///////// stats



#if 1

      // usart_printf("\n");
      float vs[10];

      // Copy empties i think.
      size_t n = fBufCopy2(&app->measure_rundown, vs, ARRAY_SIZE(vs));
      // ASSERT(n >= 1);
      // for(size_t i = 0; i < n; ++i) {
        // usart_printf(" %f ", vs[i] );
      // }
      /* TODO a single stats core function that computes all of these
      */
      // float vmean = mean(vs, n);
      usart_printf("stddev_rundown(%u) %.2f, ", n, stddev(vs, n) );

      float mean_ = mean(vs, n);
      usart_printf("mean (%u) %.2f, ", n, mean_ );

#if 1
      // do we want to push the mean into a structure as well.
    /*
      reserve a max array.
      but then use a local variable for n for modulus.
      this makes it customizable/changeable at runtime.
      use assert
    */

      static float means[ 10 ];


      static int i = 0;

      size_t nn = 5;
      ASSERT(nn <= ARRAY_SIZE(means));


      means[ i++ % nn  ] = mean_;

      usart_printf("stddev_means(%u) %.2f ", nn, stddev(means, nn  ));
#endif

    /////////////////////
#endif


      usart_printf("\n");



      app->data_ready = false;
    }


    update_console_cmd(app);

    // usart_output_update(); // shouldn't be necessary


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

static float buf_rundown[6];

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
  fBufInit(&app.measure_rundown, buf_rundown, ARRAY_SIZE(buf_rundown));


  //////////////
  // uart
  // usart_setup_gpio_portA();
  usart_setup_gpio_portB();

  usart_set_buffers(&app.console_in, &app.console_out);

  // setup print
  // usart_printf_set_buffer()
  usart_printf_init(&app.console_out);


  ////////////////
  // spi1/ice40
  spi1_port_setup();
#if 0
  spi1_special_gpio_setup();
#endif

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
  spi_reg_xfer_24(SPI1, 7, 0xffffff );
  ret = spi_reg_read(SPI1, 7);
  ASSERT(ret == 0xffffff);
*/

  // fails... with new spi target, and deferred assignment

  spi_reg_write(SPI1, 7 , 0xff00ff);
  ret = spi_reg_read(SPI1, 7);
  ASSERT(ret == 0xff00ff);

  // this works... eg. allowing high bit to be off.
  spi_reg_write(SPI1, 7 , 0x7f00ff);
  ret = spi_reg_read(SPI1, 7);
  ASSERT(ret == 0x7f00ff);

  ///////////////////
  ret = spi_reg_read(SPI1, 15);
  usart_printf("reg 15 %u %x\n", ret, ret);
  ASSERT(ret == 0xffffff );


  for(uint32_t i = 0; i < 32; ++i) {

    spi_reg_write(SPI1, 7 , i );
    ret = spi_reg_read(SPI1, 7);
    ASSERT(ret == i );
  }
#endif


  // state_change(&app, STATE_FIRST );

  loop(&app);
}


