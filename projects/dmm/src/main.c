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


#include <stdio.h>    // printf, scanf
#include <string.h>   // strcmp, memset


// library code
#include "usart.h"
#include "assert.h"
#include "streams.h"
#include "util.h"   // msleep()


#include "spi-port.h"
#include "spi-ice40.h"
#include "mux.h"
#include "reg.h"



#include "fbuffer.h"

#include "format.h"   // format_bits()



// app structure
#include "app.h"

// modes of operation.
#include "mode.h"

#include "4094.h"



static void spi1_interupt(app_t *app)
{
  UNUSED(app);
#if 0
  /*
    interupt context. avoid doing work here...
  */
  if(app->adc_drdy == true) {
    // still flagged from last time, then code is too slow, and we missed an adc read
    ++app->adc_drdy_missed;
  }

#endif

  // if flag is still active, then record we missed processing some data.
  if(app->adc_drdy == true) {
    app->adc_drdy_missed = true;
  }

  // set adc_drdy flag so that update() knows to read the adc...
  app->adc_drdy = true;
}



static void sys_tick_interupt(app_t *app)
{
  // interupt context. don't do anything compliicated here.

  ++ app->system_millis;
}









static void state_format ( uint8_t *state, size_t n)
{
  assert(state);

  char buf[100];
  for(unsigned i = 0; i < n; ++i ) {

    printf("v %s\n",  format_bits(buf, 8, state[ i ]  ));
  }
}






////////////////////
// consider move to another file
// consider rename aper_n to just aper or aperture

#define CLK_FREQ        20000000

uint32_t nplc_to_aper_n( double nplc, uint32_t lfreq )
{
  assert( lfreq);

  double period = nplc / (double) lfreq;  // seonds
  uint32_t aper = period * CLK_FREQ;
  return aper;
}


double aper_n_to_nplc( uint32_t aper_n, uint32_t lfreq)
{
  assert( lfreq);

  double period   = aper_n / (double ) CLK_FREQ;          // use aper_n_to_period()
  double nplc     = period * (double) lfreq;
  return nplc;
}


double aper_n_to_period( uint32_t aper_n)
{
  double period   = aper_n / (double ) CLK_FREQ;
  return period;
}

uint32_t period_to_aper_n(  double period )
{
  return period * CLK_FREQ;
}


bool nplc_valid( double nplc )
{
  /*
    used in a few places for user-input arg validate user input
    can be relaxed later.
    maybe use a switch/case
    todo - similar to validate/check  voltage source
  */
  return
     nplc == 0.1 || nplc == 0.5 || nplc == 1
    || nplc == 2 || nplc == 10 || nplc == 100 || nplc == 1000;
}


void aper_n_print( uint32_t aperture,  uint32_t lfreq)
{

//   uint32_t aperture = nplc_to_aper_n( f0, app->lfreq );
  printf("aperture %lu\n",   aperture );
  printf("nplc     %.2lf\n",  aper_n_to_nplc( aperture, lfreq ));
  printf("period   %.2lfs\n", aper_n_to_period( aperture ));
}






void app_transition_state( unsigned spi, const Mode *mode, volatile uint32_t *system_millis)
{
  assert(mode);
  assert( sizeof(X) == 5 );

  // should we be passing as a separate argument


  // mux spi to 4094. change mcu spi params, and set spi device to 4094
  mux_4094( spi);


  printf("-----------\n");

  printf("app_transition_state write first state\n");
  // state_format (  (void *) &mode->first, sizeof(X) );

  // and write device
  spi_4094_reg_write_n(spi, (void *) &mode->first, sizeof( X) );

  // sleep 10ms
  msleep(10, system_millis);


  // and format
  printf("app_transition_state write second state\n");
  // state_format ( (void *) &mode->second, sizeof(X) );

  // and write device
  spi_4094_reg_write_n(spi, (void *) &mode->second, sizeof(X) );

  /////////////////////////////

  // now do fpga state
  mux_ice40(spi);

  // set mode
  spi_ice40_reg_write32(spi, REG_MODE, mode->reg_mode );

    // spi_ice40_reg_write_n(app->spi, REG_DIRECT, &f, sizeof(f) );
  spi_ice40_reg_write_n(spi, REG_DIRECT,  &mode->reg_direct,  sizeof( mode->reg_direct) );
  // spi_ice40_reg_write_n(spi, REG_DIRECT2, &mode->reg_direct2, sizeof( mode->reg_direct) );   unused again.

  spi_ice40_reg_write32(spi, REG_ADC_P_APERTURE, mode->reg_adc_p_aperture );


  // can add the reg reset here.

}






static void app_update_soft_1s(app_t *app)
{
  UNUSED(app);

}


// change name app_app_update_soft_100ms.

