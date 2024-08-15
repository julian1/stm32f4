/*

  spi master and slave on the same device.

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

*/



#include <assert.h>
// #include <setjmp.h>
#include <stddef.h> // size_t
//#include <math.h> // nanf
//#include <stdio.h>
#include <string.h>   // memset
#include <stdio.h>   // putChar



#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>    // led
#include <libopencm3/stm32/spi.h>

// #include <libopencm3/cm3/nvic.h>
// #include <libopencm3/cm3/systick.h>

#include <libopencm3/usb/usbd.h>




#include <lib2/cbuffer.h>
#include <lib2/streams.h>
#include <lib2/cstring.h>
#include <lib2/usart.h>
#include <lib2/util.h>
// #include <cdcacm.h>


// #include "str.h"  //format_bits
// #include <format.h>  //format_bits



#define LED_PORT  GPIOA
// #define LED_OUT   GPIO9
#define LED_OUT   GPIO7   // aug, 2024.


static void led_on(void)
{
    gpio_set( LED_PORT, LED_OUT);
}


static void led_off(void)
{
    gpio_clear( LED_PORT, LED_OUT);
}


static void led_setup(void)
{
  // rcc_periph_clock_enable(RCC_GPIOA);
  gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_OUT);
  gpio_set_output_options(LED_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, LED_OUT);
}





#define APP_MAGIC   456


typedef struct app_t
{

  uint32_t  magic;
  bool      led_state ;     // for mcu. maybe change name to distinguish
  uint32_t  soft_500ms;

  volatile uint32_t system_millis;





  cbuf_t console_in;
  cbuf_t console_out;

  cstring_t     command;


  uint32_t    timer ;
  // usbd_device *usbd_dev ;

  uint32_t    freq;     // in Hz
  uint32_t    clk_deadtime; // in clock counts

} app_t;





static void app_systick_interupt(app_t *app)
{
  assert(app);
  assert(app->magic == APP_MAGIC);


  // interupt context. don't do anything compliicated here.

  ++ app->system_millis;
}










static void timer_set_frequency( uint32_t timer, uint32_t freq, uint32_t deadtime );


static void update_console_cmd(app_t *app)
{
  assert(app);
  assert(app->magic == APP_MAGIC);



  while( ! cbuf_is_empty(&app->console_in)) {

    // got a character
    int32_t ch = cbuf_pop(&app->console_in);
    assert(ch >= 0);

    if(ch != '\r' && cstring_count(&app->command) < cstring_reserve(&app->command) ) {
      // normal character
      cstring_push_back(&app->command, ch);
      // echo to output. required for minicom.
      putchar( ch);

    }  else {

      // newline or overflow
      putchar('\n');

      char *cmd = cstring_ptr(&app->command);

      // printf("cmd whoot is '%s'\n", cmd);


      uint32_t u0;

      if( sscanf(cmd, "freq %lu", &u0 ) == 1) {

        // 1uF + (10 + 3uH) == 44kHz resonant. avoid capacitive region.
        if(u0 >= 3 && u0 <= 100) { // 3 - 100 kHz
          app->freq = u0 * 1000;
          timer_set_frequency( app->timer, app->freq, app->clk_deadtime );
        } else {
          printf("freq out of range\n");
        }
      }

      if( sscanf(cmd, "deadtime %lu", &u0 ) == 1 || sscanf(cmd, "dead %lu", &u0 ) == 1) {

        if( /* u0 >= 0 && */ u0 <= 250  ) {
          app->clk_deadtime = u0 ;
          timer_set_frequency( app->timer, app->freq, app->clk_deadtime );
        } else {
          printf("deadtime out of range\n");
        }
      }

      // reset buffer
      cstring_clear( &app->command);

      // issue new command prompt
      printf("> ");
    }
  }
}









static void loop(app_t *app)
{
  assert(app);
  assert(app->magic == APP_MAGIC);


  /*
    loop() subsumes update()
  */

/*
  // TODO move to app_t structure?.
  static uint32_t soft_500ms = 0;
  static uint32_t soft_1000ms = 0;
*/
  while(true) {

		// usbd_poll(app->usbd_dev);

    update_console_cmd(app);

/*
    // usart1_enable_output_interupt(); // shouldn't be necessary
    if( (system_millis - soft_500ms) > 500) {
      soft_500ms += 500;
      led_toggle();
    }
*/

    // 500ms soft timer
    if( (app->system_millis - app->soft_500ms) > 500) {
      app->soft_500ms += 500;

      app->led_state = ! app->led_state;
      if(app->led_state)
        led_on();
      else
        led_off();

    }

#if 0
    // 500ms soft timer. should handle wrap around
    if( (system_millis - soft_1000ms) > 1000) {

      soft_1000ms += 1000;

#if 0
      static unsigned count = 0;

      printf("writing spi1 using cs2 \n" );
      spi_enable( SPI1 );
      // uint8_t val =  spi_xfer(SPI1, count % 2 == 0 ? 0xff : 0x00  ); // commmand.

      /*
      // this works. nice.
      // OK. reading back the last written value. works. but is QS1. high-z.  when strobe not set.
      uint8_t val =  spi_xfer(SPI1,  count );
      printf("count %d read value %d\n", count, val );
      */

      UNUSED(val);
      spi_disable( SPI1 );

      ++count;
#endif

    }
#endif

  }
}










