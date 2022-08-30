/*
  nix-shell ~/devel/nixos-config/examples/arm.nix
  rlwrap -a picocom -b 115200 /dev/ttyUSB0

  nix-shell ~/devel/nixos-config/examples/arm.nix
  make

  nix-shell ~/devel/nixos-config/examples/arm.nix
  cd smu11
  openocd -f ../../openocd.cfg

  nix-shell ~/devel/nixos-config/examples/arm.nix
  rlwrap nc localhost 4444
  reset halt ; flash write_image erase unlock ./main.elf; sleep 1500; reset run


// vim :colorscheme default. loooks good.

// cjmcu  stm32f407.
// issue. is that board/stlink doesn't appear to reset cleanly. needs sleep.

// - can plug a thermocouple straight into the sense +p, +n, and
// then use the slow (digital) integrator/pid loop - as a temperature controler..


*/

#include <libopencm3/stm32/rcc.h>   // clock
#include <libopencm3/stm32/gpio.h>    // led
#include <libopencm3/stm32/spi.h>   // SPI1

#include <libopencm3/cm3/scb.h>  // reset()


#include <stdio.h>    // printf
#include <string.h>   // strcmp, memset


// library code
#include "usart.h"
#include "assert.h"
// #include "cbuffer.h"
// #include "cstring.h"
#include "streams.h"
#include "util.h"


#include "spi1.h"
#include "mux.h"   // to blink the led.
#include "reg.h"   // to blink the led.
#include "spi-ice40.h" // to blink the led.



#include "fbuffer.h"

#include "format.h"   // format_bits()



// app structure
#include "app.h"

#include "dac8734.h"




static void spi1_interupt(app_t *app)
{
  /*
    interupt context. avoid doing work here...
  */
  if(app->adc_drdy == true) {
    // still flagged from last time, then code is too slow, and we missed an adc read
    ++app->adc_drdy_missed;
  }

  // set adc_drdy flag so that update() knows to read the adc...
  app->adc_drdy = true;
}



static void update_soft_1s(app_t *app)
{
  // maybe review this...
  UNUSED(app);


}

/*
    spi must be setup, in order for led toggle...
*/

static void update_soft_500ms(app_t *app)
{
  UNUSED(app);

  static bool led_state = 0;
  led_state = ! led_state;

  // blink the fpga led
  mux_ice40(app->spi);

  // ice40_reg_toggle(app->spi, REG_LED, LED1);
  led_state ? ice40_reg_set(app->spi, REG_LED, LED2)
        : ice40_reg_clear(app->spi, REG_LED, LED2);

  // small problem. that when ice40 is powered down we get high signal.
  // So for rails we should check high bits are 0.


  // blink stm32/mcu led
  // led_toggle();
  led_set( led_state );


/*
  uint8_t val = ice40_reg_read( app->spi, REG_LED );
  // printf("  val %u", val );
  printf("reg_led read bits %s\n", format_bits(buf, 4, val) );
*/


  // if(app->rails_print) {
#if 0
  char buf[100];
  uint8_t val = ice40_reg_read( app->spi, REG_MON_RAILS );
  printf("reg_mon_rails read bits %s\n", format_bits(buf, 4, val) );

#endif
  // }



/*
  // try w25 chip
  mux_w25(app->spi);
  msleep(20);
  spi_w25_get_data(app->spi);
*/


}






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

      // printf("cmd whoot is '%s'\n", cmd);


      uint32_t u0;
      UNUSED(u0);

      if( strcmp(cmd, "test") == 0) {

        printf("got test\n");
      }

      else if(strcmp(cmd, "reset mcu") == 0) {
        // reset stm32f4
        // scb_reset_core()
        scb_reset_system();
      }



      else if(strcmp(cmd, "mon") == 0) {

        // this will powerdown rails
        mux_ice40(app->spi);
        uint8_t val = ice40_reg_read( app->spi, REG_MON_RAILS );
        char buf[10];
        printf("reg_mon_rails read bits %s\n", format_bits(buf, 4, val) );

      }






      else if(strcmp(cmd, "reset") == 0) {

        // this will powerdown rails
        printf("reset ice40\n");
        mux_ice40(app->spi);
        ice40_reg_set(app->spi, CORE_SOFT_RST, 0 );
      }


      else if(strcmp(cmd, "powerup") == 0) {

        // ok without dac being initialized?
        // powerup 5 and +-15V rails and set analog switches

        printf("powerup ice40\n");
        mux_ice40(app->spi);

        // check rails monitor.
        uint8_t val = ice40_reg_read( app->spi, REG_MON_RAILS );
        // char buf[10];
        // printf("reg_mon_rails read bits %s\n", format_bits(buf, 4, val) );

        if(val == 1) {
          ice40_reg_set(app->spi, 6 , 0 );
        }  else {
          printf("problem with rails monitor\n");
        }
      }

