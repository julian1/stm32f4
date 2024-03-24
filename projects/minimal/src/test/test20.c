/*
    it is good to code as repl, because it tests the repl. also.

*/


#include <stdio.h>
#include <assert.h>
#include <string.h>   // strcmp

#include <app.h>


bool app_test20( app_t *app, const char *cmd)
{
  assert(app);
  assert(cmd);



  ////////////////////////////
  // boot


  if( strcmp(cmd, "test20") == 0) {

    // sample ref-hi on dcv in boot mode, no pc or az switching/ high-impedance.
    // adds Vos of the boot op-amp.
    // actually doesn't seem to have much Vos offset relative. to az/or noaz.
    // maybe 5uV opa-140 Vos..
    app_repl_statements(app, "        \
        flash cal read 123;           \
        reset;                        \
        dcv-source ref-hi;            \
        set k407 0;   set k405 1;                 \
        nplc 10; set mode 7 ; boot s3;  trig; \
      " );

    // check_data( == 7.000 )  etc.
    return 1;
  }



  ////////////////////////////
  // noazero

  else if( strcmp(cmd, "test21") == 0) {

    // test ref-hi noaz. on dcv. against star-lo
    app_repl_statements(app, "        \
        flash cal read 123;           \
        reset;                        \
        dcv-source ref-hi;            \
        set k407 0;   set k405 1;                 \
        nplc 10; set mode 7 ; noazero s3;  trig; \
      " );

    // check_data( == 7.000 )  etc.
    return 1;
  }

  else if( strcmp(cmd, "test22") == 0) {

    // test ref-hi noaz. on ch2 . himux/lomux - has +60uV offset. why?
    // was due to bad pre-charge switching codinig. was sampling boot eg. op-amp output. now 7uV.
    app_repl_statements(app, "        \
        flash cal read 123;           \
        reset;                        \
        dcv-source ref-hi;            \
        set lomux s1;                 \
        set himux s1;                 \
        nplc 10; set mode 7 ; noazero s1;  trig; \
      " );

    // check_data( == 7.000 )  etc.
    return 1;
  }

  ////////////////////////////
  // azero

  else if( strcmp(cmd, "test23") == 0) {

    /* sample ref-hi on ch1. azero - works there is 40uV offset.  because s7 is star-lo. rataher than ref-lo.
        without gnd-current comp of the ref-lo.
        EXTR.but the Vos of the integrator input amplifer - still means - need an offset parameter in the calibration  model
        - regardless.  if star-lo == ref-lo using gnd current comp.
        - actually might get absorbed into the coefficients for the ref-currents. so 3 parameter model is enough.
    */

    app_repl_statements(app, "        \
        flash cal read 123;           \
        reset;                        \
        dcv-source ref-hi;            \
        set k407 0;   set k405 1;                 \
        nplc 10; set mode 7 ; azero s3 s7;  trig; \
      " );

    // check_data( == 7.000 )  etc.
    return 1;
  }


  ////////////////////

  else if( strcmp(cmd, "test24") == 0) {

    // sample ref-hi on ch1, via the low mux, and ref-lo should be 7.000,000V.
    app_repl_statements(app, "        \
        flash cal read 123;           \
        reset;                        \
        dcv-source ref-hi;            \
        set k407 0;  set k405 1;       \
        set lomux s1;                 \
        nplc 10; set mode 7 ; azero s3 s8;  trig; \
      " );

    // check_data( == 7.000 )  etc.
    return 1;
  }

  else if( strcmp(cmd, "test25") == 0) {

    // sample ref hi on ch2 himux/lomux,  azmux s1 s8. . _
    app_repl_statements(app, "        \
        flash cal read 123;           \
        reset;                        \
        dcv-source ref-hi;            \
        set lomux s1;                 \
        set himux s1;                 \
        nplc 10; set mode 7 ; azero s1 s8;  trig; \
      " );

    return 1;
  }

  /* everything with sources - should probably use ref-lo.
    everything with external voltages - should probably use star-lo.
  */

  //////////////////////////
  // sampple dac.

  else if( strcmp(cmd, "test26") == 0) {

    // sample dac on ch1.
    app_repl_statements(app, "        \
        flash cal read 123;           \
        reset;                        \
        dcv-source dac 0x3fff;        \
        set k407 0;    set k405 1;    \
        nplc 10; set mode 7 ; azero s3 s7;  trig; \
      " );

    // check_data( == 7.000 )  etc.
    return 1;
  }

  /*
      for setting dcv-source,dac,ref.
      i think we want to leave input relays unconnected. just engage if usinig dcv.
  */


  else if( strcmp(cmd, "test27") == 0) {

    // sample dac on ch2.
    // we should probably switch the inupt relays off.
    app_repl_statements(app, "        \
        flash cal read 123;           \
        reset;                        \
        dcv-source dac 0x3fff;        \
        set lomux s1;                 \
        set himux s1;                 \
        nplc 10; set mode 7 ; azero s1 s8;  trig; \
      " );

    return 1;
  }


  //////////////////////////////////

  else if( strcmp(cmd, "test28") == 0) {

    // ratiometric/ 4 cycle - ref-hi through both input channels/separate pre-charge switches.
    /*
    ratio, 3 of 4 meas 0.999,999,9
    ratio, 0 of 4 meas 1.000,000,0
    ratio, 1 of 4 meas 1.000,000,1
    ratio, 2 of 4 meas 1.000,000,0
    ratio, 3 of 4 meas 0.999,999,9
    ratio, 0 of 4 meas 1.000,000,1
    */


    // ch1 has voltage.
    app_repl_statements(app, "        \
        flash cal read 123;           \
        reset;                        \
        dcv-source ref-hi;            \
        set k407 0;  set k405 1;       \
        set himux s1;                 \
        set lomux s1;                 \
        nplc 10; set mode 7 ; ratio ;  trig; \
      " );

    // check_data( == 7.000 )  etc.
    return 1;
  }


  ///////////////////////////////

  if( strcmp(cmd, "test29") == 0) {

    // sample external cap on dcv in boot mode, no pc or az switching/ high-impedance.
    // with 10uF. cap  has leakage of several uV / s.
    // >  set u1010 0b1011
    // > set u1010 0b1110

    app_repl_statements(app, "                \
        flash cal read 123;                   \
        reset;                                \
        dcv-source cap; set u1010 0b1011 ;    \
        set k407 0;   set k405 1;             \
        nplc 10; set mode 7 ; boot s3;  trig; \
      " );

    // check_data( == 7.000 )  etc.
    return 1;
  }




  return 0;
}