#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>




static void timer_set_frequency( uint32_t timer, uint32_t freq, uint32_t deadtime )
{
  // assert(deadtime >= 1 /*&& deadtime <= 50 */);
  // assert(freq >= 40000 && freq <= 500000);
  // assert(freq >= 10000 && freq <= 500000);
  assert(freq >= 3600 && freq <= 100000);


  timer_disable_counter(timer);

  // uint32_t freq = 200 * 1000;               // in Hz.

  double clk_period = 2 / 84000000.f ;

  uint32_t period = (84000000.f / freq) / 2; // calculated.
  uint32_t half_period = period / 2;

  printf("------\n");
  printf("freq          %.1f kHz\n", freq / 1000.f );

  printf("clk period    %lu\n", period );
  printf("period        %.1f uS\n", period * clk_period  * 1000000 );

  printf("clk deadtime  %lu (%lu)\n", deadtime,  deadtime * 2 );


  double deadtime_ns = deadtime * clk_period * 1e9 ;
  printf("deadtime      %.0fnS  (%.0fnS)\n", deadtime_ns , deadtime_ns * 2  );

  printf("deadtime      %.1f %%\n", (deadtime * 2.f ) / period * 100);


  // timer_enable_break_main_output(timer);
  timer_set_period(timer, period );    // 42kHz

  /*
    - A lot of this can be set once. But keeping it in one place is clearer.
    - we could do this by just hardware jumpering. but we may want phase shifted full bridge.

    EXTR avoiding setting mode/and output here could reduce internal timer resetting stuff
    - but it seems to work.

  */

  timer_disable_counter(timer); // helps when resetting


  // 1 & 4 are the same
  timer_enable_oc_output(timer, TIM_OC1 );
  timer_set_oc_mode(timer, TIM_OC1 , TIM_OCM_PWM1);    // Output is active (high) when counter is less than output compare value
  timer_set_oc_value(timer, TIM_OC1, half_period - deadtime);

  timer_enable_oc_output(timer, TIM_OC4);
  timer_set_oc_mode(timer, TIM_OC4, TIM_OCM_PWM1);    // Output is active (high) when counter is less than output compare value
  timer_set_oc_value(timer, TIM_OC4, half_period - deadtime);


  // 2 & 3 are the same
  timer_enable_oc_output(timer, TIM_OC2);
  timer_set_oc_mode(timer, TIM_OC2, TIM_OCM_PWM2);    // Output is active (high) when counter is greater than output compare value
  timer_set_oc_value(timer, TIM_OC2, half_period + deadtime);

  timer_enable_oc_output(timer, TIM_OC3);
  timer_set_oc_mode(timer, TIM_OC3, TIM_OCM_PWM2);    // Output is active (high) when counter is greater than output compare value
  timer_set_oc_value(timer, TIM_OC3, half_period + deadtime);



/*

  EXTR - given the application - that calls for controllable phase - precise synch isn't needed.
      although would be nice.

  To 'synchronize two timers' with phase relationship.

    - issue - is the synchronization of the enable.  not the actual input clk.
    - just set the counter if necessry

    practially it probably doesn't matter - if start slightly out of phase. due to calls to the the peripheral from  c.
    (eg. set_counter() or enable().)
    but would be nice if there was a ways to start, so that

    just do something like this,

        TIM_CR1(TIM1) |= TIM_CR1_CEN;
        TIM_CR1(TIM9) |= TIM_CR1_CEN;

    with counter start adjustment.


    void timer_enable_counter(uint32_t timer_peripheral)
    {
        TIM_CR1(timer_peripheral) |= TIM_CR1_CEN;
    }

    https://community.st.com/t5/stm32-mcus-products/synchronize-two-timers-with-each-other/td-p/652168
       Three options
          You can access the registers directly to enable them. Will cut down on delay.

          You can set the CNT of one to be a few counts in front of the other one so
          that when it starts a little delayed, the counters match. Will eliminate delay,
          but offset will depend on compiler settings.

          You can drive both timer with a master timer. Will be the most complex code,
          but will eliminate the delay.



  jul 9, 2024.
  // PSFB - would look more like this
      - with RHB put on TIM1.
      doesn't look like TIM9 can do up and downcount

  // LHB
  timer_enable_oc_output(timer, TIM_OC1 );
  timer_set_oc_mode(timer, TIM_OC1 , TIM_OCM_PWM1);    // Output is active (high) when counter is less than output compare value
  timer_set_oc_value(timer, TIM_OC1, half_period - deadtime);

  timer_enable_oc_output(timer, TIM_OC2);
  timer_set_oc_mode(timer, TIM_OC2, TIM_OCM_PWM2);    // Output is active (high) when counter is greater than output compare value
  timer_set_oc_value(timer, TIM_OC2, half_period + deadtime);

  /////////////////

  // RHB
  timer_enable_oc_output(timer, TIM_OC3);
  timer_set_oc_mode(timer, TIM_OC3, TIM_OCM_PWM2);    // Output is active (high) when counter is greater than output compare value
  timer_set_oc_value(timer, TIM_OC3, half_period + deadtime);

  timer_enable_oc_output(timer, TIM_OC4);
  timer_set_oc_mode(timer, TIM_OC4, TIM_OCM_PWM1);    // Output is active (high) when counter is less than output compare value
  timer_set_oc_value(timer, TIM_OC4, half_period - deadtime);
*/



  timer_set_counter( timer, 0 );    // make sure timer count does  not escape when shortening period

  timer_enable_counter(timer);  // seems to need this


}



