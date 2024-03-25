/*
    it is good to code as repl, because it tests the repl. also.

*/


#include <stdio.h>
#include <assert.h>
#include <string.h>   // strcmp


#include <app.h>
#include <data/data.h>
#include <data/matrix.h>  // m_rows()
// #include <data/buffer.h>


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
    app_repl_statements(app, "        \
        flash cal read 123;           \
        reset;                        \
        dcv-source ref-hi;            \
        set k407 0;  set k405 1;       \
        set lomux s1;                 \
        nplc 10; set mode 7 ; azero s3 s8;  trig; \
      " );

    // start acquisition, generating interupts, which sets data ready flags, which we ignore for the moemnt. - with trig
    printf("change state\n");
    spi_mode_transition_state( app->spi, app->mode_current, &app->system_millis);


    // reset the input data buffer
    // data->buffer = buffer_reset( data->buffer, 5 );
    data_reset( app->data );

    // start the yield loop, and wait for buffer to fill
    while( m_rows(data->buffer ) < m_rows_reserve(data->buffer) ) {
      yield( yield_ctx);
    }

    // stop sample acquisition, perhaps unnecessary
    app->mode_current->trigger_source_internal = 0;
    spi_mode_transition_state( app->spi, app->mode_current, &app->system_millis);


    // print output
    m_foutput( stdout, data->buffer );


    // check_data( == 7.000 )  etc.
    return 1;
  }


  return 0;
}



