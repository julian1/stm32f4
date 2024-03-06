
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

      _mode_t mode = *app->mode_initial;

      ////////////////////
      // phase 1, soak/charge accumulation cap

      printf("setup dcv-source and charge cap\n");

      mode.second.U1006  = S1 ;          // s1.   follow  .   dcv-mux2

      if(i0 == 10) {
        printf("with +10V\n");
        mode.second.U1003  = S1 ;       // s1. dcv-source s1. +10V.
      }
      else if(i0 == -10) {
        printf("with -10V\n");
        mode.second.U1003  = S2 ;       // s2.  -10V.
      }
      else if(i0 == 0) {
        printf("with 0V\n");
        mode.second.U1003 = S3;          // s3 == agnd
      }
      else assert(0);

      // turn on accumulation relay
      mode.first .K405 = LR_SET;     // select dcv
      mode.first .K406 = LR_RESET;   // accum relay on
      mode.first .K407 = LR_RESET;   // select dcv-source

      // set up fpga
      mode.reg_mode =  MODE_DIRECT;
      mode.reg_direct.azmux_o = SOFF;
      mode.reg_direct.sig_pc1_sw_o = SW_PC_SIGNAL;  // TODO - reveiew - why not mux BOOT ? during soak phase?.
      // mode.reg_direct.sig_pc1_sw_o = 1;  // precharge mux signal.
      mode.reg_direct.leds_o = 0b0001;        // phase first led turn on led, because muxinig signal.

      spi_mode_transition_state( app->spi, &mode, &app->system_millis);
      printf("sleep 10s\n");  // having a yield would be quite nice here.
      msleep(10 * 1000,  &app->system_millis);



      ////////////////////////
      // phase 2, discocnnect dcv-source

      printf("disconnect dcv-source and observe drift\n");
      mode.first .K407 = LR_SET;
/*     mode.second.U1006  = 0;          // weird - we switch the dc-source mux off - we have very high leakage. might be flux.
      mode.second.U1003 = 0; */
      mode.reg_direct.leds_o = 0b0010;

      spi_mode_transition_state( app->spi, &mode, &app->system_millis);
      printf("sleep 10s\n");  // having a yield would be quite nice here.
      msleep(10 * 1000,  &app->system_millis);


      ////////////////////////
      // phase 3. observe, take measurement etc

      mode.reg_direct.leds_o = 0b0100;
      // now we do the sleep- to take the measurement.
      printf("sleep 2s\n");  // having a yield would be quite nice here.
      spi_mode_transition_state( app->spi, &mode, &app->system_millis);
      msleep(2 * 1000,  &app->system_millis);


      // returning,  will revert back to mode_current state.
      return 1;
    }


  return 0;
}





   /*

OK. mar 6. 2024.

    left after cleaning for 24 hours. - seems ok. now.

    do one initial round to help DA.
    +10V   -0.2mV. +0.7mV   OK.
    -10V    +0.7mv.  +0.58mV.
    0V       +0.01mV    0.13mV

    ----
    Ok . but still a big jump 0.5mV when the relay sets.

    9.896,91 -> ,43   ,33    9.897,26    the other direction as well. weird.




mar 5. 2024.
    // OK. when we release the relay - there is a small change.   inductive???
    // eg. for 10V
    10V.
    9.896,99 to  9.896,42    -570uV.   need to try just high-z on the dcv-source mux.

    0V.
      0.000,01 -> 0.000,35    +340uV
      0.000,01 -> 0.000,26

    -10V.
      -9.899,92 -> -9.900,37    +390uV.
    ///////////////////////////

    after cleaning.
    Ok. now getting 27mV.  drift in 10s. on -10V.  using relay for off.... not good.

    and -35mV.   on +10V.   is this cap DA. or leakage. or something else?

    0V. is fine <1mV.

    dates. of caps are very different 1549.   eg. 2015.
    versus 2236.                                  2022.

    possible causes -
    - using guard traces, rather than copper fills for leakage?
    - date codes - of cap, adg1208, opa140 ?
    - having power supplies  routed on layer, directly underneath op/adg pins.
    - may just be DA.

    - we can test - using electrometer on a spare pcb.

    need to wait til morning.
    - maybe explains why when switched off dcv-source using mux - have issue. high leakage also.

    - hour later. it's worse.  -10V.   50mV.
      and when do it manually.

    set u1003 s1; set u1006 s1 ;
    set k405 set; set k406 reset; set k407 reset
    set k407 set
    ------------
      */





/*
  - this may have been done with azmux fitted - which increases leakage.

  // old. date unknown.
  // charge cap +10V hold 10sec. get 0.8mV / 10 s.  0.8pA. leave few minutes. 0.6mV. == 0.6pA.  15mins == 0.4pA.
  // charge cap 0V.  hold 10sec. get around 2.3 -> 2.6mV. == 0.3mV/.  0.3pA.
  // charge cap -10V hold 10sec. get around 2mV / 10s. == 2pA.  input pin, doesn't make much difference to negative leakaave

  // oct 18. 2023.
  // data lost?

  // oct 19.
  // refactor to use fpga no-az mode.
  // 5mins between.

  +10V.
    0.5mV. 0.6mV.
  0V.
    1.6mV.  1.8mV.
  -10V.
    4.3mV.  4.3mV.


*/







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