static void timer_port_setup(void )
{
  printf("timer port setup\n");

  uint16_t outputs = GPIO0 | GPIO1 | GPIO2 | GPIO3;

  //  port set up for alt function.
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, outputs);
  gpio_set_af(GPIOA, GPIO_AF2, outputs );
  gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, outputs);



}

static void timer_setup( uint32_t timer )
{
  // HMMMMM...

  printf("timer setup\n");

  // uint32_t timer = TIM5;
  assert( timer == TIM5 );

  rcc_periph_reset_pulse( RST_TIM5 );   // is this needed

  timer_set_prescaler(timer, 0 );  // No prescaler = 42Mhz.


/*
    - TIM_CR1_CMS_CENTER_1  Center mode 1: counter counts up and down alternatively (interrupts on counting down)
      see,
      https://bdebyl.net/post/stm32-part1/
      https://community.st.com/s/question/0D50X00009XkXePSAV/how-to-configure-the-starting-direction-of-centeraligned-timer

      TIM_OCM_PWM1  Output is active (high) when counter is less than output compare value
      TIM_OCM_PWM2  Output is active (high) when counter is greater than output compare value
      ----

      oc1 = fet1
      oc2 = fet2

      1&4 on. then 2,3 on.

      - we kind of want to check that mcu stays powered on, under power. before refine too much.
      - perhaps want ctrl over frequency...
    ---------
    no. pres
    ------------

    having a fixed deadtime time is interesting - because it increases relative to period, as freq increases which trades off power. which is what we want.
    eg. at 200kHz. and dead=50, it's about 50%.
*/


  timer_set_mode(timer, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_CENTER_1, TIM_CR1_DIR_UP);  // alternating up/down


/*
  timer_set_frequency( timer, freq, 200000 );
  timer_enable_counter(timer);
*/
}















static char buf_console_in[1000];
static char buf_console_out[1000];    // changing this and it freezes. indicates. bug


static char buf_command[1000];




static void app_init_console_buffers( app_t *app )
{
  assert(app);
  assert(app->magic == APP_MAGIC);


  /* note no printf yet.
      avoid assert()
  */

  // uart/console
  cbuf_init(&app->console_in,  buf_console_in, sizeof(buf_console_in));
  cbuf_init(&app->console_out, buf_console_out, sizeof(buf_console_out));

  cbuf_init_stdout_streams(  &app->console_out );
  cbuf_init_stdin_streams( &app->console_in );


  cstring_init(&app->command, buf_command, buf_command + sizeof( buf_command));
}





static app_t app = {

  .magic = APP_MAGIC,

};








