/*
    it is good to code as repl, because it tests the repl. also.

*/


#include <stdio.h>
#include <assert.h>
#include <string.h>   // strcmp

#include <lib2/util.h>  // UNUSED

#include <app.h>
#include <data/data.h>
#include <data/buffer.h>

#include <data/matrix.h>  // m_rows()




bool app_test20(
  app_t *app,
  const char *cmd,
  void (*yield)( void *),
  void *yield_ctx
) {
  assert(app);
  assert(app->magic == APP_MAGIC);
  assert(cmd);

  assert(yield);
  assert(yield_ctx);
/*
  // note that we access the buffer - to test if it is full. so it's correct to expose its functionality.
  data_t *data = app->data;
  assert(data);
  assert(data->magic == DATA_MAGIC);
  // assert(data->buffer); // may not be guaranteed, at start up first time
*/


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
        set k407 1;   set k405 1;  set k406 1;  \
        nplc 10; set mode 7 ; boot s3;  trig; \
      " );

    // check_data( == 7.000 )  etc.
    return 1;
  }


  // azmode- sample ref-lo - test noise
  else if( strcmp(cmd, "test20.5") == 0) {

    // sample ref-lo via dcv-source
    app_repl_statements(app, "        \
        flash cal read 123;           \
        reset;                        \
        dcv-source ref-lo;            \
        set lomux s1;                 \
        set himux s2;                 \
        set k407 1;   set k405 1;  set k406 1;  \
        nplc 10; set mode 7 ; azero s3 s8;  trig; \
        data show stats;  \
      " );

    // check_data( == 7.000 )  etc.
    return 1;
  }



  ////////////////////////////
  // noazero

  else if( strcmp(cmd, "test21") == 0) {

    // noaz sample ref-hi . on dcv. against star-lo
    app_repl_statements(app, "        \
        flash cal read 123;           \
        reset;                        \
        dcv-source ref-hi;            \
        set k407 1;   set k405 1;  set k406 1;  \
        nplc 10; set mode 7 ; noazero s3;  trig; \
      " );

    // check_data( == 7.000 )  etc.
    return 1;
  }

  else if( strcmp(cmd, "test22") == 0) {

    // noaz sample ref-hi noaz. on ch2 . himux/lomux - using ref-lo
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

    /* az sample ref-hi on ch1. with star-lo .   works but ntoe 40uV offset.  because s7 is star-lo. rataher than ref-lo.
        without gnd-current comp of the ref-lo.
        EXTR.but the Vos of the integrator input amplifer - still means - need an offset parameter in the calibration  model
        - regardless.  if star-lo == ref-lo using gnd current comp.
        - actually might get absorbed into the coefficients for the ref-currents. so 3 parameter model is enough.
        ---
        wow 460uV. with ltz1000.  mar 27. 2024.
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

    // az sample ref-hi on ch1, via the low mux, and ref-lo should be 7.000,000V.
    app_repl_statements(app, "        \
        flash cal read 123;           \
        reset;                        \
        dcv-source ref-hi;            \
        set k407 1;   set k405 1;  set k406 1;  \
        set lomux s1;                 \
        nplc 10; set mode 7 ; azero s3 s8;  trig; \
      " );

    // we have started generating interupts. - with trig
    // adc

    // data->buffer = buffer_reset( data->buffer, 5 );
    // data_reset( app->data );


    // check_data( == 7.000 )  etc.
    return 1;
  }





  else if( strcmp(cmd, "test25") == 0) {

    // az sample ref hi on ch2 himux/lomux,  azmux s1 s8. . _
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
        dcv-source header; set u1010 0b1011 ;    \
        set k407 0;   set k405 1;             \
        nplc 10; set mode 7 ; boot s3;  trig; \
      " );

    // check_data( == 7.000 )  etc.
    return 1;
  }



  if( strcmp(cmd, "test30") == 0) {

    app_repl_statements(app, "                \
        flash cal read 123;                   \
        reset;                                \
        dcv-source header; set u1010 0b0010;    \
        set k407 0;   set k405 1;             \
        nplc 10; set mode 7 ; boot s3;  trig; \
      " );

    // check_data( == 7.000 )  etc.
    return 1;
  }

  if( strcmp(cmd, "test31") == 0) {

    // start adc
    app_repl_statements(app, "                \
        flash cal read 123;                   \
        reset;                                \
        dcv-source header; set u1010 0b0011;    \
        set k407 0;   set k405 1;             \
        nplc 10; set mode 7 ; boot s3;  trig; \
      " );


    // check_data( == 7.000 )  etc.
    return 1;
  }


  return 0;
}



