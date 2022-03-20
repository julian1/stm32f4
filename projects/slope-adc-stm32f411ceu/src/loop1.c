


// #include <stdbool.h>

#include "assert.h"
// #include "streams.h"  // printf
#include "format.h" // format_bits
#include "usart.h"   // usart_flus()
#include "util.h"   // system_millis


#include "regression.h"
// #include <matrix.h>
#include "app.h"





static bool push_buffer1( MAT *buffer, unsigned *i, double value)
{

  assert(buffer);
  assert(i);
  assert( m_cols(buffer) == 1 );

  bool full = false;
  if(*i == m_rows(buffer)) {
    full = true;
    *i = 0;
  }

  m_set_val( buffer, *i, 0, value );
  ++(*i);

  return full;
}



static void push_buffer3( MAT *buffer, unsigned *i, double v0, double v1, double v2)
{
  // unused.
  assert(buffer);
  assert(i);
  assert( m_cols(buffer) == 3 );

  if(*i == m_rows(buffer)) {
    assert(0);
  }

  m_set_val( buffer, *i, 0, v0 );
  m_set_val( buffer, *i, 1, v1 );
  m_set_val( buffer, *i, 3, v2 );
  ++(*i);
}




static void push_stats_buffer( app_t *app , double value )
{
  //////////////
  // report value....
  // printf("-----------------");
  char buf[100];
  printf("push_stats_buffer %sV ", format_float_with_commas(buf, 100, 7, value));
  // printf("\n");


  push_buffer1( app->stats_buffer, &app->stats_buffer_i, value);


  // m_foutput(stdout, app->stats_buffer );


  MAT *stddev = m_stddev( app->stats_buffer, 0, MNULL );
  assert( m_cols(stddev) == 1 && m_rows(stddev) == 1);
  double stddev_ = m_get_val( stddev, 0, 0);
  M_FREE(stddev);


  printf("stddev(%u) %.2fuV, ", m_rows(app->stats_buffer), stddev_  * 1000000 );   // multiply by 10^6. for uV

  printf("\n");
}









static void process( app_t *app, double predict )
{
  // deep nested functions are kind of normal in stats.

  /* The only difference between this and an auto zero mode.
    is how we calculate the value. using two obs or four.
    OK. so perhaps do the calcuation at higher level
  */

  // printf("process \n");

#if 1
  // push onto buffer
  bool full = push_buffer1( app->buffer, &app->buffer_i, predict );

  if( full ) {
    // take the mean of the buffer.
    MAT *mean = m_mean( app->buffer, MNULL );
    assert(m_rows(mean) == 1 && m_cols(mean) == 1);
    double mean_ = m_get_val(mean, 0, 0);
    M_FREE(mean);

    double value = mean_;

    // push onto stats buffer
    push_buffer1( app->stats_buffer, &app->stats_buffer_i, value);


    MAT *stddev = m_stddev( app->stats_buffer, 0, MNULL );
    assert( m_cols(stddev) == 1 && m_rows(stddev) == 1);
    double stddev_ = m_get_val( stddev, 0, 0);
    M_FREE(stddev);

    // report
    char buf[100];
    printf("value %sV ", format_float_with_commas(buf, 100, 7, value));
    printf("stddev(%u) %.2fuV, ", m_rows(app->stats_buffer), stddev_  * 1000000 );   // multiply by 10^6. for uV
    printf("\n");

  }
#endif

}



static void simple_yield( app_t * app )
{
  update_console_cmd(app);
  app_led_update(app);
}



static void simple_wait( app_t * app, uint32_t period )
{
  update_console_cmd(app);
  app_led_update(app);


  // not static
  uint32_t soft_timer = system_millis;
  if( (system_millis - soft_timer ) > period ) {
    return;
  }
}





// #include <alloca.h>

static double calc_predicted_val(  MAT *b , Run *run, Param *param )
{
  // do xs.
  MAT *xs = run_to_matrix( param,  run, MNULL );
  assert(xs);
  assert( m_rows(xs) == 1 );

  // do aperture
  MAT *aperture = m_get(1,1);
  m_set_val( aperture, 0, 0, param->clk_count_aper_n);

  // predicted
  MAT *predicted = calc_predicted( b, xs, aperture);      // TODO - combine this function....
  assert(m_cols(predicted) == 1);
  // m_foutput(stdout, predicted );
  double value = m_get_val(predicted, 0, 0 );


  M_FREE(xs);
  M_FREE(aperture);
  M_FREE(predicted);

  return value;
}



void loop1 ( app_t *app )
{
  printf("=========\n");
  printf("loop1\n");

  ctrl_set_pattern( 0 ) ;     // no azero.

  printf("nplc   %.2lf\n", aper_n_to_nplc( ctrl_get_aperture()) );
  printf("buffer %u\n",    m_rows(app->buffer));

  Run   run;
  Param param;

  while(true) {

    // configure  integrator
    ctrl_reset_enable();
    ctrl_set_mux( HIMUX_SEL_REF_HI );
    app->data_ready = false;
    ctrl_reset_disable();

    // block/wait for data
    while(!app->data_ready ) {

      // printf("."); usart_flush();
      // we have a value.
      if(run.count_up ) {
        if(app ->b) {
          double predict = calc_predicted_val( app->b, &run, &param );
          process( app, predict );
        }
        // clear to reset
        memset(&run, 0, sizeof(Run));
      }

      simple_yield( app );   // change name simple update
      if(app->continuation_f) {
        return;
      }
    }

    // read the ready data
    run_read(&run);
    param_read_last( &param);

  }
}