int main(void)
{

  // fast hse needed for usb
	// rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ] );  // stm32f411  upto 100MHz. works stm32f407 too.
  // rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ] );  // stm32f407


  rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_3V3_84MHZ ]);    // HSI WORKS stm32f410, may 2022.

  /*
  // http://libopencm3.org/docs/latest/stm32f4/html/f4_2rcc_8h.html

    see here for rcc.c example, defining 100MHz, using 25MHz xtal.
      https://github.com/insane-adding-machines/unicore-mx/blob/master/lib/stm32/f4/rcc.c
  */
  // 16MHz. for hsi from datasheet.

  // systick_setup(16000);
  // systick_setup(16000);
  systick_setup(84000);  // 84MHz.
  // systick_setup(168000);



  // clocks
  rcc_periph_clock_enable(RCC_SYSCFG); // maybe required for external interupts?

  // LED
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOB);
  // rcc_periph_clock_enable(RCC_GPIOE);

  // USART
  rcc_periph_clock_enable(RCC_GPIOB); // F410/f411
  rcc_periph_clock_enable(RCC_USART1);


  // USB
	// rcc_periph_clock_enable(RCC_OTGFS);



  // spi1
  // rcc_periph_clock_enable(RCC_SPI1);

  // spi2
  // rcc_periph_clock_enable(RCC_SPI2);


  //////////////////////
  // setup




  // mcu clock
  systick_setup(84000); // 84MHz.
  // systick_setup(12000); // 12MHz. default lsi.
  systick_handler_set( (void (*)(void *)) app_systick_interupt, &app );  // rename systick_handler_set()



  // led blink
  // led_setup(LED_PORT, LED_OUT);
  led_setup();


  // setup external state for critical error led blink in priority
  // because assert() cannot pass a context
  assert_critical_error_led_setup( LED_PORT, LED_OUT);



  app_init_console_buffers( &app );

  /*
    dont' think it makes any sense to use a circular buffer on the output.
    instead just block control until the output is flushed.
  */
  //////////////
  // now can init usart peripheral using app console buffers
  usart1_setup_portB();

  usart1_set_buffers( &app.console_in, &app.console_out);



#if 0
  memset(&app, 0, sizeof(app_t));

  // uart/console
  cbuf_init(&app.console_in,  buf_console_in, sizeof(buf_console_in));
  cbuf_init(&app.console_out, buf_console_out, sizeof(buf_console_out));


  cstring_init(&app.command, buf_command, buf_command + sizeof( buf_command));


  // standard streams for printf, fprintf, putc.
  // cbuf_init_std_streams( &app.console_out );

  // standard streams for printf, fprintf, putc.
  cbuf_init_stdout_streams(  &app.console_out );
  // for fread, fgetch etc
  cbuf_init_stdin_streams( &app.console_in );

#endif

#if 0

  //////////////
  // initialize usart before start all the app constructors, so that can print.
  // uart
  // usart1_setup_gpio_portA();
  usart1_setup_gpio_portB();

  usart1_set_buffers(&app.console_in, &app.console_out);
#endif


  printf("\n--------\n");
  printf("addr main() %p\n", main );



#if 0
  ////////////////////////////
  // usb
  // might be better to pass as handler?
	app.usbd_dev = usb_setup();
  assert(app.usbd_dev);
#endif


  printf("\n--------");
  printf("\nstarting\n");


  assert( sizeof(bool) == 1);
  assert( sizeof(float) == 4);
  assert( sizeof(double ) == 8);


  rcc_periph_clock_enable(RCC_TIM5);


  app.timer = TIM5;
  timer_port_setup();
  timer_setup( app.timer );

  app.freq = 15000;     // 15kHz.
  // app.clk_deadtime = 15; // 15 == 357ns.
  // app.clk_deadtime = 25;    // 25 == 595ns , (1190ns) .
  // app.clk_deadtime = 50;    // with 1k. gate resistors.  works better.
  app.clk_deadtime = 70;       // for full-bridge.  time for primary to reset.

  timer_set_frequency( app.timer, app.freq, app.clk_deadtime );
  timer_enable_counter(app.timer);


  loop(&app);
}

// put the code after the function. to prevent inlining.





/*
  // timer is up counting.
  timer_set_mode(timer, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
  // timer_set_mode(timer, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_CENTER_1, TIM_CR1_DIR_UP);
  timer_enable_preload(timer);
  timer_enable_break_main_output(timer); // what does this do
  timer_set_period(timer, 1000);

  ////////
  // configure the channnel outputs to toggle on the oc (output compare) value

  // channel 1
  timer_set_oc_mode(timer, TIM_OC1, TIM_OCM_TOGGLE);
  timer_enable_oc_output(timer, TIM_OC1);
  timer_set_oc_value(timer, TIM_OC1, 500);


  timer_enable_counter(timer);
*/