static void app_update_soft_100ms(app_t *app)
{
  assert(app);

  // potentially useful soft-timer for some tests.

  if(app->test_in_progress == 2 ) {

      mux_ice40(app->spi);
      static uint32_t magic = 0;      // static. should put in app state
      uint8_t reg =  REG_LED;

      spi_ice40_reg_write32(app->spi, reg, magic);
      uint32_t ret = spi_ice40_reg_read32( app->spi, reg);

      // perhaps it's taking a long time??
      char buf[ 100] ;
      printf("r %u  v %lu %s %s\n", reg, ret, format_bits(buf, 32, ret), magic == ret ? "ok" : "error");
      ++magic;

  }
}





static void app_update_soft_500ms(app_t *app)
{
  assert(app);

  /*
    function should reconstruct to localize scope of app. and then dispatch to other functions.

  */

  app->led_state = ! app->led_state;



  // blink mcu led
  // be explicit. don't hide top-level state.
  if(app->led_state)
    gpio_clear( app->led_port, app->led_out);
  else
    gpio_set(   app->led_port, app->led_out);



  // hearbeat and led flash
  mux_ice40(app->spi);

  // use a magic number to blink the led. and test comms
  uint32_t magic = app->led_state ? 0b010101 : 0b101010 ;

  // note - led will only, actually light if fpga in mode. 1.
  spi_ice40_reg_write32( app->spi, REG_LED, magic);

  // check the magic numger
  uint32_t ret = spi_ice40_reg_read32( app->spi, REG_LED);
  if(ret != magic ) {

    // comms no good
    char buf[ 100] ;
    printf("no comms, wait for ice40 v %s\n",  format_bits(buf, 32, ret ));
    app->comms_ok = false;

    // REVIEW
    // return
  }
  else {

    // comms ok,
    if( app->comms_ok == false) {

      /////////////
      // coming from a condition with no comms - eg. initial startup, or restoration of comms.
      // then do start up sequence.

      // reset mode_current to initial
      *app->mode_current = *app->mode_initial;

      // write initial 4094 state - for muxes. before turning on 4094 OE.
      printf("write initial 4094 state\n");
      assert(app->mode_initial);
      app_transition_state( app->spi, app->mode_initial,  &app->system_millis );


      mux_ice40(app->spi);

      // now turn on 4094 OE
      printf("turn on 4094 OE %u\n", GLB_4094_OE);
      spi_ice40_reg_write32( app->spi, REG_4094, GLB_4094_OE);

      /* turning on OE could fail.  because fpga comms goes down.
        which means 4094 state won't get updated in future write.
        which is a difficult case to have to handle.
      */

      ret = spi_ice40_reg_read32( app->spi, REG_4094);
      if( ret != GLB_4094_OE) {
        printf("write of 4094 OE reg failed\n");
        return;
      }

      // now do initial transition again. to  put relays in the right state with 4094 OE enabled
      printf("rewrite initial 4094 state\n");
      app_transition_state( app->spi, app->mode_initial,  &app->system_millis );

      // make sure fpga is in a default mode.
      spi_ice40_reg_write32(app->spi, REG_MODE, 0 );


      printf("comms ok\n");
      printf("> ");
      app->comms_ok = true;
    }
  }



  // get and check, the status register.
  ret = spi_ice40_reg_read32( app->spi, REG_STATUS);
  // if( ((ret >> 8 ) & 0xff)  != app->last_reg_status ) {

  // not sure if quite right.
  if( (ret & 0xff )  != (app->last_reg_status & 0xff) ) {

    printf("status changed\n");
    char buf[ 100] ;
    printf("ret  %s\n",  format_bits(buf, 32, ret ));
    printf("last %s\n",  format_bits(buf, 32, app->last_reg_status  ));
    app->last_reg_status  = ret;
  }



  // the flipping of the input relay. test. we want to keep.

  if(app->test_in_progress == 3 ) {

    // tests b2b and K405 relay sequencing.
    if(app->led_state) {

      app_transition_state( app->spi, app->mode_initial, &app->system_millis );
    }
    else {

      //////////
      Mode mode_derived = *app->mode_initial;     // copy initial state.eg. turn all relays off

      /*
         - could stagger these - to hear/confirm they turn on.
          - for turning the relay on. test03. could stagger the relays. eg. 100ms. between each one

      */

      // TODO . want an invert function macro
      // eg. mode_derived.first.K402_CTL = invert( mode_initial.first.K402_CTL )

      // switch all relays at the same time.  should be another test
      // works...
      mode_derived.first.K405_CTL    = LR_TOP;
      mode_derived.first.K402_CTL    = LR_TOP;      // WRONG. needs to be invert of what it previously is.  write an invert function.
      mode_derived.first.K401_CTL    = LR_TOP;
      mode_derived.first.K403_CTL    = LR_TOP;


      mode_derived.first. U408_SW_CTL = 0;        // turn off b2b fets, while switching relay on.
      mode_derived.second.U408_SW_CTL = 1;       // turn on/close b2b fets.


      app_transition_state( app->spi, &mode_derived, &app->system_millis );
    }
  }


  if(app->test_in_progress == 4 ) {

    // mode is 3. blink led. by writing direct register. don't want to do this in other cases.
    F f;
    memset(&f, 0, sizeof(f));

    f.led0 = app->led_state;

    mux_ice40(app->spi);
    spi_ice40_reg_write_n(app->spi, REG_DIRECT, &f, sizeof(f) );
  }


}





