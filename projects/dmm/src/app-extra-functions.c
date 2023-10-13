


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
  Ok, we want to be able to set the input source. and then the amplifier gain.
  extra_functions.


  we can make it easier.

  have amp...  and input commands
  rather than range.
  ---
  not sure - it would be nice to work - 2W. and 22. then don't have

  Or dcv10s  - meaning using local soucr.   and dcv10 as terminal <- YES.
*/


bool app_extra_functions( app_t *app , const char *cmd, Mode *mode)
{
  /*
      - this modifies current-state.
      - actually could/ should be placed on app. be p

      - commands to set dcv-source
      - commands to set range.
      - commands to set mode az, nplc. etc
        -----------

      - to persist changinig input range with change dc-source we have to persist the mode.
        actually maybe not. maybe not.


  */


  assert(app);
  assert(cmd);

  uint32_t u1;
  int32_t i0;
  // char s[100];




  if( sscanf(cmd, "nplc %lu", &u1 ) == 1) {

    assert(u1 == 1 || u1 == 10 || u1 == 100 || u1 == 1000); // not really necessary. just avoid mistakes

    uint32_t aperture = nplc_to_aper_n( u1 );

    printf("aperture %lu\n",   aperture );
    printf("nplc     %.2lf\n",  aper_n_to_nplc( aperture ));
    printf("period   %.2lfs\n", aper_n_to_period( aperture ));
    // write duration. should move.
    spi_ice40_reg_write32(app->spi, REG_CLK_SAMPLE_DURATION, aperture );
    return 1;
  }



  else if(strcmp(cmd, "reset") == 0) {

    // think we should be derivinig from initial.
    // *mode = mode_initial;  etc.

    // ok. this is working.
    printf("reset.\n");

    // set the amp gain.
    mode->first.U506  =  W1;
    mode->second.U506 =  W1;

    mode->first. K405_CTL  = RTOP;     // dcv-input relay k405 switch on
    mode->second. K405_CTL = ROFF;

      // need to open the relay also.
    do_4094_transition( app->spi, mode,  &app->system_millis );


    mux_ice40(app->spi);
    // set mode.
    spi_ice40_reg_write32(app->spi, REG_MODE, MODE_LO ); // default.


    // turn off any concurrent test.
    app->test_in_progress = 0;


    return 1;
  }


  // issue is the accumulation relay on? accidently?

  /*
      a range is a type of mode.
      dcv10,dcv1,dcv01 are all the same except amplifier gain. so can consolidate
  */

  else if(strcmp(cmd, "dcv10") == 0
      || strcmp(cmd, "dcv1") == 0
      || strcmp(cmd, "dcv01") == 0
      ) {

    // think we should be derivinig from initial.
    // *mode = mode_initial;  etc.

    // set the amp gain.
    if( strcmp(cmd, "dcv10") == 0) {
        printf("whoot dcv10\n");
        mode->first. U506 =  W1;    // amp feedback should never be turned off.
        mode->second.U506 =  W1;
    }
    else if( strcmp(cmd, "dcv1") == 0) {
        printf("whoot dcv1\n");
        mode->first. U506 =  W2;  // amp feedback should never be turned off.
        mode->second.U506 =  W2;
    }
    else if( strcmp(cmd, "dcv01") == 0) {   // 100mV range
        printf("whoot dcv01\n");
        mode->first. U506 =  W3;  // amp feedback should never be turned off.
        mode->second.U506 =  W3;
    }


    // EXTR. - it would be better to derive the state to use from initial .

    // close/ turn on K405 relay.
    mode->first.  K405_CTL  = RBOT;
    mode->second. K405_CTL  = ROFF;

    // accumulation relay off
    mode->first .K406_CTL  = RBOT;
    mode->second.K406_CTL  = ROFF;

    // need to populate and arm the fets also.

      // need to open the relay also.
    do_4094_transition( app->spi, mode,  &app->system_millis );
    //
    mux_ice40(app->spi);
    // set mmode az azmux
    spi_ice40_reg_write32(app->spi, REG_MODE, MODE_AZ );  // mode 3. test pattern on sig
    // set params.
    F  f;
    memset(&f, 0, sizeof(f));
    f.himux2 = S4 ;    // gnd to reduce leakage on himux
    f.himux  = S7 ;    // dcv-in
    f.azmux  = S6;    // lo
    spi_ice40_reg_write_n(app->spi, REG_DIRECT, &f, sizeof(f) );
    // set the hi signal az.
    f.azmux  = S1;  // pc-out.
    spi_ice40_reg_write_n(app->spi, REG_DIRECT2, &f, sizeof(f) );


    // EXTR. important. can query/read/check fpga nplc state - and check if already set. if not then set to default for range.
    uint32_t ret = spi_ice40_reg_read32( app->spi, REG_CLK_SAMPLE_DURATION);
    if(ret == 0) {

      uint32_t nplc = 1;
      printf("aperture not set, using %lu 1nplc.u\n",  nplc );

      uint32_t aperture = nplc_to_aper_n( nplc );    // 1nplc default.

      // write duration. should move.
      spi_ice40_reg_write32(app->spi, REG_CLK_SAMPLE_DURATION, aperture );
    }




    return 1;
  }






  // have similar ranges/modes.


  else if(strcmp(cmd, "dcv1") == 0) {   // 1V range
    mode->first.U506 =  W2;
    mode->second.U506 =  W2;


  }
  else if(strcmp(cmd, "dcv0.1") == 0) {   // 0.1V range. 100mV

    mode->first.U506 =  W3;
    mode->second.U506 =  W3;
  }

  else if(strcmp(cmd, "dcv100") == 0) { // 100V range.

    // need to flip the hv divider. and set the input to DCV-DIValso.
    // amp gain == 2.
    mode->first.U506 =  W2;
    mode->second.U506 =  W2;

  }
  else if(strcmp(cmd, "2w") == 0) { // 100V range.

    // amp gain == 1.
    mode->first.U506 =  W1;
    mode->second.U506 =  W1;

  }



  else if(strcmp(cmd, "dcv10s") == 0) { //  G=1, use source as input.
    mode->first.U506 =  W1;
    mode->second.U506 =  W1;
  }

    else {
      return 0;

    }
    // have to




    // we also have to setup the input source.
    // this should probably be the terminal????
    // if dcv-source is active then it should probably be that.
/*
        F  f;
        memset(&f, 0, sizeof(f));
        f.himux2 = S1 ;    // s1 put dc-source on himux2 output
        f.himux  = S2 ;    // s2 reflect himux2 on himux output

        // set az mux.
        f.azmux  = S6 ;    // s6 == normal LO for DCV, ohms.
        spi_ice40_reg_write_n(app->spi, REG_DIRECT, &f, sizeof(f) );

        // set the hi signal az.
        f.azmux  = S1 ;         // s1 == PC_OUT (either SIG or BOOT).
        spi_ice40_reg_write_n(app->spi, REG_DIRECT2, &f, sizeof(f) );

*/

    return 1;
  }



