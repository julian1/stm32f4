
/*
    test charge-injection by charging to a bias voltage, holding, then entering az mode.
    but only switching pre-charge switch.
    baseline for charge inection.

    azmux is held off to lower leakage through amplifier. so need external DMM.

  > reset ; dcv-source 10; nplc 1; test14

  ---------

  not sure - if can do self-diagnostic.
    when connect amplifier - then amplifier can discharge cap.

  EXTR.
    but what if - at finish we go to PC= boot.  let the amplifier follow, and then, connect  the azmux. then switch back to boot.

*/




#include <stdio.h>
#include <assert.h>
#include <string.h>   // strcmp

#include <mode.h>
#include <app.h>
#include <ice40-reg.h>    // modes
#include <lib2/util.h>    // msleep()



bool app_test14( app_t *app , const char *cmd)
{
  assert(app);
  assert(cmd);
  assert(app->mode_initial);


  if( strcmp(cmd, "test14") == 0) {

      printf("test leakage and charge-injection using MODE_PC_TEST switching pre-charge switch, at different input dc-bias and frequency\n");

      /*
        needs external dmm. to monitor at boot.
      */

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
      assert( mode.reg_direct.azmux_o == SOFF) ;
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
/*
      mode.reg_mode           = MODE_PC_TEST;
*/
      // 2 phase, azmux always off, but switch pc in one phase and not the other.
      // to simulate the real muxing an AZ signal between HI, and LO, where Lo doesn't get the PC on.
      // rather than 0b01  can use PC1
      mode.reg_mode = MODE_SA_MOCK_ADC;
      mode.sa.reg_sa_p_seq0 = (PCOFF << 4) | SOFF;        // 0b00
      mode.sa.reg_sa_p_seq0 = (PC1 << 4 )  | SOFF;        // 0b01
      mode.trigger_source_internal = 1;

      mode.first .K407        = LR_SET;          // disconnect dcv
      mode.reg_direct.leds_o  = 0b0010;    // advance led.   note. won't display in different mode.

      spi_mode_transition_state( app->spi, &mode, &app->system_millis);
      printf("sleep 10s\n");  // having a yield() would be quite nice here.
      msleep(10 * 1000,  &app->system_millis);

      // we might be able to manage the transition - to connecting amplifier up, by switching on pc first in direct mode.


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
  mar 15.
    new sequence controller. replacing pc-only controller,

    reset ; dcv-source 10; nplc 1; test14
      6mV. 6.4mV

    reset ; dcv-source 0; nplc 1; test14
      7.2mV  7.2mV

    reset ; dcv-source -10; nplc 1; test14
      7.8mV 7.6mV


  mar 9 2024.
    ==========
    no change. to yesterday.
    test14 azmux fitted, but turned off, and not switched. around 20 hours, after isopropyl cleaning, actually needed.
    excellent.  so there is a high sensitivity to isopropyl.
    more to self-document what am doing.
    - indicates board routing strategy looks ok for leakage. concerned about traces to trace and trace-to-pad -
        when route with just prepreg thickness. between them.
        eg. a +-18 or +-15V power supply routing directly underneath a row of soic pins of an opamp/analog mux.
        but it looks ok.

    1nplc
      test14 10 1           # ie. 10V dc bias, and 1nplc.
        +5.4mV. +5.9mV

      test14 0 1            # 0V dc-bias
        +6.0mV.  6.4mV

      test14 -10 1          # -10V dc-bias
        +6.6mV  7.1mV.


    10nplc
      test14 10 10
        +0.7mV  +0.5mV.

      test14 0 10
        +0.9mV 0.9mV

      test14 -10 10
        +1.7mV.  +1.7mV

    ==========


  mar 8 2024,
    azmux fitted.  but not switched. so just resenting high-impedance cmos input.
    after about 6hours.
    think we need to leave for longer

    test14 10 1   # ie. 10V dc bias, and 1nplc.
      -6mV.   -5.7mV  -5.0mV.  -4.7mV       -3mV  -1mV                     <- now minus. very odd.   still some leakage???
                                                                                think some due
    test14 0 1
      +8mV. +7.6mV  +7.3mV

    test14 -10 1
      +18mV.  +17mV.  +16mV. +15.5mV.

      - wow. not really what was expecting.
      - so there's some well of capacitance.k
      - due trace being longer?  seems weird.
      - so around 17mV. difference.



    -------------
    azmux not populated/fitted.  which we might expect a little bit of cap loading to gnd.
    very consistent 6mV or so.  positive charge.  exacty as expected/ and desired.
    after leaving overnight after soldering.

    > test14 10 1
      +6.14mV.  6mV.

    > test14 0 1
      +6.6mV.   6.5mV.

    > test14 -10 1
      +8.4mV (DA),  7.2mV.   6.8mV.

*/