#if 0


      /*
          OK. putting 3.3V on the dg444 input pins  and it sinks through ESD diodes of the dg444. to one of the rails.
          - if +-15V are up (even if lp5v down), then it's ok.
              makes sense from datasheet schematic of input of dg444.
          - if +-15V disconnected at bench supply. still have problem.
          ---

      */
      else if( sscanf(cmd, "dac_ref_mux %lu", &u0 ) == 1) {
        // OK. this works. and dg444 doesn't lock up.
        mux_ice40(app->spi);
        ice40_reg_clear(app->spi, REG_RAILS_OE, RAILS_OE);  // active lo
        if(u0) {
            printf("turn on dac_ref_mux\n" );
            ice40_reg_write(app->spi, REG_DAC_REF_MUX, DAC_REF_MUX_A | DAC_REF_MUX_B );   // active lo

          } else {
            printf("turn off dac_ref_mux\n" );
            ice40_reg_write(app->spi, REG_DAC_REF_MUX, ~(DAC_REF_MUX_A | DAC_REF_MUX_B)); // active lo

          }
      }

      else if( sscanf(cmd, "rails %lu", &u0 ) == 1) {
        // OK. this works. and dg444 doesn't lock up.
        mux_ice40(app->spi);
        ice40_reg_clear(app->spi, REG_RAILS_OE, RAILS_OE);  // active lo
        if(u0) {
            printf("turn on both lp5v and lp5v rails\n" );
            ice40_reg_write(app->spi, REG_RAILS, RAILS_LP5V | RAILS_LP15V);
          } else {
            printf("turn off both lp5v and lp5v rails\n" );
            ice40_reg_write(app->spi, REG_RAILS, ~( RAILS_LP5V | RAILS_LP15V ) );
          }
      }

      else if( sscanf(cmd, "lp5v %lu", &u0 ) == 1) {
        mux_ice40(app->spi);
        ice40_reg_clear(app->spi, REG_RAILS_OE, RAILS_OE);  // active lo
        if(u0) {
            printf("turn on lp5v rails\n" );
            ice40_reg_set(app->spi, REG_RAILS, RAILS_LP5V);
          } else {
            printf("turn off lp5v rails\n" );
            ice40_reg_clear(app->spi, REG_RAILS, RAILS_LP5V);
          }
      }
#endif

#if 1

      /* this is dangerous, because of dependencies. should remove after initial testing.
          beshould test rails are up, first.
      */

      else if( sscanf(cmd, "lp15v %lu", &u0 ) == 1) {
        mux_ice40(app->spi);
        ice40_reg_clear(app->spi, REG_RAILS_OE, RAILS_OE);
        if(u0) {
            printf("turn on lp15v rails\n" );
            ice40_reg_set(app->spi, REG_RAILS, RAILS_LP15V);
          } else {
            // TODO remove this. it is dangerous
            printf("turn off lp15v rails\n" );
            ice40_reg_clear(app->spi, REG_RAILS, RAILS_LP15V);
          }
      }
      else if( sscanf(cmd, "lp24v %lu", &u0 ) == 1) {
        mux_ice40(app->spi);
        ice40_reg_clear(app->spi, REG_RAILS_OE, RAILS_OE);
        if(u0) {
            printf("turn on lp24v rails\n" );
            ice40_reg_set(app->spi, REG_RAILS, RAILS_LP24V );
          } else {
            printf("turn off lp24v rails\n" );
            ice40_reg_clear(app->spi, REG_RAILS, RAILS_LP24V );
          }
      }
      else if( sscanf(cmd, "lp50v %lu", &u0 ) == 1) {
        mux_ice40(app->spi);
        ice40_reg_clear(app->spi, REG_RAILS_OE, RAILS_OE);
        if(u0) {
            printf("turn on lp50v rails\n" );
            ice40_reg_set(app->spi, REG_RAILS, RAILS_LP50V );
          } else {
            printf("turn off lp50v rails\n" );
            ice40_reg_clear(app->spi, REG_RAILS, RAILS_LP50V );
          }
      }

      else if( strcmp(cmd, "dac init") == 0) {
        // dac init
        int ret = dac_init(app->spi, REG_DAC); // bad name?
        if(ret != 0) {
          printf("failed\n" );
          // state_change(app, STATE_HALT);
          return;
        }
      }
