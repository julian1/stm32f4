

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





// prefix app_test15()
bool test15( app_t *app , const char *cmd/*,  Mode *mode_initial*/)
{
  assert(app);
  assert(cmd);
  assert(app->mode_initial);

  int32_t i0;
  double  f0;


  if( sscanf(cmd, "test15 %ld %lf", &i0, &f0 ) == 2) {

      /*
      // test charge-injection by charging to a bias voltage, holding, then entering az mode.
      // with az-mux also switching .
      // first argument - bias voltage.  10,-10,0
      // second argument is nplc.
      // note t

      */
      printf("test leakage and charge-injection from switching pre-charge switch at different biases\n");
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
      else {
        printf("bad current source arg\n");
        return 1;
      }

      // turn on accumulation relay     RON ROFF.  or RL1 ?  K606_ON
      j.first .K406_CTL  = RBOT;
      j.second.K406_CTL  = ROFF;    // don't need this....  it is 0 by default



      // set up fpga
      j.reg_mode =  MODE_DIRECT;
      j.reg_direct.himux2 = S1 ;    // s1 put dc-source on himux2 output
      j.reg_direct.himux  = S2 ;    // s2 rej.reg_directlect himux2 on himux output
      j.reg_direct.sig_pc_sw_ctl  = 1;  // turn on. precharge.  on. to route signal to az mux... doesn't matter.


      app_transition_state( app->spi, &j,  &app->system_millis );


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
      spi_ice40_reg_write32(app->spi, REG_MODE, MODE_AZ );  // mode 3. test pattern on sig

      // use direct_reg to set up az mode sampling.
      F  f;
      memset(&f, 0, sizeof(f));
      f.himux2 = SOFF ;
      f.himux  = SOFF ;
      f.azmux  = S6 ;    // s6 == normal LO for DCV, ohms.
      spi_ice40_reg_write_n(app->spi, REG_DIRECT, &f, sizeof(f) );


      if( ! nplc_valid( f0) ) {
        printf("bad nplc arg" );
        return 1;
      }

      uint32_t aperture = nplc_to_aper_n( f0, app->lfreq );
      printf("aperture %lu\n",   aperture );
      printf("nplc     %.2lf\n",  aper_n_to_nplc( aperture, app->lfreq ));
      printf("period   %.2lfs\n", aper_n_to_period( aperture ));

      spi_ice40_reg_write32(app->spi, REG_CLK_SAMPLE_DURATION, aperture );

    return 1;
    }

  return 0;
}



/*

  oct 6, 2023.
  max4053
  +10V dc bias
  1000nplc/off   1.0mV  0.2mV.    oct 8. 0.0mV.
  100nplc        0.2mV  0.2mV     oct 8. 0.3mV
  10nplc         1.0mV  1.2mV     oct 8. 0.7mV
  1nplc          20.5mV.  20.5mV  oct 8.  20mV

  max4053
  -10V dc bias
  wait for DA.
  1000nplc/off   3.8mV. 3.2mV    some DA from +10V test.  oct 8. 3.2mV.
  100nplc        2.5mV                                  oct 8 4.4mV.   3.9mV.
  10nplc         10mV. 10mV.                            oct 8 10mV.
  1nplc          56mV.  55mV.  56mV                     oct 8 56mV.

  max4053
  0V dc bias.
  1000nplc/off   1.3mV 1.2mV.                           oct 8. 1.6mV.
  100nplc        1.8mV                                  oct 8. 1.6mV
  10nplc         4.8mV                                  oct 8. 5.3mV.
  1nplc          38mV. 37mV.                            oct 8. 39mV.

  ------------
  oct 8.
  swap precharge polarity.
  1nplc          dc bias.
    0V            30mV.         32mV.  35mV.
    +10V          -2.6V  wow.
    -10V          +3.1V.


  I think it might make sense to try to trim - it with a capacitor.

  when the precharge phase is increased from 500us to 5ms.  sensitivity to dc-bias is reduced.

  +10V bias   1nplc    25mV. 25mV.
  0V   bias   1nplc    30mV. 30mV.
  -10V bias.  1nplc.   38mV. 38mV.

  spread == 38/25mV = 1.5x.

  using 3p trimmer.   measures 2.5p with LCR meter.

  bias.   accumulation.

  +10V     26mV.  26mV
  0V      32mV. 33mV.
  -10V    39mV. 37mV.

  10p.

  +10V    26mV.
  0V.    32mV.  33mV..
  -10V   40mV.

    ok. it's not working.
      needs to be sequenced in the state machine.
  */

