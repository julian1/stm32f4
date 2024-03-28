/*
    it is good to code as repl, because it tests the repl. also.


  - it's not completely clear who should determine the yield function.
    eg. the caller passing it as a dependency of callee?
    but the callee knows what it needs . so could set up itself.
    in any case it doesn't really matter.

*/


#include <stdio.h>
#include <assert.h>
#include <string.h>   // strcmp


#include <app.h>
#include <data/data.h>
#include <data/matrix.h>  // m_rows()
// #include <data/buffer.h>


#include <mode.h>       // transition state


static void fill_buffer( app_t *app, void (*yield)( void *), void *yield_ctx)
{

  data_t *data = app->data;

  // start acquisition, generating interupts, which sets data ready flags, which we ignore for the moemnt. - with trig
  printf("change state\n");
  spi_mode_transition_state( app->spi, app->mode_current, &app->system_millis);


  // reset the input data buffer
  // data->buffer = buffer_reset( data->buffer, 5 );
  data_reset( data );

  // start the yield loop, and wait for buffer to fill
  while( m_rows(data->buffer ) < m_rows_reserve(data->buffer) ) {
    yield( yield_ctx);
  }

  // stop sample acquisition, perhaps unnecessary
  app->mode_current->trigger_source_internal = 0;
  spi_mode_transition_state( app->spi, app->mode_current, &app->system_millis);


  // print output
  // m_foutput( stdout, data->buffer );
}


static double m_get_mean( MAT *buffer )
{
  // take the mean of the buffer.
  MAT *mean = m_mean( buffer, MNULL );
  assert( m_is_scalar( mean ));

  double ret = m_to_scalar( mean);
  M_FREE(mean);
  return ret;
}






bool app_test41(
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

  data_t *data = app->data;
  assert(data);
  assert(data->magic == DATA_MAGIC);


  ////////////////////



  /*
    U1010
    AB   0b1011
    BC   0b0010
    AC   0b0011

  */
  if( strcmp(cmd, "test41") == 0) {

    if( !data->model_b) {
      printf("no cal model\n");
      return 1;
    }


    /* from test29
    // sample external cap on dcv in boot mode, no pc or az switching/ high-impedance.
    // with 10uF. cap  has leakage of several uV / s.
      TODO - rename external cap. to external pin-header. or port or somehting.
    */

    // we should use our current cal. not load a new one??
    // flash cal read 123;

    // setup a
    app_repl_statements(app, "                \
        reset;                                \
        dcv-source cap; set u1010 0b1011 ;    \
        set k407 0;   set k405 1;             \
        set lomux s1;                         \
        nplc 10; set mode 7 ; azero s3 s8;    \
        data show stats;  trig;               \
      " );

    double ar[ 5 ] ;

    for( unsigned i = 0; i < 5; ++i ) {

      app_repl_statements(app, " set u1010 0b1011; trig;");
      fill_buffer( app, yield, yield_ctx) ;
      double a  = m_get_mean( data->buffer );
      printf("a mean %lf\n", a );


      // setup b
      app_repl_statements(app, " set u1010 0b0010; trig;");
      fill_buffer( app, yield, yield_ctx) ;
      double b  = m_get_mean( data->buffer );
      printf("b mean %lf\n", b);


      // setup c.
      app_repl_statements(app, " set u1010 0b0011; trig;");
      fill_buffer( app, yield, yield_ctx) ;
      double c  = m_get_mean( data->buffer );
      printf("c mean %lf\n", c);

      double diff = a + b - c;
      ar[i] = diff;

      // printf("diff %.6lf\n", diff);
      printf("diff %.2lfuV\n", diff * 1e6);
    }
    printf("--\n");
    for( unsigned i = 0; i < 5; ++i ) {

      printf("diff %.2lfuV\n", ar[i ] * 1e6);
    }


    // check_data( == 7.000 )  etc.
    return 1;
  }




  return 0;
}