#endif

      else if( strcmp(cmd, "on") == 0) {  // output on

        ice40_reg_set(app->spi, REG_RELAY_OUT, REG_RELAY_OUT_COM_HC_CTL);
        ice40_reg_set(app->spi, REG_LED, LED1);
      }
      else if( strcmp(cmd, "off") == 0) {  // output off

        ice40_reg_clear(app->spi, REG_RELAY_OUT, REG_RELAY_OUT_COM_HC_CTL);
        ice40_reg_clear(app->spi, REG_LED, LED1);
      }

      else if( strcmp(cmd, "guard on") == 0) {  // output on

        ice40_reg_set(app->spi, REG_RELAY_OUT, REG_RELAY_GUARD_CTL);
      }
      else if( strcmp(cmd, "guard off") == 0) {  // output off

        ice40_reg_clear(app->spi, REG_RELAY_OUT, REG_RELAY_GUARD_CTL);
      }


      else if( sscanf(cmd, "sense ext %lu", &u0 ) == 1) {

        if(u0) {
          ice40_reg_clear(app->spi, REG_RELAY_OUT, REG_RELAY_SENSE_INT_CTL);    // FIXME. write not set // perhaps push to own register to make exclusive.
          ice40_reg_set(app->spi, REG_RELAY_OUT, REG_RELAY_SENSE_EXT_CTL);
        } else {
          ice40_reg_set(app->spi, REG_RELAY_OUT, REG_RELAY_SENSE_INT_CTL);
          ice40_reg_clear(app->spi, REG_RELAY_OUT, REG_RELAY_SENSE_EXT_CTL);
        }

      }






      else if( strcmp(cmd, "relay on") == 0) {  // output on

        // ice40_reg_set(app->spi, REG_RELAY_OUT, REG_RELAY_SENSE_INT_CTL);
        // ice40_reg_set(app->spi, REG_RELAY_OUT, REG_RELAY_SENSE_EXT_CTL);

        ice40_reg_set(app->spi, REG_RELAY_COM, RELAY_COM_X_CTL);
        ice40_reg_set(app->spi, REG_LED, LED1);
      }
      else if( strcmp(cmd, "relay off") == 0) {  // output off

        // ice40_reg_clear(app->spi, REG_RELAY_OUT, REG_RELAY_SENSE_INT_CTL);
        // ice40_reg_clear(app->spi, REG_RELAY_OUT, REG_RELAY_SENSE_EXT_CTL);

        ice40_reg_clear(app->spi, REG_RELAY_COM, RELAY_COM_X_CTL);

        ice40_reg_clear(app->spi, REG_LED, LED1);
      }





      // chage name to start, init sounds like initial-condition

      else if( strcmp(cmd, "start") == 0) {

        app_start2( app );
      }

      else if( strcmp( cmd , "") == 0) {

      }

      else {

            printf("unknown '%s'\n", cmd );

      }



      // reset buffer
      cStringClear( &app->command);

      // issue new command prompt
      printf("> ");
    }
  }
}
















/*
  is there something taking a really long time? async flush?
  OR are we blocking???? in the flush?

  such that we miss... the adc read??


*/



static void loop(app_t *app)
{
  // move this into the app var structure ?.
  static uint32_t soft_500ms = 0;
  static uint32_t soft_1s = 0;

  /*
    Think all of this should be done/moved to update()...

  */
  while(true) {


    // EXTREME - could actually call update at any time, in a yield()...
    // so long as we wrap calls with a mechanism to avoid stack reentrancy
    // led_update(); in systick.
    // but better just to flush() cocnsole queues.conin/out

//    update(app);

     /*
        JA
        we always want to update console first. so that we can always issue commands.
        and know that we have control
        -----------------

        All of this code needs to be refactored. so that the the command dispatch happens in the top-level loop.
    */

    update_console_cmd(app);


    // 500ms soft timer. should handle wrap around
    if( (system_millis - soft_500ms) > 500) {
      soft_500ms += 500;
      update_soft_500ms(app);
    }

    if( (system_millis - soft_1s) > 1000 ) {

      // THIS IS FUNNY....
      soft_1s += 1000;
      update_soft_1s(app);
    }

#if 0
    if(app->state == STATE_FIRST) {

      // rename ca
      state_change(app, STATE_ANALOG_UP);
    }
#endif


  }
}




#if 0