#if 0
  if( sscanf(cmd, "dcv-source %ld", &i0) == 1) {

      // use a switch / case.

      if(i0 == 10) {
        printf("with +10V\n");
        mode->second.U1003  = S1 ;       // s+ve
        mode->second.U1006  = S1 ;       // s1 = 10V.
      }
      else if(i0 == 1) {
        printf("with +1V\n");
        mode->second.U1003  = S1 ;       // +ve
        mode->second.U1006  = S2 ;       // s2 = 1V
      }
//
/*        else if(i0 == 0.1) {            // 0.1 cannot be expressed without a double.... annoying.  // why is this not an error
        printf("with +1V\n");
        mode->second.U1003  = S1 ;       // +ve
        mode->second.U1006  = S3 ;       // s2 = 0.1V
      }
*/
      else if(i0 == 0) {
        printf("with 0V\n");
        mode->second.U1003  = S1 ;       // +ve
        mode->second.U1006  = S4 ;       // s2 = 1V
      }


      else if(i0 == -1) {
        printf("with -1V\n");
        mode->second.U1003  = S2 ;       // -ve
        mode->second.U1006  = S2 ;       // s2== 1V
      }

      else if(i0 == -10) {
        printf("with -10V\n");
        mode->second.U1003  = S2 ;       // -ve
        mode->second.U1006  = S1 ;       // s1.   follow  .   dcv-mux2
      }
      else if(i0 == 0) {
        printf("with 0V\n");
        mode->second.U1003 = S3;          // s3 == agnd
        mode->second.U1006 = S6;          // s6 = agnd  .  TODO change to S7 . populate R1001.c0ww  should be S4.
      }
      else assert(0);

    do_4094_transition( app->spi, mode,  &app->system_millis );
    return 1;

  }
#endif


