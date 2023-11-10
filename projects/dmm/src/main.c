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
#include <ctype.h>    // isspace


// library code
#include "usart.h"
#include "assert.h"
#include "streams.h"
#include "util.h"   // msleep()


#include "temp-mcu.h"

#include "spi-port.h"
#include "spi-ice40.h"
#include "mux.h"
#include "reg.h"



// #include "fbuffer.h"

#include "format.h"   // format_bits()



// app structure
#include "app.h"

// modes of operation.
#include "mode.h"

#include "4094.h"




#include "calc.h"





static void spi1_interupt(app_t *app)
{
  // now on a positive transition.

  // if flag is still active, then record we missed processing some data.
  if(app->adc_measure_valid == true) {
    app->adc_measure_valid_missed = true;
    // ++app->adc_measure_valid_missed;     // count better? but harder to report.
  }

  // set adc_measure_valid flag so that update() knows to read the adc...
  app->adc_measure_valid = true;
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



/*
  wnat azmux_from_string( )  also

*/


const char * azmux_to_string( uint8_t azmux )
{
  switch(azmux) {
      // this is a mess. just return a pointer
      case AZMUX_PCOUT:   return "pcout";
      case AZMUX_BOOT:    return "boot";
      case AZMUX_STAR_LO: return "star-lo";
      case AZMUX_REF_LO:  return "ref-lo";
    default:              return "unknown";
  };
  return NULL;
}


const char * himux_to_string( uint8_t himux, uint8_t himux2 )
{
  switch (himux)

    case HIMUX_HIMUX2: {

      switch(himux2 ) {
          case HIMUX2_DCV_SOURCE: return "dcv-source";
          case HIMUX2_STAR_LO:    return "star-lo";
          case HIMUX2_REF_HI:     return "ref-hi";
          case HIMUX2_REF_LO:     return "ref-lo";
          default:                return "unknown";
      };

      case HIMUX_DCV:   return "dcv";
      default:          return "unknown";
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
/*
  return
     nplc == 0.1 || nplc == 0.5 || nplc == 1
    || nplc == 2 || nplc == 10 || nplc == 100 || nplc == 1000;
*/
  return nplc >= 0.1 && nplc <= 100;

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

  // printf("app_transition_state write first state\n");
  // state_format (  (void *) &mode->first, sizeof(X) );

  // and write device
  spi_4094_reg_write_n(spi, (void *) &mode->first, sizeof( X) );

  // sleep 10ms
  msleep(10, system_millis);


  // and format
  // printf("app_transition_state write second state\n");
  // state_format ( (void *) &mode->second, sizeof(X) );

  // and write device
  spi_4094_reg_write_n(spi, (void *) &mode->second, sizeof(X) );

  /////////////////////////////

  // now do fpga state
  mux_ice40(spi);

  // set mode
  spi_ice40_reg_write32(spi, REG_MODE, mode->reg_mode );

  spi_ice40_reg_write_n(spi, REG_DIRECT,  &mode->reg_direct,  sizeof( mode->reg_direct) );

  spi_ice40_reg_write32(spi, REG_ADC_P_APERTURE,          mode->reg_adc_p_aperture );
  spi_ice40_reg_write32(spi, REG_ADC_P_CLK_COUNT_RESET,   mode->reg_adc_p_reset );

  spi_ice40_reg_write32(spi, REG_SA_P_CLK_COUNT_PRECHARGE, mode->reg_sa_p_clk_count_precharge );

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

  // todo - remove.  not worth the complication of test_in_pgoress.
  // rewrite - in favor of non-yielding loop.

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



  // TODO - remove test_in_progress.
  // having to cancel stuff is too complicated.
  // rewrite as separate loop function.

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

#if 0
  if(app->test_in_progress == 4 ) {

    // mode is 3. blink led. by writing direct register. don't want to do this in other cases.
    F f;
    memset(&f, 0, sizeof(f));

    f.led0 = app->led_state;

    mux_ice40(app->spi);
    spi_ice40_reg_write_n(app->spi, REG_DIRECT, &f, sizeof(f) );
  }
#endif


}



static char * trim_whitespace_inplace( char *cmd )
{
  assert(cmd);
  // messy functions - should probably get more testing.
  // trim leading whitespace.
  while(*cmd && isspace( (uint8_t ) *cmd ))
    ++cmd;

  char *p = cmd;    // position at string start
  while(*p) ++p;    // find string end
  --p;              // is this is a bug if given - zero length empty string ??.
                    // no because of (p>=cmd) check

  // trim trailing whitespace
  while(p >= cmd && isspace( (uint8_t) *p ))
    *p-- = 0;

  return cmd;
}


static void app_update_console_cmd(app_t *app)
{

  // rename - include REPL.  repl ( ) conso

  while( ! cBufisEmpty(&app->console_in)) {

    // got a character
    int32_t ch = cBufPop(&app->console_in);
    assert(ch >= 0);

    if(ch != '\r' && ch != ';' && cStringCount(&app->command) < cStringReserve(&app->command) ) {
      // must accept whitespace here, since used to demarcate args
      // normal character
      cStringPush(&app->command, ch);
      // echo to output. required for minicom.
      putchar( ch);

    }  else {

      // process...
      // newline or overflow
      putchar('\n');

      char *cmd = cStringPtr(&app->command);

      cmd = trim_whitespace_inplace( cmd );

      // useful for debug
      // printf("cmd '%s'  %u\n", cmd, strlen(cmd) );



      uint32_t u0 , u1;
      // int32_t i0;
      char s0[100 + 1 ];

      ////////////////////


      if(strcmp(cmd, "reset mcu") == 0) {

        printf("perform mcu reset\n" );
        // reset stm32f4
        // scb_reset_core()
        scb_reset_system();
      }



      else if(strcmp(cmd, "reset fpga") == 0) {

        printf("perform fpga reset\n" );
        mux_ice40(app->spi);
        spi_ice40_reg_write32(app->spi, REG_RESET, 1 );

        msleep(10, &app->system_millis);
        spi_ice40_reg_write32(app->spi, REG_RESET, 0 );
      }


      ///////////////


      else if( strcmp(cmd, "mode?") == 0) {

        // TODO add some decoding here.
        mux_ice40(app->spi);
        uint32_t ret = spi_ice40_reg_read32(app->spi, REG_MODE );
        printf("reg_mode return value %lu\n", ret);

        Mode *mode = app->mode_current;
        printf("app      return value %lu\n", mode->reg_mode );

      }



      else if( sscanf(cmd, "mode %lu", &u0 ) == 1) {
        /*
          need to think about setting this in current_mode.
          in order that we can also issue nplc for the test modulation controller.
          and not have stuff get overwritten
          ----------------

          we need to know the mode - az/ non az. to know how to process data.
          so needs to be set.
        */


        Mode *mode = app->mode_current;
        mode->reg_mode = u0 ;

/*
        // set the fpga mode.
        mux_ice40(app->spi);
        spi_ice40_reg_write32(app->spi, REG_MODE, u0 );

        uint32_t ret = spi_ice40_reg_read32(app->spi, REG_MODE );
        printf("reg_mode return value %lu\n", ret);
*/
        app_transition_state( app->spi, app->mode_current,  &app->system_millis );
      }

      else if( sscanf(cmd, "mode %100s", s0 ) == 1) {

/*
        mux_ice40(app->spi);

        if(strcmp(s0, "lo") == 0 || strcmp(s0, "default") == 0)
          spi_ice40_reg_write32(app->spi, REG_MODE, MODE_LO);
        else if(strcmp(s0, "direct") == 0)
          spi_ice40_reg_write32(app->spi, REG_MODE, MODE_DIRECT);
          // ...
        else {
          printf("bad direct arg\n" );
        }
*/
        Mode *mode = app->mode_current;


        if(strcmp(s0, "lo") == 0 || strcmp(s0, "default") == 0)
          mode->reg_mode = MODE_LO;
        else if(strcmp(s0, "direct") == 0)
          mode->reg_mode = MODE_DIRECT;
          // ...
        else {
          printf("bad direct arg\n" );
        }

        app_transition_state( app->spi, app->mode_current,  &app->system_millis );
        return;
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

        // this is no longer corrent. should query the REG_STATUS. which includes the monitor
        // regardless of the mode.
        mux_ice40(app->spi);
        uint32_t ret = spi_ice40_reg_read32(app->spi, REG_DIRECT);
        ret >>= 14;
        char buf[ 100];
        printf("r %u  v %lu  %s\n",  REG_DIRECT, ret,  format_bits(buf, 8, ret ));
      }








        // OK, it would be nice to support setting a  vector. over the command line.
        // issue is cannot do the relay switching.
        // there's definitely DA.


      else if( strcmp(cmd, "cal") == 0) {


          // swapping control loops - is easy.
          // gives more control . rather than a complicated re-entrant function.

          // just use recursion. rather than using re-entrant functions, and/or yields.
          // can pump the input cmd processing if really want, for halt/exit commands.

          app_cal(app);

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

#if 0
      else if( strcmp(cmd, "test04") == 0) {

        printf("test04 use direct mode - to blink led\n");

        // TODO should be a transition.

        mux_ice40(app->spi);
        spi_ice40_reg_write32(app->spi, REG_MODE, MODE_DIRECT);

        app->test_in_progress = 4;
      }
#endif


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







static void app_update_new_measure(app_t *app)
{


  /*
    race condition here,,
    adc may have produced a valid measure.
    but the sample acquisition was put into arm state by mcu
    So see how much we
    --
    this is simpler. and keeps dependencies better isolated, compared with having the sa_acquisition squash the valid signal.
  */

  if(app->adc_measure_valid) {

    char buf[100];

    // printf("-------------------\n");

    app->adc_measure_valid = false;

    mux_ice40(app->spi);


    /*
        -a consider adding a 8 bit. counter in place of the monitor, in the status register
        in order to check all values are read in a single transaction
        - or else a checksum etc.
    */

    uint32_t status =  spi_ice40_reg_read32( app->spi, REG_STATUS );

    // suppress late measure samples arriving after signal_acquisition is returned to arm
    if( ! (status & STATUS_SA_ARM_TRIGGER)) {

      /*
          this is done in software. and can only be done in software - because there is a race- condition.
          that adc can generate the obs - in the time that we write the arm/trigger register for signal acquisition.
      */
      return;
    }


    uint32_t clk_count_mux_reset = spi_ice40_reg_read32( app->spi, REG_ADC_CLK_COUNT_MUX_RESET);
    uint32_t clk_count_mux_neg = spi_ice40_reg_read32( app->spi, REG_ADC_CLK_COUNT_MUX_NEG);
    uint32_t clk_count_mux_pos = spi_ice40_reg_read32( app->spi, REG_ADC_CLK_COUNT_MUX_POS);
    uint32_t clk_count_mux_rd  = spi_ice40_reg_read32( app->spi, REG_ADC_CLK_COUNT_MUX_RD);
    uint32_t clk_count_mux_sig = spi_ice40_reg_read32( app->spi, REG_ADC_CLK_COUNT_MUX_SIG);


    /*  - OK. it doesn't matter whether aperture is for one more extra clk cycle. or one less.  eg. the clk termination condition.
        instead what matters is that the count is recorded in the same way, as for the reference currents.
        eg. so should should always refer to the returned count value, not the aperture ctrl register.

        uint32_t clk_count_mux_sig = spi_ice40_reg_read32( app->spi, REG_ADC_P_APERTURE );
    */

    if(app->verbose)
      printf("counts %6lu %7lu %7lu %6lu %lu", clk_count_mux_reset, clk_count_mux_neg, clk_count_mux_pos, clk_count_mux_rd, clk_count_mux_sig);



    // quick indication without
/*
    if(app->mode_current->reg_mode == MODE_AZ)  {
      printf(" %s ", (status & STATUS_SA_AZ_STAMP) ? "hi" : "lo"  );
    }
*/


    if(app->b) {

      // TODO - have a scalar - version of this
      // would eases calculation. when only need a scalar.
      // WE NEED TO GET THIS CODED in a test. and the result recorded.
      // unsigned cols = 4;

      /*
          should keep fields stored against app_t structure. to avoid cost and complexity of always reallocating.
          and having to free.
          and make early return easier

      // app->xs = run_to_mat( ..., app->xs );   its a good simplification.
      */
      MAT *xs = run_to_matrix(
          clk_count_mux_neg,
          clk_count_mux_pos,
          clk_count_mux_rd,
          app->model_cols,
          MNULL
        );
      // we could make all these vars persist.
      MAT	*m_mux_sig = m_from_scalar( clk_count_mux_sig, MNULL );
      assert(m_mux_sig);
      assert( m_is_scalar(m_mux_sig) );



      if ( m_cols(xs) != m_rows( app->b) ) {

        // calibtration sampled data, mismatch the cols mismatch.
        // shouldn't happen if arm is working.
        printf("m_cols(xs) != m_rows( b) \n");

        printf("app->cols   %u\n", app->model_cols );
        printf("app->b cols %u\n", m_cols( app->b ) );
        printf("app->b rows %u\n", m_rows( app->b ) );
        printf("xs     cols %u\n", m_cols( xs ) );
        printf("xs     rows %u\n", m_rows( xs ) );


        M_FREE( xs );
        M_FREE( m_mux_sig );
        return;
      }

      //  we should persist this.  and pass it in to m_calc_predicated.
      MAT *predicted =  m_calc_predicted( app->b, xs, m_mux_sig /*, app->predicted */);
      assert(predicted);
      assert( m_is_scalar(predicted) );

      double ret = m_to_scalar(predicted );

      M_FREE( xs );
      M_FREE( m_mux_sig );
      M_FREE( predicted );


      Mode *mode = app->mode_current;

      if(mode->reg_mode == MODE_NO_AZ )  {

        if(app->verbose) {
          printf(" no-az (%s)", azmux_to_string( mode->reg_direct.azmux));

        }
      }
      else if(mode->reg_mode == MODE_AZ)  {

        if(app->verbose)
          printf(" az");

        // determine if az obs high or lo
        if( status & STATUS_SA_AZ_STAMP  ) {
          // treat as hival
          if(app->verbose)
            printf(" (hi %s) ", himux_to_string( mode->reg_direct.himux, mode->reg_direct.himux2 ));
          app->hi = ret;
        }
        else {
          // treat as lo val
          if(app->verbose)
            printf(" (lo  %s)", azmux_to_string( mode->reg_direct.azmux));

          app->lo[ 1] = app->lo[ 0];  // shift last value
          app->lo[ 0] = ret;
        }

        if(app->verbose) {
          printf(" (hi %sV)",  format_float_with_commas(buf, 100, 7, app->hi ));
          printf(" (lo %sV",   format_float_with_commas(buf, 100, 7, app->lo[0]  ));
          printf(", %sV)",  format_float_with_commas(buf, 100, 7, app->lo[1] ));
        }

        // regardless whether we got a lo or a hi. calculate and show a new value.
        ret = app->hi - ((app->lo[ 0 ] + app->lo[1] ) / 2.0);
      }
      else {
          printf(" sample acquired from unknown mode");
      }


      // we could quieten this also.
      // raw data. no formatting.
      printf(" %.7lf", ret );
      // printf(" meas %sV", format_float_with_commas(buf, 100, 7, ret ));

      if(m_rows(app->sa_buffer) < m_rows_reserve(app->sa_buffer)) {

        // just push onto sample buffer
        m_push_row( app->sa_buffer, & ret , 1 );
      }

      else {
        // buffer is full, so insert
        // TODO there's an issue with modulo overflow/wrap around.

        unsigned idx = app->sa_count_i++ % m_rows(app->sa_buffer);
        // printf(" insert at %u\n", idx );
        m_set_val( app->sa_buffer, idx, 0,  ret );
      }

      if(app->verbose) {
        printf(" ");
        m_stats_print( app->sa_buffer );
      }
    }

    printf("\n");
  }


  // did we miss data, for any reason
  if( app->adc_measure_valid_missed == true) {
    printf("missed data\n");
    app->adc_measure_valid_missed = false;
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

    // deal with new input samples in priority


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

    app_update_new_measure(app);


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
static char buf_console_out[1000];    // changing this and it freezes. indicates. bug

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

  .reg_sa_p_clk_count_precharge = CLK_FREQ * 500e-6,             //  `CLK_FREQ * 500e-6 ;   // 500us.

  .reg_adc_p_aperture = CLK_FREQ * 0.2,   // 200ms. 10nplc 50Hz.  // Not. should use current calibration?  // should be authoritative source of state.

  .reg_adc_p_reset = CLK_FREQ * 500e-6                // 500us.


};






// use m_reserve_rows() to flexibly reserve/resize.


// Can put  regression here, also.
// static R regression;

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


  // adc/temp
  rcc_periph_clock_enable(RCC_ADC1);

  // this isn't right. it's for the fan.
  // gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO0);
  // gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO1);


  /*
    Do led first, even if need update() and systick loop() to blink it.
  */


  adc_setup();



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


  // some of this stuff could be deferred until app_loop()
  // perhaps should group into another structure.
  app.comms_ok = false;
  app.fixedz  = false;
  app.lfreq = 50;
  app.model_cols = 3;

  app.azmux_lo_val  = AZMUX_STAR_LO;



  // mesch turn on mem tracking
  mem_info_on(1 );


  // set up the sample buffer
  app.sa_buffer = m_resize( app.sa_buffer, 10, 1 );
  app.sa_count_i = 0;

  app.verbose = 1;

  ///////////////////
#if 0
  printf("-------------\n" );

  MAT   *xs = NULL;
  xs = m_resize(xs, 5, 2);
  m_truncate_rows( xs, 0 );

  printf("rows          %u\n", m_rows( xs) );
  printf("rows reserve  %u\n", m_rows_reserve( xs) );
/*
  printf("truncate rows\n" );
  double v[] = { 1,2, 3, 4, 5,6, 7,8, 9, 10 };
  xs = m_fill(  xs , v, ARRAY_SIZE(v));
  m_foutput( stdout, xs);
*/
  // ok. so we can create a new matrix. then truncate the rows.
  // then push data.

  double row[] = { 1, 2 } ;

  m_push_row( xs, row  , 2 );
  // m_push_row( xs, row  , 2 );
  m_foutput( stdout, xs);

  M_FREE(xs);
  printf("-------------\n" );
#endif
  ////////////////////////


  // printf("_stack      %u\n", _stack ); // linker var not exposed.

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