void loop2 ( app_t *app /* void (*pyield)( appt_t * )*/  )
{
  // auto-zero

  printf("=========\n");
  printf("loop2\n");

  assert(app);

  ctrl_set_pattern( 0 ) ;     // no azero.

  /*
    autozero - should use two zero values, between read.
    have tmp.
    1) do ref-hi/sig .
    2) then lo. and convert using 3 values.
    3) then copy lo to temp.   and use for the next input.
  */

  Run   run_zero;
  Param param_zero;
  Run   run_sig;
  Param param_sig;

  memset(&run_zero, 0, sizeof(Run));
  memset(&run_sig, 0, sizeof(Run));


  while(true) {

    // configure ref_lo
    ctrl_reset_enable();
    ctrl_set_mux( HIMUX_SEL_REF_LO );
    app->data_ready = false;
    ctrl_reset_disable();


    // block/wait for data
    while(!app->data_ready ) {

      // we have both obs available...
      if(run_zero.count_up && run_sig.count_up ) {

        if(app ->b) {
          double predict_zero   = calc_predicted_val( app->b , &run_zero, &param_zero );
          double predict_sig    = calc_predicted_val( app->b , &run_sig,  &param_sig );
          double predict        = predict_sig - predict_zero;
          process( app, predict );
        }

        // clear to reset
        memset(&run_zero, 0, sizeof(Run));
        memset(&run_sig, 0, sizeof(Run));
      }

      simple_yield( app );   // change name simple update
      if(app->continuation_f) {
        return;
      }
    }

    // read data
    run_read(&run_zero);
    param_read_last( &param_zero);
    assert(param_zero.himux_sel ==  HIMUX_SEL_REF_LO );



    // configure ref_hi
    ctrl_reset_enable();
    ctrl_set_mux( HIMUX_SEL_REF_HI );
    app->data_ready = false;
    ctrl_reset_disable();

    // block/wait for data
    while(!app->data_ready ) {

      simple_yield( app );
      if(app->continuation_f) {
        return;
      }
    }

    // read data
    run_read(&run_sig);
    param_read_last( &param_sig);
    assert(param_sig.himux_sel == HIMUX_SEL_REF_HI );

    // printf("got value should be predict %sV\n", format_float_with_commas(buf, 100, 7, calc_predicted_val( app-> b , &run_sig , &param_sig )));

  }
}
















void loop3 ( app_t *app   )
{
  // could pass the continuatino to use.
  // auto-zero
  // iMPORTANT do three variable azero . by shuffling  values about.

  /*
    we don't need matrix

    sleep can be done with a loop.

  */

  printf("=========\n");
  printf("loop3\n");

  assert(app);

  ctrl_set_pattern( 0 ) ;


  Run   run_a;
  Param param_a;
  Run   run_b;
  Param param_b;

  memset(&run_a, 0, sizeof(Run));
  memset(&run_b, 0, sizeof(Run));

  // ref hi
  ctrl_reset_enable();
  ctrl_set_mux( HIMUX_SEL_REF_HI );
  ctrl_reset_disable();

  // OK. hang on. why use a matrix buffer. why not just spit values out to stdout????
  unsigned id = 0;
  // unsigned row = 0;
  // MAT *buffer = m_get( 100, 3);


  while(true) {

    // configure nplc
    ctrl_reset_enable();
    ctrl_set_aperture( nplc_to_aper_n(10));
    app->data_ready = false;
    ctrl_reset_disable();


    // block/wait for data
    while(!app->data_ready ) {

      // we have both obs available...
      if(run_a.count_up && run_b.count_up ) {

        if(app ->b) {
          double predict_a      = calc_predicted_val( app->b , &run_a, &param_a );
          double predict_b      = calc_predicted_val( app->b , &run_b,  &param_b );

          printf("%u   %.7lf,  %.7lf\n", id, predict_a, predict_b );

#if 0
          char buf[100], buf2[100];
          printf("%u   %sV\t  %sV\n",
            id,
            format_float_with_commas(buf, 100, 7, predict_a),
            format_float_with_commas(buf2, 100, 7, predict_b )
          );
#endif
        }

        // clear to reset
        memset(&run_a, 0, sizeof(Run));
        memset(&run_b, 0, sizeof(Run));
      }

      simple_yield( app );   // change name simple update
      if(app->continuation_f) {

        printf("whoot done \n");

        return;
      }
    }

    // read data
    run_read(&run_a);
    param_read_last( &param_a);
    assert( aper_n_to_nplc(param_a.clk_count_aper_n) == 10);



    // configure nplc
    ctrl_reset_enable();
    ctrl_set_aperture( nplc_to_aper_n(11));
    app->data_ready = false;
    ctrl_reset_disable();

    // block/wait for data
    while(!app->data_ready ) {

      simple_yield( app );
      if(app->continuation_f) {
        return;
      }
    }

    // read data
    run_read(&run_b);
    param_read_last( &param_b);
    assert(  aper_n_to_nplc(param_b.clk_count_aper_n) == 11);

    // printf("got value should be predict %sV\n", format_float_with_commas(buf, 100, 7, calc_predicted_val( app-> b , &run_b , &param_b )));

  }
}















/////////////////////////


/* Passing a continuation.

  - to allow calculating mean/std.
  - and to allow aggregating multiple entries. eg. nplc 50 == 5 lots of nplc 10.

  - the problem is that we cannot partially apply the continuation .  at the top level.
        - we could peel off the continuations off an array.
        - but that has type safety issues.

  - OR. pass a structure - with the named continuations.
  struct A
  {
     void (*continuation_for_yeild)( app_t *app, double val ) ;
     void (*continuation_for_stats)( app_t *app,  );
  }

  - or perhaps . it isn't really necessary and the signal processing chain . if just trunk to leaf

*/

