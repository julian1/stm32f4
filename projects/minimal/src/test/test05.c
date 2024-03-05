
#include <stdio.h>
#include <assert.h>

#include <mode.h>
#include <app.h>
#include <ice40-reg.h>    // MODE_DIRECT
#include <lib2/util.h>         // msleep()



bool app_test05( app_t *app , const char *cmd)
{
  assert(app);
  assert(cmd);
  assert(app->mode_initial);

  int32_t i0;

  if( sscanf(cmd, "test05 %ld", &i0 ) == 1) {

      printf("test non-az mode - leakage by first charging for 10sec, then turn off muxes, and mux signal via pc-out to amplifier\n");
      // app->test_in_progress = 0;

      _mode_t j = *app->mode_initial;

      j.second.U1006  = S1 ;          // s1.   follow  .   dcv-mux2

      if(i0 == 10) {
        printf("with +10V\n");
        j.second.U1003  = S1 ;       // s1. dcv-source s1. +10V.
      }
      else if(i0 == -10) {
        printf("with -10V\n");
        j.second.U1003  = S2 ;       // s2.  -10V.
      }
      else if(i0 == 0) {
        printf("with 0V\n");
        j.second.U1003 = S3;          // s3 == agnd
      }
      else assert(0);

      // turn on accumulation relay
      j.first .K405 = LR_SET;     // select dcv
      j.first .K406 = LR_RESET;   // accum relay on
      j.first .K407 = LR_RESET;   // on dcv-source

      // set up fpga
      j.reg_mode =  MODE_DIRECT;
      // j.reg_direct.himux2 = S1 ;    // s1 put dc-source on himux2 output
      // j.reg_direct.himux  = S2 ;    // s2 himux mux himux2 output
      j.reg_direct.azmux_o = SOFF;
      j.reg_direct.sig_pc1_sw_o = 1;  // precharge mux signal.
      j.reg_direct.leds_o = 0b1;        // turn on led, because muxinig signal.

      // app_transition_state( app->spi, &j,  &app->system_millis );

      spi_mode_transition_state( app->spi, &j, &app->system_millis);

      ////////////////////////////
      // so charge cap to the dcv-source, then turn off the mux and see how it drifts.
      // charge for 10sec. for DA....
      printf("sleep 10s\n");  // having a yield would be quite nice here.
      msleep(10 * 1000,  &app->system_millis);

      /////////////

      printf("switchout relay - to observe drift\n");

      j.first .K407 = LR_SET;   // switch off/out dcv-source
      spi_mode_transition_state( app->spi, &j, &app->system_millis);

      printf("sleep 10s\n");  // having a yield would be quite nice here.
      msleep(10 * 1000,  &app->system_millis);


      // OK. when we release the relay - there is a small change.   inductive???
      // eg. for 10V
      // 9.896,99 to  9.896,42     weird.   need to try just high-z on the dcv-source mux.
      // 9.897,50  this time.
      // about a 0.5mV.  jump.  when flick the relay.  charge injection. or something to do with the contacts spiking?

#if 0
      /////////
      F  f;
      memset(&f, 0, sizeof(f));         // turn off himux, and azmux.

      f.himux2 = S4 ;               // s4 gnd. himux2 to reduce leakage.
      f.sig_pc_sw_ctl  = 1;         // precharge mux signal. into azmux which is off.
      f.led0 = 1;                   // because muxinig signal.

      spi_ice40_reg_write_n(app->spi, REG_DIRECT, &f, sizeof(f) );
#endif


/*
      F  f;
      memset(&f, 0, sizeof(f));         // turn off himux
      f.himux2 = S4 ;                   // s4 gnd. himux2 to reduce leakage.
      spi_ice40_reg_write_n(app->spi, REG_DIRECT, &f, sizeof(f) );
      spi_ice40_reg_write32(app->spi, REG_MODE, MODE_NO_AZ );
*/


      return 1;
    }


  return 0;
}

