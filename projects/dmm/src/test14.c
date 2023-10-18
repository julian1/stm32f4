

#include <stdio.h>    // printf, scanf
#include <string.h>   // strcmp, memset
#include <assert.h>

// lib
#include "util.h"       // msleep()



// local
#include "reg.h"
#include "mux.h"        // mux_ice40()
#include "spi-ice40.h" // spi_ice40_reg_write32()


#include "app.h"

// modes of operation.
#include "mode.h"





bool test14( app_t *app , const char *cmd/*,  Mode *mode_initial*/)
{
  assert(app);
  assert(cmd);
  assert(app->mode_initial);


    uint32_t u1;  // rename
    int32_t i0;


    if( sscanf(cmd, "test14 %ld %lu", &i0, &u1 ) == 2) {

        /*

          test charge-injection by charging to a bias voltage, holding, then entering az mode.
          but only switching pre-charge switch.
          baseline for charge inection.

          kind needs to be rewritten. after changes to do_transition.

        */
        printf("test leakage and charge-injection from switching pre-charge switch and not azmux\n");
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

        // turn on accumulation relay     RON ROFF.  or RL1 ?  K606_ON
        j.first .K406_CTL  = 0b01;
        j.second.K406_CTL  = 0b00;    // don't need this....  it is 0 by default

        do_4094_transition( app->spi, &j,  &app->system_millis );

        /////////////////
        // make sure we are in direct mode.
        mux_ice40(app->spi);
        spi_ice40_reg_write32(app->spi, REG_MODE, MODE_DIRECT );

        // now control the hi mux.
        F  f;
        memset(&f, 0, sizeof(f));
        f.himux2 = S1 ;    // s1 put dc-source on himux2 output
        f.himux  = S2 ;    // s2 reflect himux2 on himux output
        f.sig_pc_sw_ctl  = 1;  // turn on. precharge.  on. to route signal to az mux... doesn't matter.
        spi_ice40_reg_write_n(app->spi, REG_DIRECT, &f, sizeof(f) );

        ////////////////////////////
        // so charge cap to the dcv-source
        // and let settle 10sec. for DA....
        printf("sleep 10s\n");  // having a yield would be quite nice here.
        msleep(10 * 1000,  &app->system_millis);

        /////////////////
        // now change to az mode.
        printf("changing to az mode.\n");  // having a yield would be quite nice here.
        // setup az mode
        mux_ice40(app->spi);
        // spi_ice40_reg_write32(app->spi, REG_MODE, MODE_AZ );  // mode 3. test pattern on sig
        spi_ice40_reg_write32(app->spi, REG_MODE, MODE_PC );  // mode 3. test pattern on sig

        //////////////
        // use direct register - for the lo sample, in azmode.
        memset(&f, 0, sizeof(f));
        f.himux2 = SOFF ;
        f.himux  = SOFF ;
        f.azmux  = SOFF ;
        spi_ice40_reg_write_n(app->spi, REG_DIRECT, &f, sizeof(f) );

        // set reg_direct2 controlling azmux hi val, the same. so only switching the pc switch.
        // spi_ice40_reg_write_n(app->spi, REG_DIRECT2, &f, sizeof(f) );


        assert(u1 == 1 || u1 == 10 || u1 == 100 || u1 == 1000); // not really necessary. just avoid mistakes

         uint32_t aperture = nplc_to_aper_n( u1 );

        printf("aperture %lu\n",   aperture );
        printf("nplc     %.2lf\n",  aper_n_to_nplc( aperture ));
        printf("period   %.2lfs\n", aper_n_to_period( aperture ));

        spi_ice40_reg_write32(app->spi, REG_CLK_SAMPLE_DURATION, aperture );

        return 1;
      }


  return 0;
}



  /// EXTR.   AHHHH. a reason the leakage and charge-accumulation results differ - voltage. 5V6 zener for 4.8V..  versus using 5V1 zener, may have been less..
  // can check the codes.


  // with 5V6 zener.  so giving boot supply rail of 4.7V.

  // 0ct 3.
  // test14 at 10V bias.
  // 100nplc / 2000ms   == +32mV.      leakage and charge injection.  similar to no switching.
  // 10nplc / 200ms     == +39mV.
  // 1nplc /20ms.       == +86mV.

  // same - new date. so we could test. soldering a lower voltage 4053. on.

  // oct 4.
  //              20s.  == +28mV     29mV.  23mV.  (may be a difference if pc switch starts on/of )
  // 100nplc / 2000ms   == +22mV.  22mV
  // 10nplc, 200ms.     == +29mV.   29mV.
  // 1nplc,  20ms.      == 82mV.   84mV.

  ////////////
  // 6v2 zener.  with 5.5V out.   5mins after soldering.
  //              20s  ==  47mV.
  // 100nplc / 2000ms   == 59mV.
  // 10nplc, 200ms.     == 43mV.  51mV.
  // 1nplc,  20ms.      == 108mV  107mV. 100mV (20mins after soldering).

  // so it's worse with higher voltage...
  // all incredibly strange.
  // Or perhaps we used a lower voltage zener for previous tests??  ta is our memory

  // 5.1V zener.  weird.   measure 4.7V across zener. giving 4V boot rail <- weird.  code D3L
  // 20s                29mV.
  // 100nplc / 2000ms.  24mV
  // 10nplc / 200ms.    31mV. 32mV.
  // 1nplc / 20ms       81mV. 81mV. 80mV.

  // ok. so not a lot of difference.
  // we should probably replace the dg508.  with a 1208.  that we received.

  // use 1208.  replacing maxmim 508. - don't actually expect much difference.after soldering.

  // 1000nplc / 20s     18mV.  28mV.  29mV.
  // 100nplc / 2000ms.  25mV   23mV
  // 10nplc / 200ms.    30mV  30mV
  // 1nplc / 20ms       78mV. 79mV.

  // after a few hours.

  /*


    Still exploring cmos switches for a bit,

    Briefly for sn74lv4053a one difference with the previous tests - is the zener used to set the boot supply rail.
    but tests show that a bootstrap supply rail between 4V to 5.5V doesn't matter much for leakage or charge injection.
    Also tried another sn74lv4053a, purchased a few years apart from the one used for initial tests, but with the same result.
    So i am not sure how to explain the discrepancy with previous test results.

    Running az modulation. all muxes fitted.
    DC accumulation on 10nF/ over 10s.

    test14.
    sn74lv4053a
    +10V dc bais
    1000nplc/off   20mV. 18mV.
    100nplc/2s     17mV. 17mV.
    10nplc/200ms   21mV. 22mV.
    1nplc/20ms     35mV. 73mV.  70mV.   large measured difference. odd. but was definltey there.


    But max4053 looks a lot better,
    I almost wasn't going to bother re-testing it, based on past resulsts.
    Identical setup as above - accumulation on 10nF/ 10s.

    max4053
    +10V dc bias.
    1000nplc/off   0.3mV. 0.5mV
    100nplc        0.8mV.
    10nplc         3.8mV.   3.6mV.
    1nplc          30mV.   28mV.

    max4053
    -10V dc bias.
    leave five minutes for +4.5mV/10s. cap DA to settle.
    1000nplc/off   2.5mV  2.8mV   - oct 8  2.3mV.
    100nplc        3.0mV. 3.3mV   - oct 8.  2.3mV
    10nplc         5.6mV.  5.7mV  - oct 8. 5.2mV.
    1nplc          30mV   30mV.   - oct 8  29mV.

    max4053
    0V dc bias.
    1000nplc/off   0.8mV.
    100nplc        1.0mV.  1mV.
    10nplc         3.8mV.  3.6mV.
    1nplc          28mV.

    leakage is more controlled -  <1pA for +10V and 0V, and <3pA for -10V dc-bias.

    for charge injection
    ie. 1nplc == 20ms.  10s/0.02s == 500 cycles.
    this is 30mV / 500 == 0.06pC .
    if I have the units correct, through full-cycle switch.

    The above tests were done with the azmux held off, with only the pre-charge switch switching.
    this would eliminate/isolate any leakage through the amplifer input jfets (if fitted) .


    test15.
    When the azmux also changed to for normal sampling beetween PC-OUT (S1) and LO (S6), the result is similar - eg good.

    max4053
    0V dc bias.
    1000nplc/off   1.3mV 1.2mV.
    100nplc        1.8mV
    10nplc         4.8mV
    1nplc          38mV. 37mV.


  ///////////////////////////
  // oct 18.
        using new MODE_PC. and eliminating reg_direct2.
  //

  test14.  test only pc switches - himux and azmux off

  10V.
    1000nplc/off    -0.2mV  -0.2mV.
    1nplc            -1.9mV -2.0mV
  0V.
    1000nplc/off    +0.7mV +0.9mV.
    1nplc            0.0mV  0.1mV.
  -10.
    1000nplc/off     +2.4mV. +2.5mV
    1nplc            +3.7mV. +3.5mV.


  */