/*
  ///////////////////
   oct 9.

   test15.
   revert to baseline. 500us precharge.
   max4053, 1nplc,
   +10V   21mV,  21mV
   0V     37mV.   37mV.
   -10V.  55mV.   58mV.

   I felt for-sure that tie-ing the bootin guard (currently floating) to gnd would do something, and change the loading.
   since there is quite a bit of copper sufrace area and proximity to mux-out.
   BOOTIN tied to gnd. also pin7 azmux. - no difference
   +10V.   20mV
   0V      39mV.  39mV.
   -10V.   55mV   56mV.

   remove cap C430. slowing pre-charge switching. - no difference.
   +10V    20.5mV.
   0V.     37mV.  39mV.
   -10V.   56mV.

   change lo source-resistor from 0R jumper. to 4.7k to match schematic.
   resoldered 4.7k. resistor again. with heat gun instead of soldering iron. much better. after only couple minutes.
    but no difference.
   +10V  19mV. 19mV.
   0V.   38mV.  38mV.
   -10V. 58mV.

   tie-off any unused azmux inputs (dci-lo, 4w-lo) to gnd. all azmux inputs are now defined.

   +10V  20mV.
   0V.   39mV.
   -10V  57mV.

   it is impressively consistent and stubbon.
   although it is still open to attack the magnitude of the charge, which will also trim the difference.
    --------
    - change goto in state machine.
    +10  19mV.
    0    37mV.
   -10V   57mV>
    ------

    As a test increasing max4053 supply rail, from 4V to 4.7V.  has a strong effect on charge injection

    +10   31mV.  31mV.
    0     51mV.  51mV.
    -10V  71mV.  74mV

    The datasheet is characterized down to 3V single-supply. and can operate as low as 2.7V.
    If I can find some zeners, I'll try reducing the supply rail.
    -------
    With max4053 supply at 2.70V.

    1nplc.
    +10V.      -12mV.  -12mV.            note negative .
    0V        7.5mV.  7mV.
    -10V.      26mV   27mV.

    leakage is still controlled.

    +10V   1000nplc/off -0.4mV
    -10V  1000nplc / off 3.8mV.

    But it is still a 26 - -12 = 39mV. difference. So the charge is the same, it is just centred differently.

    //////////////////////
    // oct 10.
    // test 15.

    baseline max4053, 2.7V supply. 500us precharge. 1nplc
    +10V        -12mV
    0V          7.0mV.
    -10V        26mV.


    temproary test for curiosity - using exaggerated 5ms precharge duration to observe effect.
    precharge duration has strong influence, which suggests something apart from the floating 4053 charge-injection, as Kleinstein notes.
    +10V.       -0.5mV  -0.6mV
    0V.         6.1mV   5.8mV.
    -10V        13.8mV  13.2mV.


    revert to 500us precharge.
    +10V      -12mV.
    etc.

    ------------------
    lift azmux out pin - so not

    leakage 1000nplc/off
    +10V.  -0.2mV.
    0V.     1.7mV. 1.0mV. 0.1mV.  ??
    -10V    2.2mV. 2.1mV.

      ---------------------
    1nplc
    +10V   2.6mV. 2.6mV      Wow. GOOD.
    0V.    5.0mV. 5.0mV.
    -10V.  8.5mV. 8.4mV.

    ---------
    repeat.
    +10V  2.2mV.
    0     4.7mV.
    -10.  8.9mV.   9.2mV


    repeat - before refactor.  oct 11.
    with only 1minute before settle.
    +10V   2.7mV.
    0V     5.1mV.
    -10V   8.1mV.



  /////////////////////////////
  // oct 11.

    after code refactor. moving test15() out of main.c
    +10V  2.8mV.
    0V    4.7mV.
    -10V   7.9mV.

    after adding amplifier. but not connected.
    +10V   2.1V.


    ok. try connecting using air wire.
    disconnect
    +10  1.8mV. -> 2.1mV. -> 2.5mV .  over 5 mintes. after soldering pin. ok.
    need to test amp again.
    ---------------


  amplifier populated in a simple configuration - jfe2140.  5V6 zener.  tle2071 op.

  An air-wire connects mux-out to the amplifier input.
  The board is *not* well cleaned due to a few bodge wires that limit access.
  But this also provides a good test of guard effectiveness.

  - with bootin tied to gnd.

  leakage 1000nplc/off
    +10V    -0.7mV. -0.8mV -0.8mV.        all at once. when there is a switch.
    0V      +1.1mV +1.1mV
    -10V     +5.4mV +5.3mV

  charge 1nplc
    +10V    -7.6mV. -9.2mV -9.3mV.  -8.9mV
    0V     +6.9mV +7.2V
    -10V    +22mV +23mV.


  - identical except bootin driver op added.
  (this copies the voltage on the copper fill under the lifted mux-out pin, and surrounding the amp input air-wire connection, as well as other sensitive amplifier pins)

  leakage. 1000nplc/off
  +10V    +0.7mV. +0.7mV.
  0V      2.2mV.  2.1mV
  -10V    5.2mV.  5.3mV

  charge 1nplc.
  +10V    -0.9mV  -1.0mV.
  0V      4.9mV  4.8mV. 4.8mV.
  -10V    +11.2mV  11.3mV

  So it looks like bootin is quite effective.

  ////////////////////////
  // oct 12.
  // want to repeat the test.
  // 4-5mins between.

  leakage. 1000nplc/off
  +10V    +0.0mV.  -0.1mV. -0.5mV.
  0V.      0.0V  0.6mV
  -10V.    2.5mV  1.2mV. 2.7mV

  charge 1nplc
  +10V    -1.3mV  -1.3mV.
  0V.      4.4mV  4.2mV.
  -10V    +10.6mV  10.5mV

  so leakage is trimmed. nice.

  clean board. about 5-10mins after - leakage is hi - when led hi - due to leakage mux-out or amplifier.

  leakage
  +10V    -0.1mV.
  0V       1.2  1.1mV.
  -10V     3.6mV  3.4mV

  charge 1nplc
  +10V     -2.0mV -2.3mV.
  0         +4.5mV.  4.4mV
  -10V      +11.9mV. 11.4mV.

  /////////
  // amplifier with mux populated, using G=1. feedback.
  // quieck test again. with amplifier working at G=1. concerned screwed up precharge duration.
  // seems. ok.

  leakage
  +10V.    0mV
  -10V     3.3mV  2.5mA.

  charge 1nplc
  +10V     -0.4mV.  -0.4mV
  0V.      +4.8mV. +4.9mV.
  -10V      +10.2mV.

  ////////////////
  // oct 14.   after heavy refactor bringing fpga into mode.
            quick test.
          expect should be the same.

  charge 1nplc
  +10V      -0.2mV
  0V        +4.6mV.
  -10V      +9.9mV.

  // and factor test15 code to do initial setup in state transition.
  charge 1nplc
  +10V      -0.6mV.
  0V        +4.6mV
  -10V      9.7mV

  //////////////////////////
  // oct 15.
  use lsk389.  swap jfe2140 for lsk389.
  4053 at 2.7V.
  and pmos for centre current mirror.
  expect larger caps, more input capacitance.
  board uncleaned.

  charge 1nplc
  +10V.   1.5mV.  1.4mV.
  0V      4.8mV.  5.0mV.
  -10V    7.2mV   12.2mV. 12.1mV. 12.1mV

  leakage 1000nplc / off
  +10V.  +1.6mV.  0.7mV.      (still had ER lead connected).
  0V       0.9mV. 0.8mV
  -10V    3.9mV. 3.0mV.

  So. think it's actually better. more centred.
  4053 at 4V
  charge 1nplc
  +10V    -3.3mV  -3.4mV
  0V.     -0.4mV  -0.3mV
  -10V    7.8mV   7.8mV

    difference 8 - -3 = 11mV.

  leakage.
  +10V    0.5mV. 0.1mV.
  0V.     2.0V  0.9mV
  -10V    2.5mV.  5.6mV. 4.1mV.


  //////////////////////
  change boot driver resistor from 10R to 0R. so that boot and himux out are the same voltage.
  max4053 at 4V.

  leakage  1000nplc / off
  +10V    -0.2mV. -0.3mV.
  0V      1.5mV.  1.2mV
  -10     2.7mV.  2.3mV   1.9mV (5mins later)


  charge 1nplc
  +10V    -36mV.  -34mV.  -35mV.               damn.  much worse.  so it looks like the difference was probably trimming.
  0V      -23mV   -23mV
  -10V     -20mV  -19mV.

  - the difference spread - is about the same. -35 - -20 = 15mV.

  hmmmm....
  So we try to trim the supply and see if it improves. but maybe that the supply trimming was going something else before.
  or try .
  //////
  change max4053 voltage to 2.68V. to see what happens.

  leakage  1000nplc / off
  +10V   -0.5mV. +0.4mV.
  0V    1.4mV  1.0mV.
  -10V.   +3.7mV.  2.3mV

  charge 1nplc
  +10V.    -22.9mV  -21.8mV. -21.8mV
  0V       -16.9mV. -17.6mV.
  -10V     -10.5mV. -11mV.

    - difference is ~= 12mV. improvement at lower voltage.

  so improves centre a lot and difference a bit.
  but cannot trim further with lower supply voltage.



  ///////////////////
  // oct 16.

  - max4053. 2.68V.  using 3v3 sod323 zener. same as yesterday
  charge 1nplc
  +10V   -21mV.
  0V.    -17mV.
  -10V   -11mV.

  - difference is ~= 10mV.  but centred negative.

  - sn74lv4053.   at 2.7V.
  leakage  1000nplc / off
  +10V    -0.1mV.  -0.3mV
  0V      0.9mV 1.7mV.  0.6mV.
  -10V    3.5mV 1.8mV. 2.1mV

  charge 1nplc
  +10V   9.4mV.  8.9mV.
  0V     12.4mV  12.3mV
  -10V   18.8mV   18.9mV

  - difference is around 10mV. but centred positive.

  - sn74lv4053.   at 4.2V. using 4.7V zener.
  leakage  1000nplc / off
  +10V    2.3mV.  1.1mV
  0V      2.3mV.  1.4mV
  -10V    3.5mV.  3.3V

  charge 1nplc
  +10V   6.8mV.  6.8mV
  0V     9.9mV  10.0mV.
  -10V   16.8mV  16.9mV.

  - same difference. small improvement in centre.
  - charge is always positive. would trimming work to shift. yes it should?
  - changng the pcb layout might change a lot of this.

  /////////////////
  - sn74lv4053.   at 5.47V  using 5V6 zener.
  leakage  1000nplc / off
  +10V    5.9mV  3.8mV.
  0V      4.8mV  7.3mV. 7.0mV
  -10V    5.3mV   8.2mV

  charge 1nplc
  +10V    6.4mV 6.0mV     a lot of this might just be leakage.
  0V      9.4mV  9mV
  -10mV   16mV.  15.4mV.

  - not much difference. leakage worse.

  ////////////////
  // Ok. try to add a cap and see what happens. 10k + 10p.

  - sn74lv4053.   at 5.47V  using 5V6 zener.
  - and 10k/ 10p cap.

  leakage  1000nplc / off     leakage is high- due to high supply.
  +10V    5.6mV
  0V.     7.8mV
  -10V    10mV.  3.5mV. 5.5mV.  depends on switch phase?

  charge 1nplc
  +10V    -6mV    -5.8mV      Ok. it worked to trim negatively. wow.
  0V      -1mV.  -1.1mV     Wow. looking good.
  -10V    +7.1mV. 7.0mV.

    great!.

  //////////////
  - sn74lv4053.   at 2.7V.   10p. compensation cap.
  - leakage is better at lower supply voltage.

  leakage  1000nplc / off
  +10V    -3.8mV  -0.3mV -2.2mV -0.3mV.   variation due to jfet leakage..
  0V.     -1mV.  -1.2mV
  -10V     1.5mV  0.2mV 1.7mV

  charge 1nplc
  +10V    -7.6mV -7.4mV
  0V.     -2mV.  -2.3mV.
  -10V    +5.7mV  5.9mV.

  ////
  // repeat . same as above.  hour later.

  leakage  1000nplc / off
  +10V    -0.9mV.  -1.5mV
  0V.     -0.5mV   -2.2mV
  -10V    0.9mV    1.9mV.

  charge 1nplc
  +10V    -6.1mV  -5.8mV.
  0V.     -2.6mV -2.4mV
  -10V    +5.8mV  6.2mV


  //////////////////
  ///////////
  // oct 18.
  // same. a couple days off.

  leakage  1000nplc / off
  +10V     2.0mV.  -0.2mV.   2mV.     - leakage is high when pc on/ amp jfets exposed
  0V.      0.8mV. 2.0mV. 0.9mV.
  -10V      5.0mV.  2.1mV  2.3mV.

  charge 1nplc
  +10V    -5.0mV.  -5.1mV.
  0V.    -1.0mV   -0.9mV.
  -10V    8.6mV  8.2mV.

  charge 10nplc
  +10V    +0.6mV. +0.1mV 0.1mV.   - wanders around 0V.
  0V.     0.1mV.  2.1mV 0.1mV.    - wanders. a bit over 10sec.
  -10V     4.9mV  6.2mV

  // remove azmux_hi_val
  // looks ok.

  /////////////////
  // oct 20.
  // remove the 10p. which slaps around the input signal.

  0V.
    with wire about 3mm.

    12.2mV.  12.8mV. 12.7mV   So quite good.

    can we bend the wire to change the effect.
    wire broke. removed.

    13.3mV.  13.3mV.

    air wire restored.
    12.4mV.  12.4mV.

    2.5p trimmer cap. measured with LCR meter.
    7.4mV.  7.4mV.

    so adding trimmer adjusts about 5mV. - around 5pA in the right direction. nice.
    needs another 5mV or so.

    - So. test +10,0,-10V for distribution.
    - then check what input looks like on the scope again.

    1nplc.
    10V.
      4.4mV  4.3mV.
    0V.
      7.2mV.  7.5mV
    -10V
      15.1mV. 14.5mV.


  - think the difference in cap size between discrete j201,sd5400,   would be a bit the reduced voltage range.
  datasheet for lv4053, IVL is 0.8V. and cmos could transition below that.

  - but problem.
      has the same effect of slapping the input around - although less because less capacitance.
      hmmmmm.

  - perhaps the issue is not charge injection  - it's async - it's leakage.




  /////////////////////////////
  // oct 21.
  // remove the compensation cap. clean around the 4053.
  // rewrite code, including support 0.1nplc.

  input dc-bias
  10V
    1000nplc/off    1.8mV. 0.6mV.   0.1mV.  1.4mV.   varation recorded depends on switch phase.
    10nplc          2.7mV. 2.7mV    2.7mV.
    1nplc           10mV   10.2mV.
    0.5nplc         18mV   18.9mV.

  0V.
    1000nplc/off    0.7mV. 0.7mV,   1.9mV
    10nplc          2.7mV  2.6mV
    1nplc           12.6mV 13.8mV.
    0.5nplc         24mV   24mV

  -10V.
    1000nplc/off    2.0mV  2.8mV 2.0mV
    10nplc          6.7mV  6.7mV
    1nplc           20mV   20mV. 20.5mV
    0.5nplc         32mV   34.2mV

*/


