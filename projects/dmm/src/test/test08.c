
#include <stdio.h>      // printf, scanf
#include <string.h>     // strcmp, memset
#include <assert.h>

// lib
#include "util.h"       // msleep()



// local
#include "reg.h"
#include "mux.h"        // mux_ice40()
#include "spi-ice40.h"  // spi_ice40_reg_write32()


#include "app.h"

// modes of operation.
#include "mode.h"


/*

  TODO review.
  EXTR. - the above tests need to ensure that the bootttrapped pre-charge switch is turned off.
        - else loading on switch.

*/




// prefix app_test05()
bool test08( app_t *app , const char *cmd)
{


  if( strcmp(cmd, "test08") == 0 || strcmp(cmd, "test09") == 0 || strcmp(cmd, "test10") == 0) {

    // TODO - change test number - should be before test05, test06..
    // test mux leakage while off. by reset and holding at 0V. while putting a voltage on the cri mux.
    // TODO refactor these to take an argument for the voltage.

    printf("reset accumulation cap to 0V/agnd\n");
    app->test_in_progress = 0;
    Mode j = * app->mode_initial;

    if(strcmp(cmd, "test08") == 0) {
      printf("with +10V\n");
      j.second.U1003  = S1 ;       // s1. dcv-source s1. +10V.
      j.second.U1006  = S1 ;       // s1.   follow  .   dcv-mux2
    }
    else if(strcmp(cmd, "test09") == 0) {
      printf("with -10V\n");
      j.second.U1003  = S2 ;       // s2.  -10V.
      j.second.U1006  = S1 ;       // s1.   follow  .   dcv-mux2
    }
    else if(strcmp(cmd, "test10") == 0) {
      printf("with 0V\n");
      j.second.U1003 = S3;          // s3 == agnd
      j.second.U1006 = S6;          // s6 = agnd  .  TODO change to S7 . populate R1001.c0ww
    }
    else assert(0);

    // turn on accumulation relay     RON LR_OFF.  or RL1 ?
    j.first .K406_CTL  = LR_BOT;
    j.second.K406_CTL  = LR_OFF;    // don't need this....  it is 0 by default

    app_transition_state( app->spi, &j,  &app->system_millis );

    /////////////////
    // make sure we are in direct mode.
    mux_ice40(app->spi);
    spi_ice40_reg_write32(app->spi, REG_MODE, MODE_DIRECT );

    // now control the hi mux.  to reset the cap.
    F  f;
    memset(&f, 0, sizeof(f));
    f.himux2 = S4 ;    // s4 mux ground to reset cap.
    f.himux  = S2 ;    // s2 reflect himux2 on himux output
    spi_ice40_reg_write_n(app->spi, REG_DIRECT, &f, sizeof(f) );

    // it actually needs a kind of double transition.

    ////////////////////////////
    // so reset cap to 0V/agnd
    printf("sleep 10s\n");  // having a yield would be quite nice here.
    msleep(10 * 1000,  &app->system_millis);
    printf("turn off muxes - to see drift\n");


    // now float/turn of the  himux while leaving the input voltage on the pin.
    memset(&f, 0, sizeof(f));      // turn off everything
    f.himux2 = S1 ;                 // s1 put dc-source on himux2 output
    f.himux = SOFF;                // float himux
    spi_ice40_reg_write_n(app->spi, REG_DIRECT, &f, sizeof(f) );

    // with either +-10V. at input of himux, but with himux turned off,  we get about 1pF. rising.

    return true;
  }


  return false;
}





#if 0
      else if( strcmp(cmd, "test11") == 0 || strcmp(cmd, "test12") == 0 || strcmp(cmd, "test13") == 0) {

        // TODO - probably deprecate.

        printf("test normal AZ modulation at 10V,-10V,0V/agnd\n");
        app->test_in_progress = 0;
        Mode j = mode_initial;

        if(strcmp(cmd, "test11") == 0) {
          printf("with +10V\n");
          j.second.U1003  = S1 ;       // s1. dcv-source s1. +10V.
          j.second.U1006  = S1 ;       // s1.   follow  .   dcv-mux2
        }
        else if(strcmp(cmd, "test12") == 0) {
          printf("with -10V\n");
          j.second.U1003  = S2 ;       // s2.  -10V.
          j.second.U1006  = S1 ;       // s1.   follow  .   dcv-mux2
        }
        else if(strcmp(cmd, "test13") == 0) {
          printf("with 0V\n");
          j.second.U1003 = S3;          // s3 == agnd
          j.second.U1006 = S6;          // s6 = agnd  .  TODO change to S7 . populate R1001.c0ww
        }
        else assert(0);

        // accumulation relay is off
        app_transition_state( app->spi, &j,  &app->system_millis );

        /////////////////
        // setup az mode
        mux_ice40(app->spi);
        spi_ice40_reg_write32(app->spi, REG_MODE, MODE_AZ );  // mode 3. test pattern on sig

        //////////////
        // use DIRECT0 to set lo sample, in azmode.
        F  f;
        memset(&f, 0, sizeof(f));
        f.himux2 = S1 ;       // s1 put dc-source on himux2 output
        f.himux  = S2 ;       // s2 reflect himux2 on himux output
        f.azmux  = S6 ;    // s6 == normal LO for DCV, ohms.
        // f.azmux  = S2 ;    // s2 == BOOT for test.
        // f.azmux  = SOFF ;     // soff for high-z for test.
        spi_ice40_reg_write_n(app->spi, REG_DIRECT, &f, sizeof(f) );

        // use DIRECT 2 to set the hi sample
        memset(&f, 0, sizeof(f));
        f.azmux  = S1 ;         // s1 == PC_OUT (either SIG or BOOT).
        // f.azmux  = S2 ;      // s2 == BOOT for test.
        // f.azmux  = SOFF ;    // soff for high-z for test.
        spi_ice40_reg_write_n(app->spi, REG_DIRECT2, &f, sizeof(f) );

        // for precharge spinning the switch. want azmuxs for both samples to be high-z.
        // OK. want to be able to set the clk_duration.

        // behavior is quite different because of timing.

        // write the frequency. 10MHz. counter freq.
        spi_ice40_reg_write32(app->spi, REG_CLK_SAMPLE_DURATION, CLK_FREQ * 20e-3 );        // 1nplc, 20ms. freq == 25Hz for hi/lo period.
        // spi_ice40_reg_write32(app->spi, REG_CLK_SAMPLE_DURATION, CLK_FREQ * 200e-3 );    // 10nplc 200ms.

      }
#endif


