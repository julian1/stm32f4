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


#include "spi-port.h"
#include "spi-ice40.h"
#include "mux.h"
#include "reg.h"



#include "fbuffer.h"

#include "format.h"   // format_bits()



// app structure
#include "app.h"

#include "dac8734.h"
#include "4094.h"




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


static void relay_set( uint32_t spi, bool state, uint8_t u304, uint32_t l1, uint32_t l2)
{
  // we don't actually have to pass u304 by reference sinice we don't modify it.

  assert( !(u304 & l1) );
  assert( !(u304 & l2) );

  if(state) {
    // hmmmmm we need a local
    u304 |=  l1;
    spi_4094_reg_write(spi, u304 );
    msleep(10);
    u304 &= ~ l1;
    spi_4094_reg_write(spi, u304 );
    msleep(10);
  }
  else {

    u304 |= l2 ;
    spi_4094_reg_write(spi, u304 );
    msleep(10);
    u304 &= ~ l2 ;
    spi_4094_reg_write(spi, u304 );
    msleep(10);

  }
}



static void update_soft_500ms(app_t *app)
{
  // UNUSED(app);

  // char buf[100];
  static bool led_state = 0;
  led_state = ! led_state;


  // blink the fpga led
  mux_ice40(app->spi);

  if(led_state)
    ice40_reg_set(app->spi, REG_LED,   LED0 |  LED2 /*| LED3*/ );
  else
    ice40_reg_clear(app->spi, REG_LED, LED0 | LED2 /*| LED2 | LED3 */);


  // blink mcu led
  led_set( led_state );

  // hang on. is the writing of data over the spi the issue.


  // enable OE of 4094
  ice40_reg_write(app->spi, REG_4094,  GLB_4094_OE );
  // ice40_reg_write(app->spi, REG_4094,  0 );


#if 1
  static int count = 0;
  printf("count %u\n", count++);

  mux_4094(app->spi);

  // it is a bit weird - should be clicking every 500ms. 
  // or maybe - it is slient when returns?
  // seems ok. when check on continuity. think relay is just silent in other direction.

  // toggle both relays at same time. doubles current
  relay_set( app->spi, led_state, app->u304, /*U304_K301_L1_CTL |*/ U304_K302_L1_CTL , /*U304_K301_L2_CTL |*/ U304_K302_L2_CTL);

#endif

/*
  char buf[100];
  mux_ice40(app->spi);
  uint8_t val = ice40_reg_read( app->spi, REG_SPI_MUX );
  printf("reg_spi_mux read bits %u %s\n", val, format_bits(buf, 8, val) );
*/


}


static void update_soft_stress_test_2_50ms( app_t *app)
{
  return;

  static bool led_state = 0;
  led_state = ! led_state;

  printf("--\n");

  // blink the fpga led
  mux_ice40(app->spi);

  if(led_state)
    ice40_reg_set(app->spi, REG_LED,   LED0 |  LED2 /*| LED3*/ );
  else
    ice40_reg_clear(app->spi, REG_LED, LED0 | LED1 | LED2 | LED3 );


  char buf[100];
  uint8_t val0, val1 ;

  val0 = ice40_reg_read( app->spi, REG_LED );
  printf("led read bits %u %s\n", val0, format_bits(buf, 8, val0) );

  val1 = ice40_reg_read( app->spi, REG_LED );
  printf("led read bits %u %s\n", val1, format_bits(buf, 8, val1) );

  if(val0 != val1)  {
    printf("different\n");
    critical_error_blink();
  }

  // blink mcu led
  led_set( led_state );

}