/*

mar 28.


8x 1.2V enneloup batteries in a battery-holder with taps, switched manually
Method - is sample AB for 10 readings, 10nplc, then BC (bottom half) , then AC (series ), take the means, and calculate the diff/delta.
repeat 5 times.
eg. diff = 4.8V + 4.8V - 9.6V


reduce resolution, change series rundown bias-resistor from 220R to 1k. and new cal.

> data cal show
Matrix: 3 by 1
row 0:     17.4986934
row 1:    -17.9358312
row 2:   -0.458200302
model_id    0
model_cols  3
stderr(V)   0.86uV  (nplc10)
res         0.115uV  digits      7.94 (nplc 10)

4.8
diff -3.50uV
diff -5.12uV
diff -4.66uV
diff -4.04uV
diff -1.76uV

2.4
diff -5.39uV
diff -4.11uV
diff -3.88uV
diff -0.94uV
diff -3.44uV

7.2
diff -5.95uV
diff -2.77uV
diff -3.09uV
diff -4.99uV
diff -2.00uV

3.6
diff -4.51uV
diff -6.62uV
diff -5.07uV
diff -4.34uV
diff -1.98uV


6.0
diff -8.43uV
diff -7.10uV
diff -3.93uV
diff -7.28uV
diff -2.49uV

6.0 repeat.
diff -8.09uV
diff -4.13uV
diff -3.95uV
diff -3.43uV
diff -4.59uV


1.2
diff -1.94uV
diff -2.22uV
diff 0.08uV
diff -2.07uV
diff -4.10uV

8.4
diff -3.94uV
diff -1.96uV
diff -2.35uV
diff -2.66uV
diff -2.43uV





  -------------
  after doing another cal.

  10nplc.  sample n=10, for each of AB (4.8V), BC (4.8V), AC (9.6V).
  diff -4.06uV
  diff -3.74uV
  diff -4.70uV
  diff -4.84uV
  diff -4.97uV
  diff -3.40uV

    at least it's stable.
    waitin

----

  mar 27.
    after upgrading ltz1000.
  1nplc.
    diff -1.67uV
    diff -5.79uV
    diff -9.30uVo
    diff -5.10uV
    diff -5.46uV

    doesn't look very good.

  10 nplc
    diff -2.06uV
    diff -3.51uV
    diff -3.00uV
    diff -2.43uV
    diff -3.52uV

      Ok. so noise is down a lot kkkkkk

  mar 26.

  Second attempt - is 8x 1.2V enneloup batteries in a battery-holder with centre tap, for the middle voltage.

  The cal done a few days ago, so there may be some temp drift on the cal coefficients.
  There seems to be some noise, which I think is from the adc ref - currently lt1021.
  So several runs are kind of needed to get any sense of it.

  Method - is to sample AB (top half battery pack) for 10 readings, then BC (bottom half) , then AC (series ), take the means, and calculate the diff/delta.
  eg. diff = 4.8V + 4.8V - 9.6V

  10nplc, sample n=10, azmode
  diff -3.97uV
  diff -2.53uV
  diff -1.90uVo
  diff -2.50uV
  diff -0.09uV
  diff -4.44uV

  1nplc, sample n=10, azmode
  diff 3.09uV
  diff -1.50uV
  diff 0.79uV
  diff 4.45uV
  diff 1.70uV
  diff -0.63uV
  diff 0.88uV

  So there's quite a bit of noise, and upgrading the adc ref may be the next step.
  It may make sense to do more interleaving for the different voltage more, althouth it needs all the serial peripheral state on the board to update.


*/





#if 0
    // az sample ref-hi on ch1, via the low mux, and ref-lo should be 7.000,000V.

    /* note, there's real confusion - with in order, and out of order repl statements.
      eg. flash cal read, and data show stats etc will be done in sequence, while mode update is out of bound
      we want to do the data_reset after the adc is running.
    */


    app_repl_statements(app, "        \
        flash cal read 123;           \
        reset;                        \
        dcv-source ref-lo;            \
        set k407 0;  set k405 1;      \
        set lomux s1;                 \
        nplc 10; set mode 7 ; azero s3 s8;  halt; \
        data show stats;              \
        data buffer size 30;          \
      " );

    // cal
    spi_mode_transition_state( app->spi, app->mode_current, &app->system_millis);

    // data->buffer = buffer_reset( data->buffer, 30);     // resise buffer
    // data_reset( data );                                 // reset

    // note - we could set the buffer, etc. and then do the trigger later.

    ice40_port_trigger_source_internal_enable();    // rename set/clear() ? better?
#endif


