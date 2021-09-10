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

#include "spi1.h"
#include "ice40.h"



typedef struct app_t
{
  CBuf console_in;
  CBuf console_out;

  bool data_ready ;

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


5 sec integration time at 20MHz = 100M count period.
(5000 * 10000 + 5000 * 10000) / 20MHz =  5 sec.
10kcount/20MHz = 500uS. phase.

count_up 5000      count_down 5000    count_rundown 6717
count_up 5000      count_down 5000    count_rundown 6718
count_up 5000      count_down 5000    count_rundown 6718
count_up 5000      count_down 5000    count_rundown 6721
count_up 5000      count_down 5000    count_rundown 6715
count_up 5000      count_down 5000    count_rundown 6721
count_up 5000      count_down 5000    count_rundown 6720
count_up 5000      count_down 5000    count_rundown 6720
count_up 4999      count_down 5001    count_rundown 6721
count_up 5000      count_down 5000    count_rundown 6720

reg 8 207
reg 8 206
reg 8 207
reg 8 208
reg 8 209
reg 8 209
reg 8 207

warm start - eg. flash fpga .
count_up 0      count_down 0    count_rundown 0
count_up 4999      count_down 5001    count_rundown 1267
count_up 5000      count_down 5000    count_rundown 6213
count_up 5000      count_down 5000    count_rundown 6643
count_up 5000      count_down 5000    count_rundown 6671
count_up 5000      count_down 5000    count_rundown 6678
count_up 5000      count_down 5000    count_rundown 6682
count_up 5000      count_down 5000    count_rundown 6684
count_up 4999      count_down 5000    count_rundown 6680
count_up 5000      count_down 5000    count_rundown 6683
count_up 5000      count_down 5000    count_rundown 6684
count_up 5000      count_down 5000    count_rundown 6685
count_up 5000      count_down 5000    count_rundown 6683
count_up 5000      count_down 5000    count_rundown 6683
count_up 5000      count_down 5000    count_rundown 6682
count_up 5000      count_down 5000    count_rundown 6683
count_up 5000      count_down 5000    count_rundown 6684

takes a while to converge. even if ref warm. so perhaps DA of caps. or TC of switch.

count_up 5125,   count_down 4876  rundown 3196     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3196     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3196     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3197     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3198     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3196     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3198     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3198     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3198     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3195     trans_up 5001    trans_down 5001

----------
switch from mlcc to vishay c0g.  actually worse... although board started from cold.
count_up 5125,   count_down 4876  rundown 3458     trans_up 5001    trans_down 5001
count_up 5126,   count_down 4875  rundown 3447     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3433     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3427     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3422     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3417     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3413     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3407     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3403     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3401     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3393     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3392     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3387     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3385     trans_up 5001    trans_down 5001

vishay c0g. warm start (eg. just flash ice40., much better.

count_up 5124,   count_down 4877  rundown 922     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3287     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3288     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3288     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3289     trans_up 5001    trans_down 5001

So there's a strong TC with board temperature.
yes. starts great.

count_up 0,   count_down 0  rundown 0     trans_up 0    trans_down 0
count_up 5124,   count_down 4877  rundown 924     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3287     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3289     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3289     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3289     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3286     trans_up 5001    trans_down 5001
count_up 5125,   count_down 4876  rundown 3287     trans_up 5001    trans_down 5001

get a syringe with isopropyl.

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

      usart_printf("count_up %u,   ", spi_reg_read_24(SPI1, 9 ));
      usart_printf("count_down %u  ", spi_reg_read_24(SPI1, 10 ));
      usart_printf("rundown %u     ", spi_reg_read_24(SPI1, 11 ));


      usart_printf("trans_up %u    ", spi_reg_read_24(SPI1, 12 ));
      usart_printf("trans_down %u  ", spi_reg_read_24(SPI1, 14 ));

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


static app_t app;


int main(void)
{
  // hsi setup high speed internal!!!
  // TODO. not using.


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
  systick_setup(16000);

  // led
  led_setup();


  memset(&app, 0, sizeof(app_t));

  ///////
  // uart/console
  cBufInit(&app.console_in,  buf_console_in, sizeof(buf_console_in));
  cBufInit(&app.console_out, buf_console_out, sizeof(buf_console_out));

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
  spi1_special_gpio_setup();
  // adc interupt...
  spi1_interupt_gpio_setup( (void (*) (void *))spi1_interupt, &app);


  spi_ice40_setup(SPI1);




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
  ret = spi_reg_read_24(SPI1, 7);
  ASSERT(ret == 0xffffff);
*/

  // fails... with new spi target, and deferred assignment
  spi_reg_xfer_24(SPI1, 7, 0xff00ff );
  ret = spi_reg_read_24(SPI1, 7);
  ASSERT(ret == 0xff00ff);

  // this works... eg. allowing high bit to be off.
  spi_reg_xfer_24(SPI1, 7, 0x7f00ff );
  ret = spi_reg_read_24(SPI1, 7);
  ASSERT(ret == 0x7f00ff);

  ///////////////////
  ret = spi_reg_read_24(SPI1, 15);
  usart_printf("reg 15 %u %x\n", ret, ret);
  ASSERT(ret == 0xffffff );



  for(uint32_t i = 0; i < 32; ++i) {
    spi_reg_xfer_24(SPI1, 7, i );
    ret = spi_reg_read_24(SPI1, 7);
    ASSERT(ret == i );
  }
#endif


  // state_change(&app, STATE_FIRST );

  loop(&app);
}


