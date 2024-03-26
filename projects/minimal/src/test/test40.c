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






bool app_test40(
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

  if( strcmp(cmd, "test40") == 0) {

    // az sample ref-hi on ch1, via the low mux, and ref-lo should be 7.000,000V.
    app_repl_statements(app, "        \
        flash cal read 123;           \
        reset;                        \
        dcv-source ref-hi;            \
        set k407 0;  set k405 1;       \
        set lomux s1;                 \
        nplc 10; set mode 7 ; azero s3 s8;  trig; \
      " );

    // ok.
    fill_buffer( app, yield, yield_ctx) ;
    printf("mean %lf\n", m_get_mean( data->buffer ));


    // charnge params.
    app_repl_statements(app, " nplc 1;   trig;");

    // collect other data
    fill_buffer( app, yield, yield_ctx) ;
    printf("mean %lf\n", m_get_mean( data->buffer ));

    // check_data( == 7.000 )  etc.
    return 1;
  }



  /*
    AB   0b1011
      // boot, 0 of 1 meas 4.830,793,8V
      // boot, 0 of 1 meas 4.830,819,8V

    BC   0b0010
    // boot, 0 of 1 meas 4.826,508,8V
    // boot, 0 of 1 meas 4.826,530,8V

    A    0b0011

  */


  else if( strcmp(cmd, "test41") == 0) {

    /* from test29
    // sample external cap on dcv in boot mode, no pc or az switching/ high-impedance.
    // with 10uF. cap  has leakage of several uV / s.
      TODO - rename external cap. to external pin-header. or port or somehting.
    */

    // setup a
    app_repl_statements(app, "                \
        flash cal read 123;                   \
        reset;                                \
        dcv-source cap; set u1010 0b1011 ;    \
        set k407 0;   set k405 1;             \
        set lomux s1;                 \
        nplc 1; set mode 7 ; azero s3 s8;  trig; \
      " );

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

    printf("diff %lf\n", diff);

    printf("diff %.2lfuV\n", diff * 10e6);

    // check_data( == 7.000 )  etc.
    return 1;
  }




  return 0;
}


/*

  whoahh.

c mean 9.659951
diff -0.00
diff -0.47uV


c mean 9.659957
diff -0.00
diff -24.44uV

c mean 9.659957
diff 0.00
diff 2.78uV

nplc 1
diff -7.83uV
diff 7.11uV
diff -13.50uV
diff -23.06uV
diff -9.92uV
diff 8.30uV

*/


#if 0
flash lock
done
aperture 4000000
nplc     10.00
period   0.20s
trigger set
change state
az, 0 of 2
az, 1 of 2
az, 0 of 2
az, 1 of 2 meas 4.832,098,0V
az, 0 of 2 meas 4.832,098,2V
az, 1 of 2 meas 4.832,098,1V
az, 0 of 2 meas 4.832,097,3V
az, 1 of 2 meas 4.832,096,9V
az, 0 of 2 meas 4.832,096,9V
az, 1 of 2 meas 4.832,096,8V
az, 0 of 2 meas 4.832,096,1V
az, 1 of 2 meas 4.832,096,1V
az, 0 of 2 meas 4.832,096,8V
a mean 4.832097
trigger set
change state
az, 0 of 2
az, 1 of 2
az, 0 of 2
az, 1 of 2 meas 4.827,854,6V
az, 0 of 2 meas 4.827,854,3V
az, 1 of 2 meas 4.827,854,0V
az, 0 of 2 meas 4.827,854,1V
az, 1 of 2 meas 4.827,854,1V
az, 0 of 2 meas 4.827,853,8V
az, 1 of 2 meas 4.827,853,7V
az, 0 of 2 meas 4.827,853,9V
az, 1 of 2 meas 4.827,853,8V
az, 0 of 2 meas 4.827,852,9V
b mean 4.827854
trigger set
change state
az, 0 of 2
az, 1 of 2
az, 0 of 2
az, 1 of 2 meas 9.659,951,9V
az, 0 of 2 meas 9.659,950,2V
az, 1 of 2 meas 9.659,950,2V
az, 0 of 2 meas 9.659,952,6V
az, 1 of 2 meas 9.659,952,0V
az, 0 of 2 meas 9.659,949,4V
az, 1 of 2 meas 9.659,948,6V
az, 0 of 2 meas 9.659,952,1V
az, 1 of 2 meas 9.659,951,9V
az, 0 of 2 meas 9.659,951,7V
c mean 9.659951
diff -0.00
diff -0.47uV
#endif

