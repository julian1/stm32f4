
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




// prefix app_test05()
bool test06( app_t *app , const char *cmd)
{
  assert(app);
  assert(cmd);
  assert(app->mode_initial);

  int32_t i0;

  if( sscanf(cmd, "test06 %ld", &i0 ) == 1) {

      // similar except mux boot, and feed through the the amplifier.
      // electrometer mode.
      printf("test electrometer mode - leakage by first charging for 10sec, then turn off himuexes, and mux boot direct to amplifier\n");
      app->test_in_progress = 0;

      Mode j = *app->mode_initial;

      if(i0 == 10) {
        printf("with +10V\n");
        j.second.U1003  = S1 ;       // s1. dcv-source s1. +10V.
        j.second.U1006  = S1 ;       // s1.   follow  .   dcv-mux2
      }
      else if(i0 == -10) {
        printf("with -10V\n");
        j.second.U1003  = S2 ;       // s2.  -10V.
        j.second.U1006  = S1 ;       // s1.   follow  .   dcv-mux2
      }
      else if(i0 == 0) {
        printf("with 0V\n");
        j.second.U1003 = S3;          // s3 == agnd
        j.second.U1006 = S6;          // s6 = agnd  .  TODO change to S7 . populate R1001.c0ww
      }
      else assert(0);

      // turn on accumulation relay
      j.first .K406_CTL  = RBOT;

      // set up fpga
      j.reg_mode =  MODE_DIRECT;
      j.reg_direct.himux2 = S1 ;    // s1 put dc-source on himux2 output
      j.reg_direct.himux  = S2 ;    // s2 himux mux himux2 output
      j.reg_direct.azmux = SOFF;
      j.reg_direct.sig_pc_sw_ctl  = 0;  // precharge mux boot.
      j.reg_direct.led0 = 0;        // should probably be off, since not signal.

      app_transition_state( app->spi, &j,  &app->system_millis );

      ////////////////////////////
      // so charge cap to the dcv-source, then turn off the mux and see how it drifts.
      // charge for 10sec. for DA....
      printf("sleep 10s\n");  // having a yield would be quite nice here.
      msleep(10 * 1000,  &app->system_millis);
      printf("turn off input muxes, azmux now mux boot\n");



      F  f;
      memset(&f, 0, sizeof(f));         // turn off himux

      f.himux2 = S4 ;                   // s4 gnd. himux2 to reduce leakage.
      f.sig_pc_sw_ctl  = SW_PC_BOOT;     // pc switch muxes boot. to turn off signal to reduce leakage
      f.azmux          = S2;             // azmux muxes boot directly

      spi_ice40_reg_write_n(app->spi, REG_DIRECT, &f, sizeof(f) );

      spi_ice40_reg_write32(app->spi, REG_MODE, MODE_NO_AZ ); // use no az mode.



      spi_ice40_reg_write_n(app->spi, REG_DIRECT, &f, sizeof(f) );

      return 1;
    }


  return 0;
}


/*
  // oct 18.

  +10V.
    -0.3mV  -0.3mV
  0V.
    0.7mV.  0.7mV.
  -10V.
    2.6mV.  2.5mV.

  // oct 19.
  // refactor to use fpga electrometer mode.
  // 5mins between.

  +10V.
      -0.1mV. 0.0mV
  0V.
      0.5mV 0.5mV
  -10V.
      1.8mV  1.7mV.


*/



