
#include <stdio.h>
#include <assert.h>
#include <string.h>   // strcmp

#include <mode.h>
#include <app.h>
#include <ice40-reg.h>    // modes
#include <lib2/util.h>    // msleep()



bool app_test15( app_t *app , const char *cmd)
{
  assert(app);
  assert(cmd);
  assert(app->mode_initial);



  /*
      test charge-injection by charging to a bias voltage, holding, then entering az mode.
      with az-mux also switching .

    > reset ; dcv-source 10; nplc 1; test15

      only thing that really changes with test14. is the actual mode.  MODE_AZ instead of MODE_PC
      lot of opportunity to refactor.

  */

  if( strcmp(cmd, "test15") == 0) {


      printf("test leakage and charge-injection by switching pre-charge/azmux at different input dc-bias and frequency\n");


      /* assume dcv-source and nplc have been set up on mode already.
        we could verify with some checks.  */

      _mode_t mode = *app->mode_current;
 
      ////////////////////////
      // phase 1, soak/charge accumulation cap

      // setup input relays.
      mode.first .K405 = LR_SET;     // select dcv. TODO change if support himux.
      mode.first .K406 = LR_RESET;   // accum relay on
      mode.first .K407 = LR_RESET;   // select dcv-source on

      // set up fpga - with direct mode - for soak/charge of accum cap.
      mode.reg_mode     =  MODE_DIRECT;

      /*
        in direct_mode we manually set azmux to the azmux-hi-val while soaking/charging the accumulation capacitor.
        this keeps amplifier following input. and avoids case of amplifier oing out-of-range on floating input,
        and then discharging the accum cap, when coming back into stable-state, on entering az mode.
      */
      mode.reg_direct.azmux_o = mode.sa.reg_sa_p_azmux_hi_val;
      assert( mode.reg_direct.azmux_o == S3);       // can relax this to the other input later

      assert( mode.reg_direct.sig_pc_sw_o == 0b00 );

      mode.reg_direct.leds_o = 0b0001;        // phase first led turn on led, because muxinig signal.

      spi_mode_transition_state( app->spi, &mode, &app->system_millis);
      printf("sleep 10s\n");  // having a yield would be quite nice here.
      msleep(10 * 1000,  &app->system_millis);


      ////////////////////////
      // phase 2, discocnnect dcv-source
      //           and switch into precharge mode.

      printf("mode to pc-only\n");
      printf("disconnect dcv-source and observe drift\n");
      mode.reg_mode           = MODE_AZ_TEST;
      mode.first .K407        = LR_SET;          // disconnect dcv
      mode.reg_direct.leds_o  = 0b0010;    // advance led

      spi_mode_transition_state( app->spi, &mode, &app->system_millis);
      printf("sleep 10s\n");  // having a yield() would be quite nice here.
      msleep(10 * 1000,  &app->system_millis);



      ////////////////////////
      // phase 3. observe, take measurement etc

      mode.reg_mode = MODE_DIRECT;
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

  Mar 11.

  after refactor

    > reset ; dcv-source 10; nplc 1; test15
      5.0mV 4.95mV

    > reset ; dcv-source 0; nplc 1; test15
      7.5mV 8.2mV

    > reset ; dcv-source -10; nplc 1; test15
      8.15mV.

  ------
    reset ; dcv-source 10;  set azmux_o s3 ; nplc 1; test15
      5.5mV.

   reset ; dcv-source 0;  set azmux_o s3 ; nplc 1; test15
      9.0mV 8.4mV

  reset ; dcv-source -10;  set azmux_o s3 ; nplc 1; test15
      9.1mV.  8.3mV.



  Mar 10
  ----
  10 seconds. 10nF. charge cap.

  with amplifier and bootin. working.

  > reset ; dcv-source 10;  set azmux s3 ; nplc 1; test15
      6.3mV. 6.0mV.

  > reset ; dcv-source 0;  set azmux s3 ; nplc 1; test15
      9.4mV. 9.0mV

  > reset ; dcv-source -10;  set azmux s3 ; nplc 1; test15
      9.4mV. 9.0mV 8.0mV.

  Good.

  ----------
  reset ; dcv-source 10; nplc 1; test15
    +5.5mV.


    issue - perhaps ampfier draws the charge out of accumulation- if it goes from disconnected. to connected.

    woked got good signal on scope.

    but output is sometimes going negative.  eg. something discharges the cap after release.  bad timing or short.
    it's locked at -13VC
    or the amplifier isn't working.
    No. we measure -13V on the DMM.

  ----


  mar 10 2024.
    - signal collapses. if have 1Meg. scope probe attached.

    - switching into az switch.   no amplifier.  no bootin.

    reset ; dcv-source 10; nplc 1; test15
      -26mV.  -26mV.

    reset ; dcv-source 0; nplc 1; test15
      +6.8mV +6.4mV

    reset ; dcv-source -10; nplc 1; test15
      +40mV. +38mV.


    precharge switch - is too far from azmux???

    so next step - is to add bootin. and see if it helps.
*/