static void app_update_console_cmd(app_t *app)
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


      uint32_t u0 , u1;
      // int32_t i0;

      char s0[100 + 1 ];

      ////////////////////


      if(strcmp(cmd, "reset mcu") == 0) {
        // reset stm32f4
        // scb_reset_core()
        scb_reset_system();
      }



      else if(strcmp(cmd, "reset fpga") == 0) {

        printf("do reset\n" );
        mux_ice40(app->spi);
        spi_ice40_reg_write32(app->spi, REG_RESET, 1 );

        msleep(10, &app->system_millis);
        spi_ice40_reg_write32(app->spi, REG_RESET, 0 );
      }


      ///////////////


      else if( sscanf(cmd, "mode %lu", &u0 ) == 1) {

        // set the fpga mode.
        mux_ice40(app->spi);
        spi_ice40_reg_write32(app->spi, REG_MODE, u0 );

        uint32_t ret = spi_ice40_reg_read32(app->spi, REG_MODE );
        printf("reg_mode return value %lu\n", ret);
      }
      else if( sscanf(cmd, "mode %100s", s0 ) == 1) {

        mux_ice40(app->spi);

        if(strcmp(s0, "lo") == 0 || strcmp(s0, "default") == 0)
          spi_ice40_reg_write32(app->spi, REG_MODE, MODE_LO);
        else if(strcmp(s0, "direct") == 0)
          spi_ice40_reg_write32(app->spi, REG_MODE, MODE_DIRECT);
          // ...
        else {
          printf("bad direct arg\n" );
        }
        //
      }

      ////////////////////

      else if( sscanf(cmd, "direct %lu", &u0 ) == 1) {

        // set the direct register.
        printf("set direct value to, %lu\n", u0 );

        mux_ice40(app->spi);
        spi_ice40_reg_write32(app->spi, REG_DIRECT, u0 );
        // confirm.
        uint32_t ret = spi_ice40_reg_read32(app->spi, REG_DIRECT );
        char buf[ 100 ] ;
        printf("r %u  v %lu  %s\n",  REG_DIRECT, ret,  format_bits(buf, 32, ret ));
      }
      else if( sscanf(cmd, "direct bit %lu %lu", &u0, &u1 ) == 2) {

        // modify direct_reg and bit by bitnum and val
        /* eg.

            mode direct
            direct 0         - clear all bits.
            direct bit 13 1  - led on
            direct bit 13 0  - led off.
            direct bit 14 1  - mon0 on
            direct bit 22 1  - +ref current source on. pushes integrator output lo.  comparator pos-out (pin 7) hi.
            direct bit 23 1  - -ref current source on. pushes integrator output hi.  comparator pos-out lo
            --
            for slow run-down current. turn on bit 23 1. to push integrator hi.
            then add bit 22 1.  for slow run-down. works, can trigger on scope..about 2ms. can toggle bit 22 off against to go hi again.

            direct bit 25   - reset. via 20k.
            direct bit 26   - latch.  will freeze/latch in the current comparator value.

          - note. run-down current creates integrator oscillation when out-of-range.
        */

        mux_ice40(app->spi);
        uint32_t ret = spi_ice40_reg_read32(app->spi, REG_DIRECT );
        if(u1)
          ret |= 1 << u0 ;
        else
          ret &= ~( 1 << u0 );

        char buf[ 100 ] ;
        printf("r %u  v %lu  %s\n",  REG_DIRECT, ret,  format_bits(buf, 32, ret ));
        spi_ice40_reg_write32(app->spi, REG_DIRECT, ret );
      }

      ////////////////////

    // could probably

      else if( strcmp(cmd, "mode?") == 0) {

        mux_ice40(app->spi);
        uint32_t ret = spi_ice40_reg_read32(app->spi, REG_MODE );

        // TODO add some decoding here.
        printf("reg_mode return value %lu\n", ret);
      }
      else if( strcmp( cmd, "direct?") == 0) {

        mux_ice40(app->spi);
        uint32_t ret = spi_ice40_reg_read32(app->spi, REG_DIRECT );
        char buf[ 100];
        printf("r %u  v %lu  %s\n",  REG_DIRECT, ret,  format_bits(buf, 32, ret ));
      }
      else if( strcmp( cmd, "status?") == 0) {

        mux_ice40(app->spi);
        uint32_t ret = spi_ice40_reg_read32(app->spi, REG_STATUS);
        char buf[ 100];
        printf("r %u  v %lu  %s\n",  REG_STATUS, ret,  format_bits(buf, 32, ret ));
      }

    /*
      -- don't really need, just query direct reg for monitor and right shift.
    */
      else if( strcmp( cmd, "monitor?") == 0) {
        mux_ice40(app->spi);
        uint32_t ret = spi_ice40_reg_read32(app->spi, REG_DIRECT);
        ret >>= 14;
        char buf[ 100];
        printf("r %u  v %lu  %s\n",  REG_DIRECT, ret,  format_bits(buf, 8, ret ));
      }








        // OK, it would be nice to support setting a  vector. over the command line.
        // issue is cannot do the relay switching.
        // there's definitely DA.


      else if( strcmp(cmd, "loop3") == 0) {

          // just swapping control loops - is easy.
          // gives more control . rather than a complicated re-entrant function.
          // and pump the message buffers etc.
          // but
          // its just that existing. if we pump the command buffer.
          // Loop3  loop3;
          // memset(&loop3, 0, sizeof(loop3));

          app_loop3(app/*, &loop3*/);

      }



      else if( strcmp(cmd, "test02") == 0) {
        // EXTR - this can be a different loop mode.
        // spi stress test in 100ms soft timer
        printf("spi stress test spi comms\n");
        app->test_in_progress = 2;
      }



      else if( strcmp(cmd, "test03") == 0) {

        // don't care about mode. for 4094 stuff.
        printf("test03 use 4094, to click dcv sequencing/ b2b fet/ relays\n");
        app->test_in_progress = 3;
      }


      else if( strcmp(cmd, "test04") == 0) {

        printf("test04 use direct mode - to blink led\n");

        // TODO should be a transition.

        mux_ice40(app->spi);
        spi_ice40_reg_write32(app->spi, REG_MODE, MODE_DIRECT);

        app->test_in_progress = 4;
      }


    /*
      having arguments is ok. for internal test.  that just prints a result.

    */

      else if( test05( app, cmd  )) { }
      else if( test06( app, cmd  )) { }

      else if( test08( app, cmd  )) { }   // change to test 4. i think.

      else if( test11( app, cmd  )) { }

      else if( test14( app, cmd  )) { }

      else if( test15( app, cmd  )) { }

      else if( test16( app, cmd  )) { }

      // TODO. Ok. we are manipulating mode_initial which isn't right.

      else if( app_extra_functions( app, cmd  ))
      {
        // test15 done
      }




      else if( strcmp( cmd , "") == 0) {

      }

      else {

            printf("unknown cmd, or bad argument '%s'\n", cmd );

      }



      // reset buffer
      cStringClear( &app->command);

      // issue new command prompt
      printf("> ");
    }
  }
}