static void update_soft_stress_test_50ms(app_t *app)
{
  return;

  printf("--\n");

  static bool led_state = 0;
  led_state = ! led_state;

  // blink the fpga led
  mux_ice40(app->spi);

  if(led_state)
    ice40_reg_set(app->spi, REG_LED,   LED0 |  LED2 | LED3 );
  else
    ice40_reg_clear(app->spi, REG_LED, LED0 | LED1 | LED2 | LED3 );

  // blink mcu led
  led_set( led_state );

  char buf[100];
  uint8_t val;

  val = ice40_reg_read( app->spi, REG_LED );
  printf("reg_led read bits %s\n", format_bits(buf, 4, val) );

  if(led_state && val !=  (LED0 |  LED2 | LED3 ) ) {
    printf("*** error\n" );
    critical_error_blink();
  }

  if(!led_state && (val !=   0 ) ) {
    printf("*** error\n" );
    critical_error_blink();
  }

  ice40_reg_set(app->spi, REG_SPI_MUX,   0x5 );

  val = ice40_reg_read( app->spi, REG_SPI_MUX);
  printf("reg_spi_mux read bits %u %s\n", val, format_bits(buf, 8, val) );
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
  static uint32_t soft_50ms = 0;
  static uint32_t soft_500ms = 0;
  static uint32_t soft_1s = 0;

  /*
    Think all of this should be done/moved to update()...
  */


  // while(true);

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



    // 50ms soft timer. for stress testing.
    if( (system_millis - soft_50ms) > 50) {
      soft_50ms += 50;
      update_soft_stress_test_50ms(app);
      update_soft_stress_test_2_50ms(app);
    }


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
  spi1_port_cs1_setup();
#if 0
  spi1_special_gpio_setup();
#endif
  // adc interupt...
  spi1_port_interupt_gpio_setup( (void (*) (void *))spi1_interupt, &app);


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


  // do fpga reset
  spi1_port_cs1_cs2_gpio_setup();

  msleep(1000);

  //while(1) 
  { 
    // set cs lo, to pull creset lo
    printf("assert cs\n");
    spi1_port_cs1_enable();
    spi1_port_cs2_enable();
    msleep(100);
    printf("deassert cs\n");
    spi1_port_cs1_disable();
    spi1_port_cs2_disable();
    msleep(100);
  }



  loop(&app);

	for (;;);
	return 0;
}



/*
  if(led_state)
    spi_4094_reg_write(app->spi , 0b11111111 );
  else
    spi_4094_reg_write(app->spi , 0 );
*/

  // we can factor this fairly easily into a single function -- with the delay.
  // actualyl variable name probably not great. because will chain them.
/*
  if(led_state) {
    // hmmmmm we need a local
    app->u304 |=  U304_K302_L1_CTL;
    spi_4094_reg_write(app->spi, app->u304 );
    msleep(10);
    app->u304 &= ~ U304_K302_L1_CTL ;
    spi_4094_reg_write(app->spi, app->u304 );
  }
  else {

    app->u304 |= U304_K302_L2_CTL ;
    spi_4094_reg_write(app->spi, app->u304 );
    msleep(10);
    app->u304 &= ~ U304_K302_L2_CTL ;
    spi_4094_reg_write(app->spi, app->u304 );

  }
  */

  /*
  // the issue is the verilog 4094 vector - will continue to follow cs2 - when we change from gpio back to spi alternate function.
      eg. and it will go high - as the ordinary parking state of cs.

      options - invert the connection to the output.
      use fpga register to control cs of the 4094. so do write. and then toggle the register.

      eg. an independent approach.

      maybe the whole thing would be easier - if just used ice40 regsisters for CS.
      issue is. the sequence. set to mux ice40 and assert. then set to peripheral peripheral, write. set to muxice 40 and to cs deassert.
      -----
      what if changed. so on mcu side.

      WE don't alternate the cs pins.   instead we just assert an extra pin/ to disambiguate target  using gpio .

      eg. SO ALL WRITES USE ordinary CS1.
      Whether it's too the fpga or the fpga peripheral.  is whether the additional gpio is hi or lo.
      there are no synchronization issues with mcu spi.

      and it avoids constantly having to reconfigure the spi-ports
      --------------

      Doesn't fix. the issue for 4094.  but simplifying things - would make inverting the strobe simpler. / or assign
      --------------

      lets try the invert trick as is. to see if can get work. and test relay.
      -------------
      EXTR - there may be synchronization issues. eg. CS not deasserted. before gpio is swapped.
  */

#if 0
  mux_4094(app->spi);
  spi_4094_reg_write(app->spi , 0b01010101 );

  // msleep(1);    // if we put a sleep here we get a diffferent read value?????

  /*
  // perhaps the action of reading is also setting the value????
      becasue it's an 8 bit register and the bit is in the clear bits....
    -----
    Or it's an overflow....   on read.

    SOLUTION - might be not to remove all the bit setting - but have a parallel register that is direct write.
    -------

    - try writing something that fits in 3 bits. and see if that gets overwritten.
    - maybe change the 16bit width to 24 bit.
  */
  mux_ice40(app->spi);

  val = ice40_reg_read( app->spi, REG_SPI_MUX);
  printf("reg_spi_mux read bits %u %s\n", val, format_bits(buf, 8, val) );

  msleep(1);

  val = ice40_reg_read( app->spi, REG_SPI_MUX);
  printf("reg_spi_mux read bits %u %s\n", val, format_bits(buf, 8, val) );

#endif


