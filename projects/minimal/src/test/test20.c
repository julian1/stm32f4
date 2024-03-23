
/*

*/


#include <stdio.h>
#include <assert.h>
#include <string.h>   // strcmp

// #include <mode.h>
#include <app.h>
// #include <ice40-reg.h>    // modes
// #include <lib2/util.h>    // msleep()




bool app_test20( app_t *app, const char *cmd)
{
  assert(app);
  assert(cmd);
  assert(app->mode_initial);

  // just some basic test modes. ratiometric.
  // first test should be az. mode for dcv.

  // TODO. add no az. cases


  if( strcmp(cmd, "test20") == 0) {

    // REVIW
    // test ref-hi noaz. on ch2 . himux/lomux - has +60uV offset. why?
    // was because of bad pre-charge switching . was sampling boot. now 7uV.
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

  if( strcmp(cmd, "test21") == 0) {

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


  ////////////////////////////

  if( strcmp(cmd, "test22") == 0) {

    // sample ref-hi on ch1. azero - works there is 40uV offset.  because s7 is star-lo. rataher than ref-lo.
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

  else if( strcmp(cmd, "test23") == 0) {

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

  else if( strcmp(cmd, "test24") == 0) {

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

  else if( strcmp(cmd, "test25") == 0) {

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


  else if( strcmp(cmd, "test26") == 0) {

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

  else if( strcmp(cmd, "test27") == 0) {

    // sample ref-hi on ch1, via the low mux, and ref-lo should be 7.000,000V.
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