static void app_loop(app_t *app)
{
/*
 - EXTR. having separate 4094, from fpga controlled outputs. is good.
 because we don't have to pulse the relays.

  - when need to detect coming out of loss of comms - and redo the 4094.
*/

  /*
    at initial start up. eg. check rails
    initial state - should do once.
    ----
    and after loosing comms.

  */


  while(true) {


    // process data in priority
    if(app->adc_drdy == true) {

      app->adc_drdy = false;

      mux_ice40(app->spi);

      uint32_t clk_count_mux_neg = spi_ice40_reg_read32( app->spi, REG_ADC_CLK_COUNT_MUX_NEG);
      uint32_t clk_count_mux_pos = spi_ice40_reg_read32( app->spi, REG_ADC_CLK_COUNT_MUX_POS);
      uint32_t clk_count_mux_rd  = spi_ice40_reg_read32( app->spi, REG_ADC_CLK_COUNT_MUX_RD);
      uint32_t clk_count_mux_sig = spi_ice40_reg_read32( app->spi, REG_ADC_CLK_COUNT_MUX_SIG);

      printf("app loop data  %lu %lu %lu %lu\n", clk_count_mux_neg, clk_count_mux_pos, clk_count_mux_rd, clk_count_mux_sig);


    }

    // did we miss data, for any reason
    if( app->adc_drdy_missed == true) {
      printf("missed data\n");
      app->adc_drdy_missed = false;
    }



    /*
      it is possible to call this in a long-running function - to keep the main command process working
      but noting that it will not unwind the stack.
      like a proper yield() function.

      // EXTR. - could actually call update at any time, in a yield()...
      // so long as we wrap calls with a mechanism to avoid stack reentrancy
      // led_update(); in systick.
      // but better just to flush() cocnsole queues.conin/out
      // ----
      // no better for a callee to yield back to update(), while setting up a dispatch callback to get control.
    */

    app_update_console_cmd(app);



    // 100s soft timer
    if( (app->system_millis - app->soft_100ms) > 100) {
      app->soft_100ms += 100;
      app_update_soft_100ms(app);
    }


    // 500ms soft timer
    if( (app->system_millis - app->soft_500ms) > 500) {
      app->soft_500ms += 500;
      app_update_soft_500ms(app);
    }

    // 1000ms soft
    if( (app->system_millis - app->soft_1s) > 1000 ) {
      app->soft_1s += 1000;
      app_update_soft_1s(app);
    }


    /*
      // simpler approach to threads, cooperative threading/co-routines.
      // we've done everythinig needed.  now check if there is a yielded function, and then pass control back out to it.

      if(appp->yielded_function) {

        app->yielded_function( app->spi );
        app->yielded_function( app->spi,  app->yielded_ctx );
      }

      where the yielded function. is a state machine. that does the yield in a long running sequence,
      and which handle re-entry by using state, and a switch/case statement.
      -----

      can use the fsm function approach - where the top level of function is a switch/case statement - to handle the re-entrancy.

    */

  }
}









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




