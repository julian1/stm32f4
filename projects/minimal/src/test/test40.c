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
      printf("no cal model\n");
      return 1;
    }

    app_repl_statements(app, "        \
        reset;                        \
        dcv-source ref-lo;            \
        set k407 0;  set k405 1;       \
        set lomux s1;                 \
        nplc 10; set mode 7 ; azero s3 s8;  trig; \
      " );

    spi_mode_transition_state( app->spi, app->mode_current, &app->system_millis);

    data->show_stats = true;
    data->buffer = buffer_reset( data->buffer, 10 );
    data_reset( data );

    // check_data( == 7.000 )  etc.
    return 1;
  }




  return 0;
}