static void assert_app(app_t *app, const char *file, int line, const char *func, const char *expr)
{
  /*
    note the usart tx interupt will continue to flush output buffer,
    even after jump to critical_error_blink()
  */
  printf("\nassert_app failed %s: %d: %s: '%s'\n", file, line, func, expr);

  state_change(app, STATE_HALT );

#if 1
  // we have to do a critical error here... else caller code will just progress,
  // if we were in a state transition, then it will continue to just progress...
  critical_error_blink();
#endif
}

#endif








/////////////////////////
/*
  TODO.
  Maybe move raw buffers into app structure?


  Why not put on the stack? in app_t ?
*/

static char buf_console_in[1000];
static char buf_console_out[1000];

// static char buf_cmds[1000];

static char buf_command[1000];



static float buf_vfb_measure[100];
static float buf_ifb_measure[100];


static float buf_vfb_range[100];
static float buf_ifb_range[100];





// move init to a function?
// no... because we collect/assemble dependencies. ok in main()
static app_t app;



/*
  TODO.
  OK. it would be very nice to know how many values are in the
  float circular buffer...

  Rather than counting interupts in a separate var .

*/












int main(void)
{
  // high speed internal!!!
  // TODO. not using.

	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ] );  // stm32f411  upto 100MHz.

  // this is the mcu clock.  not the adc clock. or the fpga clock.
  // systick_setup(16000);
  systick_setup(84000);  // 84MHz.

  // clocks
  rcc_periph_clock_enable(RCC_SYSCFG); // maybe required for external interupts?

  // LED
  // rcc_periph_clock_enable(RCC_GPIOE);
  rcc_periph_clock_enable(RCC_GPIOA);

  // USART
  rcc_periph_clock_enable(RCC_GPIOB); // F410 / f411
  rcc_periph_clock_enable(RCC_USART1);


  // spi / ice40
  rcc_periph_clock_enable(RCC_SPI1);

  /*
    Do led first, even if need update() and systick loop() to blink it.
  */

#define LED_PORT  GPIOA
#define LED_OUT   GPIO9

  led_setup(LED_PORT, LED_OUT);




  //////////////////////
  // main app setup

  memset(&app, 0, sizeof(app_t));


  // uart/console
  cBufInit(&app.console_in,  buf_console_in, sizeof(buf_console_in));
  cBufInit(&app.console_out, buf_console_out, sizeof(buf_console_out));

  cbuf_init_stdout_streams(  &app.console_out );
  cbuf_init_stdin_streams( &app.console_in );


  cStringInit(&app.command, buf_command, buf_command + sizeof( buf_command));

  //////////////
  // initialize usart before start all the app constructors, so that can print.
  // uart
  // usart1_setup_gpio_portA();
  usart1_setup_gpio_portB();

  usart1_set_buffers(&app.console_in, &app.console_out);



  printf("\n\n\n\n--------\n");
  printf("addr main() %p\n", main );

  printf("\n--------");
  printf("\nstarting\n");


  assert( sizeof(bool) == 1);
  assert( sizeof(float) == 4);
  assert( sizeof(double ) == 8);





  //////////////////////////////


  // vfb buffer
  // fBufInit(&app.vfb_cbuf, buf_vfb, ARRAY_SIZE(buf_vfb));


  /*
    really not sure that app_t initilization should be done here....
  */

  // measure
  fBufInit(&app.vfb_measure, buf_vfb_measure, ARRAY_SIZE(buf_vfb_measure));
  fBufInit(&app.ifb_measure, buf_ifb_measure, ARRAY_SIZE(buf_ifb_measure));

  // range
  fBufInit(&app.vfb_range, buf_vfb_range, ARRAY_SIZE(buf_vfb_range));
  fBufInit(&app.ifb_range, buf_ifb_range, ARRAY_SIZE(buf_ifb_range));





  ////////////////
  spi1_port_setup();
#if 0
  spi1_special_gpio_setup();
#endif
  // adc interupt...
  spi1_interupt_gpio_setup( (void (*) (void *))spi1_interupt, &app);


  ////////////////////

  // TODO . this is very specific. not sure this should be here.
  // should have separate app_t setup.

  app.spi = SPI1 ;
  app.print_adc_values = true;
  app.output = false;

  app.nplc_measure = 50;
  app.nplc_range   = 20;
  app.digits = 6;

  // app.vrange = 0;
  // app.irange = 0;


  // state_change(&app, STATE_FIRST );

  loop(&app);

	for (;;);
	return 0;
}