// move to main ?
// no reason to be static.
// no... because we collect/assemble dependencies. ok in main()
static app_t app;








static const Mode mode_initial =  {

  //  maybe make explicit all values  U408_SW_CTL. at least for the initial mode, from which others derive.

  .first .K406_CTL  = LR_TOP,     // accumulation relay off

  .first. K405_CTL  = LR_BOT,     // dcv input relay k405 switch off - works.
  .first. K402_CTL  = LR_BOT,     // dcv-div/directz relay off
                                // must match app->fixedz

  // .first. K401_CTL  = LR_BOT,     // dcv-source relay off.    (WRONG. turns it on??).
  .first. K401_CTL  = LR_TOP,     // dcv-source relay off.    (WRONG. turns it on??).
  .first. K403_CTL  = LR_BOT,     // ohms relay off.

  .first .U408_SW_CTL = 0,      // b2b fets/ input protection off/open


  // AMP FEEDBACK SHOULD NEVER BE TURNED OFF.
  // else draws current, and has risk damaging parts. mux pin 1. of adg. to put main amplifier in buffer/G=1 configuration.
  .first. U506 =  W1,     // should always be on

  // .second.K406_CTL  = LR_OFF,     // clear relay. default.
  // .second.K405_CTL  = LR_OFF,     // clear relay
  .second.U408_SW_CTL = 0,

  .second.U506 =  W1,           // should  always be on.


  // fpga mode default. blink led.

  .reg_mode = MODE_LO,

  .reg_adc_p_aperture = 4000000         // wset explicitly in dcv
                                        // Not. should use current calibration?
                                        // should be authoritative source of state.

};



static Mode mode_current;


int main(void)
{
  // high speed internal!!!
  // TODO. not using.

	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ] );  // stm32f411  upto 100MHz.


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

  gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_OUT);

  // setup external state for critical error led blink
  // because assert() cannot pass a context
  critical_error_led_setup(LED_PORT, LED_OUT);


  // this is the mcu clock.  not the adc clock. or the fpga clock.
  // systick_setup(16000);

  // extern void systick_setup(uint32_t tick_divider, void (*pfunc)(void *),  void *ctx);
  systick_setup(84000,  (void (*)(void *)) sys_tick_interupt, &app);  // 84MHz.


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


  // sep 16, 2023.
  // set ER-CRESET-  hi.  not to reset.   This needs to have an external  pullup fitted.
#define ER_CRESET_PORT  GPIOA
#define ER_CRESET_PIN   GPIO1

  gpio_mode_setup(ER_CRESET_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, ER_CRESET_PIN);
  gpio_set(ER_CRESET_PORT, ER_CRESET_PIN);




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


  app.led_port = LED_PORT;
  app.led_out  = LED_OUT;


  app.mode_initial =  &mode_initial;
  app.mode_current =  &mode_current;

  app.comms_ok = false;
  app.fixedz  = false;
  app.lfreq = 50;


  printf("sizeof X    %u\n", sizeof(X));
  printf("sizeof Mode %u\n", sizeof(Mode ));
  printf("sizeof F    %u\n", sizeof(F));

  // check some obvious stuff.
  assert(sizeof(F) == 4);
  assert(sizeof(X) == 5);

  assert( (1<<3|(6-1)) == 0b1101 );
  assert( S6  == 0b1101 );



  // modes_init();

  // go to main loop
  app_loop(&app);

	for (;;);
	return 0;
}


/*
  probably from printf
  _close_r  _fstat_r _getpid_r _kill_r _isatty_r

  see.
    -specs=nano.specs -specs=nosys.specs

    arm-none-eabi-gcc: fatal error: /nix/store/3ydyllv3y22qpxcgsf9miwq4dkjwjcj2-gcc-arm-embedded-12.2.rel1/bin/../lib/gcc/arm-none-eabi/12.2.1/../../../../arm-none-eabi/lib/nosys.specs: attempt to rename spec 'link_gcc_c_sequence' to already defined spec 'nosys_link_gcc_c_sequence'


    --specs=rdimon.specs

  https://stackoverflow.com/questions/73742774/gcc-arm-none-eabi-11-3-is-not-implemented-and-will-always-fail
*/

#if 1

void _close_r( void );
void _fstat_r( void );
void _getpid_r( void);
void _kill_r( void);
void _isatty_r( void);
void _lseek_r( void);
void _read_r( void);
void _write_r( void);


void _close_r( void )
{
  assert(0);
}

void _fstat_r( void )
{
  assert(0);
}

void _getpid_r( void)
{
  assert(0);
}

void _kill_r( void)
{
  assert(0);
}

void _isatty_r( void)
{
  assert(0);
}

void _lseek_r( void)
{
  assert(0);
}

void _read_r( void)
{
  assert(0);
}

void _write_r( void)
{
  assert(0);
}

#endif





