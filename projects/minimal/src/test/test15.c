

/*
  same as test14.  except switch the azmux also.

    test charge-injection by charging to a bias voltage, holding, then entering az mode.
    with az-mux also switching .

  > reset ; dcv-source 10; nplc 1; test15

    only thing that really changes with test14. is the actual mode.  MODE_AZ instead of MODE_PC
    lot of opportunity to refactor.

*/

#include <stdio.h>
#include <assert.h>
#include <string.h>   // strcmp

#include <mode.h>
#include <app.h>
#include <ice40-reg.h>    // modes
#include <lib2/util.h>    // msleep()



static void test(app_t *app)
{
  // assert( 0 ); // FIXME

  printf("test leakage and charge-injection by switching pre-charge/azmux at different input dc-bias and frequency\n");


  /* assume dcv-source and nplc have been set up on mode already.
    we could verify with some checks.  */

  _mode_t mode = *app->mode_current;

  ////////////////////////
  // phase 1, soak/charge accumulation cap

  // setup input relays.
  mode.first .K407 = LR_SET;    // select dcv-source on ch1.
  mode.first .K405 = LR_SET;     // select ch1. to feed through to accum cap.
  mode.first .K406 = LR_RESET;   // select accum cap



  // set up fpga - with direct mode - for soak/charge of accum cap.
  mode.reg_mode     =  MODE_DIRECT;

  /*
    in direct_mode we manually set azmux to the azmux-hi-val while soaking/charging the accumulation capacitor.
    this keeps amplifier following input. and avoids case of amplifier oing out-of-range on floating input,
    and then discharging the accum cap, when coming back into stable-state, on entering az mode.
  */
  // TODO fixme, review
  // why not just set it vias direct register externally?
  // it will almost always be dcv/S3.
  mode.reg_direct.azmux_o = mode.sa.reg_sa_p_seq0;

  assert( mode.reg_direct.azmux_o == S3);       // can relax this to the other input later
  assert( mode.reg_direct.sig_pc_sw_o == 0b00 );

  mode.reg_direct.leds_o = 0b0001;        // phase first led turn on led, because muxinig signal.

  spi_mode_transition_state( app->spi, &mode, &app->system_millis);
  printf("sleep 10s\n");  // having a yield would be quite nice here.
  msleep(10 * 1000,  &app->system_millis);


  ////////////////////////
  // phase 2, discocnnect dcv-source, enter az mode
  //           and switch into precharge mode.

  printf("mode to az\n");
  printf("disconnect dcv-source,  and observe drift\n");



  mode.reg_mode = MODE_SA_MOCK_ADC;

  // july 2024 - note that this is all default.
  // need to review. make sure that pc switches first, then azmux.
  mode.sa.reg_sa_p_seq_n = 2,

// +6.5mV..
  mode.sa.reg_sa_p_seq0 = (0b01 << 4) |  S3,         // dcv
  mode.sa.reg_sa_p_seq1 = (0b00 << 4) | S7,         // star-lo

/*  +6.5mV
  mode.sa.reg_sa_p_seq0 = (PCOFF << 4) | S7 ;        // lo.
  mode.sa.reg_sa_p_seq1 = (PC01 << 4 )  | S3 ;    // dcv-source
*/

  // trigger start of sample acquisition
  mode.trig_sa = 1;

  mode.first .K407        = LR_RESET;   // disconnect dcv


  // mode.reg_direct.leds_o  = 0b0010;    // won't display when running.

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

}



bool app_test15( app_t *app , const char *cmd)
{
  assert(app);
  assert(cmd);
  assert(app->mode_initial);


  if( strcmp(cmd, "test15") == 0) {

    test(app);


      // returning,  will revert back to mode_current state.
      return 1;
    }


  return 0;
}


/*
  july 2024.
    after adding full amplifier. board not cleaned.

    we have not properly guarded the azmux output.  could be leakage from u410,p8.
    seems to have increased.
    need to review the switching cycle. since we changed.
    -------
    EXTR. - it is not the pc switching.  because test14. just switches the pc switch. and there is  no strong leakage/ bias.
                  - actually it is close to test14.
    EXTR. - it could just be amplifier input leakage. need another test. like test 12. without any whitchwithout
    EXTR. - should try copying test14. and just change the seq0, seq1.


  >  reset ; dcv-source 10; nplc 1; test15
    0.7mV.  0.7mV.

  > reset ; dcv-source 0; nplc 1; test15
    +4mV.  +4mV.  +4mV.

  reset ; dcv-source -10; nplc 1; test15
    8mV. 7mV. 7mV.

    - could be leakage from jfet input.
    - we need another test, like test12.  but with azmux switched on, through to the amplifier input.
*/




/*

A quick update,

One goal for a new board, is to change the copper features to improve the input switching parasitics, based on the previous experiments.
The design now features traditional ring guards wherever leakage needs to be controlled.
As well as a copper fill at BOOTIN potential, underneath the AZ mux, and surrounding the azmux node and amplifer jfets, to reduce capacitive loading.


Lekage - input leakage can be tested by first charging 10n cap for 10sec, then turn off azmux and observe leakage by sampling boot.
leakage looks very controlled.

    > reset ; dcv-source 10; test05
        0.57mV 0.54mV

    > reset ; dcv-source 0; test05
        0.7mV 0.8mV

    > reset ; dcv-source -10; test05
        1.2mV 1.1mV.


Precharge switching,
Change injection is constant at different input bias voltages - as expected due to the switch bootstapping.
This can be improved/trimmed, by lowering the supply voltage on 4053, and trimming VEE relative to BOOT, with a bipolar current source.
But I haven't bothered for the moment.
Accumulated charge injection, nplc 1, on 10nF for 10s, using lv4053,

    > reset ; dcv-source 10; nplc 1; test14
      6mV. 6.4mV

    > reset ; dcv-source 0; nplc 1; test14
      7.2mV  7.2mV

    > reset ; dcv-source -10; nplc 1; test14
      7.8mV 7.6mV


Normal Az switching,
The copper fill at BOOTIN (copying the AZ input voltage), under the azmux works to suppress the capacitive loading of the switch-node output.

    > reset ; dcv-source 10; nplc 1; test15
      5.0mV 4.95mV

    > reset ; dcv-source 0; nplc 1; test15
      7.5mV 8.2mV

    > reset ; dcv-source -10; nplc 1; test15
      8.15mV.


Board has two distinct input channels, with separate pre-charge switches.
So four-cycle RM and AG (to compensate thermal walk of a high-gain amplifier) functions are possible,

ratio of ref-hi, 10nplc, sampled on two separate channels,
ratio, 3 of 4 meas 0.999,999,9 mean(10) 0.9999999V, stddev(10) 0.06uV,
ratio, 0 of 4 meas 0.999,999,9 mean(10) 0.9999999V, stddev(10) 0.06uV,
ratio, 1 of 4 meas 0.999,999,9 mean(10) 0.9999999V, stddev(10) 0.06uV,
ratio, 2 of 4 meas 1.000,000,1 mean(10) 0.9999999V, stddev(10) 0.07uV,
ratio, 3 of 4 meas 1.000,000,0 mean(10) 0.9999999V, stddev(10) 0.07uV,
ratio, 0 of 4 meas 1.000,000,0 mean(10) 0.9999999V, stddev(10) 0.07uV,









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



