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
#include <data/buffer.h>


#include <mode.h>       // transition state






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

    // note, if we call data buffer reset in repl. it will be done out of order.
    // we want the data_reset after the adc is running.

    if( !data->model_b) {
      printf("no cal model - loading one\n");

      app_repl_statements(app, "flash cal read 123;");
      // return 1;
    }

    app_repl_statements(app, "        \
        reset;                        \
        dcv-source ref-lo;            \
        set k407 0;  set k405 1;       \
        set lomux s1;                 \
        nplc 10; set mode 7 ; azero s3 s8;  trig; \
      " );

    spi_mode_transition_state( app->spi, app->spi_4094, app->spi_ad5446, app->mode_current, &app->system_millis);

    data->show_stats = true;
    data->buffer = buffer_reset( data->buffer, 10 );
    data_reset( data );

    // check_data( == 7.000 )  etc.
    return 1;
  }




  return 0;
}


/*
mar 28


  Ok. resolution is too hi. it is in the 10s of nanovolts.  with our 226R resistor.

  az, 0 of 2 meas -0.000,000,71V
  az, 1 of 2 meas -0.000,000,72V
  az, 0 of 2 meas -0.000,000,74V
  az, 1 of 2 meas -0.000,000,77V

  stderr(V) 0.88uV  (nplc10)
  res       0.026uV  digits 8.59   (nplc10)
  calling spi_mode_transition_state()

  ----
  change bias to 1k. foil.


  model_id   0
  model_cols 3
  stderr(V) 0.86uV  (nplc10)
  res       0.115uV  digits 7.94   (nplc10)

  resolution looks better.
  az, 0 of 2 meas -0.000,000,24V mean(10) -0.0000014V, stddev(10) 0.62uV,
  az, 1 of 2 meas -0.000,000,42V mean(10) -0.0000013V, stddev(10) 0.63uV,
  az, 1 of 2 meas -0.000,000,42V mean(10) -0.0000014V, stddev(10) 0.70uV,
  az, 0 of 2 meas -0.000,000,42V mean(10) -0.0000009V, stddev(10) 0.47uV,
  az, 0 of 2 meas -0.000,000,55V mean(10) -0.0000012V, stddev(10) 0.54uV,
  az, 1 of 2 meas -0.000,000,64V mean(10) -0.0000011V, stddev(10) 0.56uV,
  az, 0 of 2 meas -0.000,000,64V mean(10) -0.0000010V, stddev(10) 0.52uV,
  az, 1 of 2 meas -0.000,000,69V mean(10) -0.0000010V, stddev(10) 0.46uV,
  az, 0 of 2 meas -0.000,000,86V mean(10) -0.0000013V, stddev(10) 0.59uV,
  az, 1 of 2 meas -0.000,000,86V mean(10) -0.0000015V, stddev(10) 0.47uV,
  az, 1 of 2 meas -0.000,000,90V mean(10) -0.0000016V, stddev(10) 0.47uV,
  az, 0 of 2 meas -0.000,000,95V mean(10) -0.0000013V, stddev(10) 0.71uV,



*/