/*
      if( strcmp(cmd, "reset") == 0) {
        printf("reset initial state\n");

        Mode j = mode_initial;
        app_transition_state( app->spi, &j,  &app->system_millis );

        mux_ice40(app->spi);
        spi_ice40_reg_write32(app->spi, REG_MODE, 0 );  // defaultpattern.

        // make sure relay switching test is off.
        app->test_in_progress = 0;

      }
*/

// app.print_adc_values = true;
  // app.output = false;



#if 0
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

#endif



#if 0

  // we don't need to wait anymore... we have heartbeat
  // mux spi to ice40
  mux_ice40(app.spi);
  spi_ice40_wait_for_ice40( app.spi );


  /*
  TODO
    this is not quite right.
    we must write /reset 4094 state before turning on OE.
    else 4094 flip/flops may come up in any state,

    should also verify that value was correct. by writing.  and without asserting strobe.
    relays latch could be caught in on state.
  */

  printf("turning on 4094 OE\n");
  // output enable 4094
  spi_ice40_reg_write32( app.spi, REG_4094,  GLB_4094_OE );

#endif


  // printf("count %u\n", ++ app->count);
/*
  // 500ms. heartbeat check here.
  // this works nicely
  {
    uint32_t magic = LR_TOP10;   // this is returning the wrong value....
    mux_ice40(app->spi);
    spi_ice40_reg_write32( app->spi, REG_LED, magic);
    uint32_t ret = spi_ice40_reg_read32( app->spi, REG_LED);
    if(magic != ret) {
      char buf[ 100] ;

      printf("no comms, wait for ice40 v %s\n",  format_bits(buf, 32, ret ));
      // return
      // or should probably do a reset. when comms re-established
    }
  }

// should perhaps use top LR_BOT LR_TOPTOM
#define LR_OFF      0
#define LR_BOT      0b01      // top contact closed.
#define LR_TOP      0b10      // bottom contact closed.


*/


/*
  EXTR.
    might be easier to pass these around inside the app structure.

    Yes.
    rather than globals.
    should be passed. directly. or else taken from a context.

    commonly used vectors.
  ----------

  NO . these are vectors that should be constructed as needed'.



*/

////////////////////


/*
  Not sure if need.

  just carry a single current- state around.  for use with some tests.
*/
// static Mode mode_dcv_az ;

/* IMPORTANT the precharge switch relay - is held constant/closed for the mode.
  but we cycle one of the muxes - to read gnd/ to reset the charge on the cap.
  we can also. use dc-source to add an initial voltage bias to cap.
*/

// Mode mode_test_accumulation;

/*
  change name modes_init.

*/


/*
Other members are initialized as zero: "Omitted field members are implicitly initialized the same as objects that have static storage duration." (https://gcc.gnu.org/onlinedocs

*/



#if 0

static void modes_init( void )
{
  /*
    instead of having individual registers. we have individual elemental mode states , that exist.

    is there an alignment issue... with setting the bitfield???

  */

  // memset( &mode_zero,    0, sizeof( Mode) );
  memset( &mode_initial, 0, sizeof( Mode) );
  // memset( &mode_dcv_az,  0, sizeof( Mode) );
  // memset( &mode_test_accumulation,  0, sizeof( Mode) );

  /*
    ALL relays must be given a pulse here. to get to default state.
    cannot just leave at 0V.
  */

  //  should be explicit for all values  U408_SW_CTL. at least for the initial mode, from which others derive.
  mode_initial.first .K406_CTL  = LR_TOP;     // accumulation relay off   (seems inverted for some reason).
  mode_initial.second.K406_CTL  = LR_OFF;     // clear relay

  mode_initial.first .U408_SW_CTL = 0;      // b2b fets/ input protection off/open

  mode_initial.first. K405_CTL  = LR_BOT;     // dcv-input relay k405 switch off
  mode_initial.second.K405_CTL  = LR_OFF;     // clear relay

  mode_initial.second.U408_SW_CTL = 0;


  mode_initial.first. U506 =  W1;              // AMP FEEDBACK SHOULD NEVER BE TURNED OFF. danger of damaging parts. mux pin 1. of adg. to put main amplifier in buffer/G=1 configuration.
  mode_initial.second.U506 =  W1;

  /* EXTR. with series resistors to 4094 - for driving lower coil-voltage relays - there is a drop on the drive side.
      AND a drop on the lo side.  eg. with 50R. this looks like a poor on-pulse

  */
/*
  //////////
  mode_dcv_az = mode_initial;               // eg. turn all relays off
  mode_dcv_az.first. K405_CTL    = LR_TOP;     // turn dcv-input K405 on.
  mode_dcv_az.second.K405_CTL    = LR_OFF;    // clear relay.  don't really need since inherits from initial.

  mode_dcv_az.first. U408_SW_CTL = 0;        // turn off b2b fets, while switching relay on.
  mode_dcv_az.second.U408_SW_CTL = 1;       // turn on/close b2b fets.

*/

  ////
  // cap-accumulation mode.
  // mode_test_accumulation = mode_initial;
  // mode_test_accumulation.first .K406_CTL  = LR_BOT;  // accumulation relay on

}