/*
  > flash cal read 123; reset ; noazero s1 ; set k406 1 ; set k407 1; set himux s1 ; dcv-source ref-hi ; nplc10; set mode 7 ; trig
    and ref.lo
    flash cal read 123; reset ; noazero s1 ; set k406 1 ; set k407 1; set himux s1 ; dcv-source ref-lo ; nplc10; set mode 7 ; trig

    - but the azero version doesn't work.
      looks like there is 7V coming out of the precharge switch ok.

  >  flash cal read 123; reset ; azero s1 s8 ; set k406 1 ; set k407 1; set himux s1 ; set lomux s1 ;  dcv-source ref-hi ; nplc10; set mode 7 ; trig

        az, 1 of 2 meas 7.000,059,0V
        works. it was the lo that was floating.

        but note. the 60u offset. sampling through the hi-muxes.

    # sample dcv-source on s1 s8. through the relay.
    >  flash cal read 123; reset ; azero s1 s8; set k405 1 ;   set lomux s1 ;  dcv-source ref-hi ; nplc10; set mode 7 ; trig

    # but when we do az cross it's very good.
    > flash cal read 123; reset ; azero cross; set k406 1 ; set k407 1; set himux s1 ; set lomux s1 ;  dcv-source ref-hi ; nplc10; set mode 7 ; trig

      az, 1 of 2 meas 6.999,997,3V
*/



/*



*/