#endif



// flashing of led... is writing ????



#if 0

static void spi_ice40_wait_for_ice40( uint32_t spi)
{
  // we don't need this anymore.
  // TODO better ame doto

  mux_ice40(spi);

  printf("wait for ice40\n");
  uint32_t ret = 0;
  // uint8_t magic = LR_BOT01; // ok. not ok now.  ok. when reset the fpga.
  uint8_t magic = LR_TOP10;   // this is returning the wrong value....
  do {
    // printf(".");

    spi_ice40_reg_write32( spi, REG_LED, magic);
    msleep( 50);
    ret = spi_ice40_reg_read32( spi, REG_LED);

    char buf[ 100] ;
    printf("wait for ice40 v %s\n",  format_bits(buf, 32, ret ));

    msleep( 50);
  }
  while( ret != magic );
  printf("\n");

}

#endif


#if 0
  {
    char buf[100];

      // printf("val %s\n",  format_bits(buf, 8,  write_val8( 0xff, 2, 1, 0xff ) ));

    // printf("val %s\n",  format_bits(buf, 8,  write_val8( 0x0, 1, 5, 0xff ) ));

    uint8_t v[ 3 ];
    memset(v, 0xff, sizeof(v));
    state_write ( v, sizeof(v), 16, 3, 0x0 );

    printf("v[0] %s\n",  format_bits(buf, 8, v[0 ]));
    printf("v[1] %s\n",  format_bits(buf, 8, v[1 ]));
    printf("v[2] %s\n",  format_bits(buf, 8, v[2 ]));
  }
#endif

  //////////////////////////////





/*
// chage name spi_ice40_stress test.

static void spi_ice40_stress_test_spi( uint32_t spi)
{
  // TODO better name. move code to separate test folder.
  // prefix with test ? perhaps

  printf("stress test spi comms\n");
  mux_ice40(spi);
  uint8_t magic = 0;

  uint8_t reg =  REG_LED;

  while(1) {
    spi_ice40_reg_write32(spi, reg, magic++);
    // spi_ice40_reg_write( spi, reg, magic ++ );
    uint32_t ret = spi_ice40_reg_read32( spi, reg);
    char buf[ 100] ;
    printf("r %u  v %lu  %s\n",  reg, ret,  format_bits(buf, 32, ret ));
    msleep( 150);
  }
}


static void spi_ice40_just_read_reg ( uint32_t spi)
{
  mux_ice40(spi);

  while(1) {
    uint32_t v = spi_ice40_reg_read32(spi, REG_LED);
    // uint32_t v = spi_ice40_reg_read32(spi, LR_OFF111111 );
    char buf[32+1];
    printf("value of led %lu %s\n", v, format_bits(buf, 32, v ));
    msleep( 500 );
  }
}


*/





#if 0


static uint32_t write_bits8( uint8_t v, uint8_t set, uint8_t clear )
{
  // *v = ~(  ~(*v | set ) | clear);  // clear has priority over set

  return ~( ~v | clear)  | set;    // set has priority
}


static uint8_t create_mask8( uint8_t pos, uint8_t width)
{
  uint8_t val = (1 << (pos + width )) - 1;   // set all bits below pos + width.
  uint8_t val2 = (1 << pos) - 1;            // set all bits below pos

  return val & ~val2;
}




static uint8_t write_val8 ( uint8_t v, uint8_t pos, uint8_t width, uint8_t value)
{
  // write value into v - at pos with width
  assert(pos < 8);
  assert(width < 8);

  uint8_t mask = create_mask8( pos, width);

  return write_bits8( v, (value << pos) & mask, mask);
}


// we manipulate the state. then write it.
// there is writing state to state var. then there is spi writing it.

static void state_write ( uint8_t *state, size_t n, unsigned pos, uint8_t width, uint8_t value)
{
  // determine index to use into byte array, and set value
  unsigned idx = pos >> 3 ;   // div 8
  assert( idx < n );
  uint8_t *v = & state[ idx ];

  *v = write_val8 ( *v, pos % 8, width, value);
}


#endif




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




/*
      else if( strcmp(cmd, "relay on") == 0) {  // output on

        relay_set( app->spi, app->state_4094, sizeof(app->state_4094), REG_K406,  1 );
      }
      else if( strcmp(cmd, "relay off") == 0) {  // output off

        relay_set( app->spi, app->state_4094, sizeof(app->state_4094), REG_K406,  0 );
      }
*/


#if 0
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






#endif



#if 0

static void relay_set( unsigned spi, uint8_t *state_4094, size_t n, unsigned reg_relay,   unsigned relay_state )
{


  // mux spi to 4094. change mcu spi params, and set spi device to 4094
  mux_4094( spi);


  // set two relay bits, according to dir
  state_write ( state_4094, n, reg_relay, 2, relay_state ? LR_BOT : LR_TOP );

  // output/print - TODO change name state_format_stdout, or something.
  state_format ( state_4094, n);

  // and write device
  spi_4094_reg_write_n(spi, state_4094, n );

  // sleep 10ms
  msleep(10);

  // clear the two relay bits
  state_write ( state_4094, n, reg_relay, 2, LR_OFF );   // clear

  // and write device
  spi_4094_reg_write_n(spi, state_4094, n );


  // turn off spi muxing
  mux_no_device(spi);

}
#endif




#if 0
// position in state.
/*
#define REG_K406    2
*/

#define REG_K406    2   // pins 2 and 3.  works well.


// #define REG_K401    (17  ) // third 4094.


// #define REG_K405_4094    2 (16 + 6  ) // third 4094.     L0.

#define REG_K405    (16 + 6  ) // third 4094.     L0.

#define REG_K405_L1    (2 * 8 + 6)
#define REG_K405_L2    (2 * 8 + 7)



#define REG_U404      8   // 2nd 4094.  4 bits.  works. well.

#endif

// we have to OR together an array....
// mode or state.

// so it would be easier to create values in functions.  where we can use an OR function - and record the bit position jj.
// Need to be able to indicate if the relay
// Does C99 have static initializers?
// static void my_or( uint8_t *a, uint8_t *b
// HANG on. it is still tricky. even with functions.... No with functions.
// mode_latch_relay_mask








#if 0
  // 4094 OE should have been configured already,
  uint32_t v = spi_ice40_reg_read32( app->spi, REG_4094);
  UNUSED(v);

  // assert(v == 1);
  // char buf[32+1];
  // printf("4094 state %lu %s\n", v, format_bits(buf, 32, v ));


  //////////////////////////////////
  // EXTR. THE specific led should be injected into APP state on construction.
  // not accessed as a global macro. makes it hard to test.
  //////////////////////////////////
#endif


#if 0
  // spi_ice40_reg_write32(app->spi, REG_LED, count );   // we don't have the set and clear bits...

/*
  uint32_t v = spi_ice40_reg_read32(app->spi, REG_LED);
  char buf[32+1];
  printf("value of led %lu %s\n", v, format_bits(buf, 32, v ));
*/


  // set mcu led state
  led_set( led_state );

#endif


  // toggles relay ok.
  // relay_set( app->spi, app->state_4094, sizeof(app->state_4094), REG_K406  , led_state );


  // toggles relay K405 - works well..
  // relay_set( app->spi, app->state_4094, sizeof(app->state_4094), REG_K405, led_state );
//


/*
  /// toggle ctrl pins of U404
  mux_4094( app->spi);
  // set two relay bits, according to dir.  is 4 is the number of bits.
  state_write ( app->state_4094, sizeof(app->state_4094), REG_U404 , 4, led_state ? LR_TOP00 : LR_OFF00 );
  // output/print - TODO change name state_format_stdout, or something.
  state_format ( app->state_4094, sizeof(app->state_4094));
  // and write device
  spi_4094_reg_write_n(app->spi, app->state_4094, sizeof(app->state_4094) );
*/

  /*
      - every state change has to be encoded - as two states.  eg. transition state, and resulting state.
          and has to be handled as a sequency.
          this is a little complicated.

      - But can still be encoded/recorded as a fixed string.

      - Eg. for DCV.
      - - have intermittent relay state.

      - perhaps even three states. eg. for dcv on. the sequencing of turn off b2b fets, then manipulate the relay, then turn on b2b fets.
      - but we may not need this.
      ------------------

      - EXTR.   AND we should encode the MUX registers to use. in the 4094 string. to make it single authoritative source.
                even if use a different way to write these to the fpga.
      -------
      - possible to also use maskable writes.

      - should be a single function that handles the 20ms timing. regardless of which state we want to use
      - we don't need to encode much.

      - we don't necessarily have to record the current state.  unless we
      - transition (  STATE_DCV  )    - and this would have the three transitions .
      - we don't keep individual registers arounds.   only some defines.
      -------------------

      Also the ability to copy.  with memcpy.   and then modify.

      - OR use single byte-array.    and store the two states end-to-end.   eg. if have 5 bytes for state. then use 10 bytes. for 2 transition states.

      - Also can store the mask.

      - latching relays have to use the transition data.   but analog switches need to remain.   Also makes sense to switch relays - before doing analog switches.

      - muxing has the 3x4 muxes = 12 bits. repeated for the signal, and zero.   so these would both be embedded in the string.  perhaps with a number representing sequence.
      --------
      EXTR.
      - eg. don't have separate registers for analog switches etc.  instead entire unit state encoded as single register.

      EXTR.
      - it may be possible to encode everything as single transition - if we manage how to .   eg. a relay set mask.  at least for   .

      - eg. relay clear mask.   this would be fixed/static   that would be or'ed,

      - initialize in a main functino    MODE_DCV_AZ
      - the writing of these bit valuees can be done in a function.
      - and post state mask - to clear relays.
      - transition(   state0, state1, state2 ).

      ------
      - simplest is to have the transition function manage - the latch clear state -  just with a fixed.

  */






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
  spi_4094_reg_write(app->spi , LR_BOT010101 );

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


