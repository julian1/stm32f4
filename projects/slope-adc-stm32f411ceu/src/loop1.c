


// #include <stdbool.h>

#include "assert.h"
#include "format.h" // format_bits
#include "usart.h"   // usart_flus()
#include "util.h"   // system_millis


#include "regression.h"
// #include <matrix.h>
#include "app.h"


#include "voltage-source.h"



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










// #include <alloca.h>

static double m_calc_predicted_val(  MAT *b , Run *run, Param *param )
{
  /*
    extr. we can use model = m_cols(b). for this.

  */

  unsigned model = m_cols(b);

  // do xs.
  MAT *xs = param_run_to_matrix( param,  run, model, MNULL );
  assert(xs);
  assert( m_rows(xs) == 1 );

  // do aperture
  MAT *aperture = m_get(1,1);
  m_set_val( aperture, 0, 0, param->clk_count_aper_n);

  // predicted
  MAT *predicted = m_calc_predicted( b, xs, aperture);      // TODO - remove/combine this function....

  assert(m_cols(predicted) == 1);
  assert(m_rows(predicted) == 1);

  // m_foutput(stdout, predicted );
  double value = m_get_val(predicted, 0, 0 );


  M_FREE(xs);
  M_FREE(aperture);
  M_FREE(predicted);

  return value;
}



void app_loop1 ( app_t *app )
{
  printf("=========\n");
  printf("app_loop1 - values\n");

  ctrl_set_pattern( app->spi, 0 ) ;     // no azero.

  printf("nplc   %.2lf\n", aper_n_to_nplc( ctrl_get_aperture(app->spi)) );
  printf("buffer %u\n",    m_rows(app->buffer));

  Run   run;
  Param param;

  while(true) {

    // configure  integrator
    // ctrl_reset_enable();
    // ctrl_set_mux( HIMUX_SEL_REF_HI );
    app->data_ready = false;
    // ctrl_reset_disable();

    // block/wait for data
    while(!app->data_ready ) {

      // printf("."); usart1_flush();
      // we have a value.
      if(run.count_up ) {
        if(app ->b) {
          double predict = m_calc_predicted_val( app->b, &run, &param );
          process( app, predict );
        }
        // clear to reset
        memset(&run, 0, sizeof(Run));
      }

      app_update( app );   // change name simple update
      if(app->continuation_f) {
        return;
      }
    }

    // read the ready data
    ctrl_run_read(app->spi, &run);
    ctrl_param_read_last( app->spi, &param);

  }
}









void app_loop2 ( app_t *app )
{
  printf("=========\n");
  printf("app_loop2 - cal loop using permutation of nplc/aperture\n");

  ctrl_set_pattern( app->spi, 0 ) ;     // no azero.

#define X_COLS 4

  // may want a row pointer as well.
  unsigned  max_rows =  10 * 9 * 2;
  MAT *xs       = m_get(max_rows, X_COLS); // TODO change MNULL
  MAT *y        = m_get(max_rows, 1);
  MAT *aperture = m_get(max_rows, 1); // required for predicted



  // unsigned nplc_[] = { 9, 10, 11, 12 };
  unsigned nplc[] = { 8, 9, 10, 11, 12, 13, 14, 15, 16 };
  // double y  = 0;

  unsigned row = 0;

  // loop nplc
  for(unsigned h = 0; h < ARRAY_SIZE(nplc); ++h)
  {
    int nplc_ = nplc[h];
    uint32_t aperture_ = nplc_to_aper_n( nplc_ );  // move this up a loop.
    printf("nplc   %u\n", nplc_    );


    // loop mux
    for(unsigned j = 0; j < 2; ++j)
    {
      uint32_t mux = j == 0 ? HIMUX_SEL_REF_LO : HIMUX_SEL_REF_HI;
      double   y_  = j == 0 ? 0                : 7.1;

      printf("mux %s\n", himux_sel_format( mux));


      ctrl_reset_enable(app->spi);
      ctrl_set_aperture( app->spi, aperture_);
      ctrl_set_mux( app->spi, mux );
      ctrl_reset_disable(app->spi);


      for(unsigned i = 0; i < 7; ++i) {


        ctrl_reset_enable(app->spi);
        app->data_ready = false;
        ctrl_reset_disable(app->spi);

        // block/wait for data
        while(!app->data_ready ) {
          app_update( app );
          if(app->continuation_f) {
            return;
          }
        }

        // read the data
        Run   run;
        Param param;

        ctrl_run_read(        app->spi, &run);
        ctrl_param_read_last( app->spi, &param);

        run_report(&run);
        /*
        // existing for calibration we won't be using b
        if(app ->b) {
          double predict = m_calc_predicted_val( app->b, &run, &param );
          printf("val(current cal) %lf", predict );
        } */

        if(i < 2) {
          printf("discard");
        }
        else {

          // record xs
          assert(row < m_rows(xs));
          MAT *whoot = param_run_to_matrix( &param, &run, X_COLS, MNULL );
          assert(whoot);
          assert( m_cols(whoot) == m_cols(xs) );
          assert( m_rows(whoot) == 1  );

          // printf("\n");
          // m_foutput(stdout, whoot );
          m_row_set( xs, row, whoot );
          M_FREE(whoot);

          // record aperture
          assert(row < m_rows(aperture));
          assert( param.clk_count_aper_n  == aperture_ );
          m_set_val( aperture, row, 0, param.clk_count_aper_n );


          // record y, as target * aperture
          assert(row < m_rows(y));
          m_set_val( y       , row , 0, y_  *  param.clk_count_aper_n );

          // increment row.
          ++row;
        }

        printf("\n");



      } // i
    } // j
  } // h



  // restore default
  ctrl_reset_enable(app->spi);
  ctrl_set_aperture( app->spi, nplc_to_aper_n( 10 ));
  ctrl_reset_disable(app->spi);



  printf("row %u\n", row);
  usart1_flush();

  // shrink matrixes to size collected data
  m_resize( xs, row, m_cols( xs) );
  m_resize( y,  row, m_cols( y) );
  m_resize( aperture, row, m_cols( aperture) ); // we don't use aperture


  #if 0
  // need to multiply by the aperture?
  m_foutput(stdout, xs );
  usart1_flush();

  m_foutput(stdout, y );
  usart1_flush();
  #endif


  MAT *b = m_regression( xs, y, MNULL );
  assert( m_rows(b) == m_cols( xs) ); // calibration coeff is horizontal matrix.
  printf("b\n");
  m_foutput(stdout, b);
  usart1_flush();


  // no we need the aperture. to calc predicted

  MAT *predicted = m_calc_predicted( b, xs, aperture);
  printf("predicted\n");
  m_foutput(stdout, predicted);


  app->b = b;

}











void app_loop3 ( app_t *app /* void (*pyield)( appt_t * )*/  )
{
  // auto-zero

  printf("=========\n");
  printf("app_loop3\n");

  assert(app);

  ctrl_set_pattern( app->spi, 0 ) ;     // no azero.

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
    ctrl_reset_enable(app->spi);
    ctrl_set_mux( app->spi, HIMUX_SEL_REF_LO );
    app->data_ready = false;
    ctrl_reset_disable(app->spi);


    // block/wait for data
    while(!app->data_ready ) {

      // we have both obs available...
      if(run_zero.count_up && run_sig.count_up ) {

        if(app ->b) {
          double predict_zero   = m_calc_predicted_val( app->b , &run_zero, &param_zero );
          double predict_sig    = m_calc_predicted_val( app->b , &run_sig,  &param_sig );
          double predict        = predict_sig - predict_zero;
          process( app, predict );
        }

        // clear to reset
        memset(&run_zero, 0, sizeof(Run));
        memset(&run_sig, 0, sizeof(Run));
      }

      app_update( app );   // change name simple update
      if(app->continuation_f) {
        return;
      }
    }

    // read data
    ctrl_run_read(app->spi, &run_zero);
    ctrl_param_read_last( app->spi, &param_zero);
    assert(param_zero.himux_sel ==  HIMUX_SEL_REF_LO );



    // configure ref_hi
    ctrl_reset_enable(app->spi);
    ctrl_set_mux( app->spi, HIMUX_SEL_REF_HI );
    app->data_ready = false;
    ctrl_reset_disable(app->spi);

    // block/wait for data
    while(!app->data_ready ) {

      app_update( app );
      if(app->continuation_f) {
        return;
      }
    }

    // read data
    ctrl_run_read(app->spi, &run_sig);
    ctrl_param_read_last( app->spi, &param_sig);
    assert(param_sig.himux_sel == HIMUX_SEL_REF_HI );

    // printf("got value should be predict %sV\n", format_float_with_commas(buf, 100, 7, m_calc_predicted_val( app-> b , &run_sig , &param_sig )));

  }
}







void app_voltage_source_set( app_t *app, double value )
{
  // this has to read the adc - which makes it a lot more specific
  // than general app_( app_t *app)  code.
  // so put here, instead of app.c

  double current = app_simple_read( app);

  if( value > current ) {

    voltage_source_set_dir(1);
    while(1) {
      current = app_simple_read( app);
      // printf("val %lf\n", current);
      printf(".");
      if(current > value)
        break;

      if(app->continuation_f)
        break;
    }

    // should be renamed set_dir
    voltage_source_set_dir(0);


  } else {

    voltage_source_set_dir(-1);
    while(1) {
      current = app_simple_read( app);
      printf(".");
      // printf("val %lf\n", current);
      if(current < value)
        break;
      if(app->continuation_f)
        break;

    }

    voltage_source_set_dir(0);
  }

  printf("\n");
}



double app_simple_read( app_t *app)
{
  // not sure if this should be here.
  //

  // minimum needed to read a value
  // used to steer the current before we do anything.
  Run   run;
  Param param;

  // clear to reset
  memset(&run, 0, sizeof(Run));

  /* setting the aperture here, will confuse, if call from another loop.
    could record the aperture and then return...
    but this case, should probably be handled elsewhere...
  */

  ctrl_reset_enable(app->spi);
  ctrl_set_aperture( app->spi, nplc_to_aper_n(10));
  app->data_ready = false;
  ctrl_reset_disable(app->spi);

  // block/wait for data
  while(!app->data_ready ) {

    app_update( app );
    /*if(app->continuation_f) {
      printf("whoot done \n");
      return;
    }
    */
  }

  ctrl_run_read(app->spi, &run);
  ctrl_param_read_last(app->spi, &param);

  // we have both obs available...
  assert(run.count_up);
  assert(app ->b);

  double predict = m_calc_predicted_val( app->b , &run, &param );
  return predict;
}





/*

*/



void app_loop4 ( app_t *app   )
{
  // loop44 INL.

  /*
    can permute.
    (1) nplc
    (2) var pos / var neg
    (3) var in relation to fix
    (4) all of the above

    probably better to keep nplc constant.

  */

  printf("=========\n");
  printf("app_loop4\n");

  assert(app);

  ctrl_set_pattern( app->spi, 0 ) ;


  // mux signal input
  ctrl_reset_enable(app->spi);
  ctrl_set_mux( app->spi, HIMUX_SEL_SIG_HI );
  ctrl_reset_disable(app->spi);


  // app_voltage_source_set( app, 5.0 );



  float target_[] = { 5, 4.5, 4, 3.5, 3, 2.5, 2, 1.5, 1, 0.5, 0 } ;


  for(unsigned i = 0; i < ARRAY_SIZE(target_); ++i)
  {

    double target = target_[i];

    // change to voltage
    printf("voltage set %lf\n", target );
    app_voltage_source_set( app, target );

    // sleep to let DA settle.
    unsigned sleep = i == 0 ? 60 : 30;
    printf("sleep %us\n", sleep );
    app_simple_sleep( app, sleep * 1000 );


    // 10 obs
    for(unsigned obs = 0; obs < 10; ++obs)
    {


      // do A
      // configure nplc
      ctrl_reset_enable(app->spi);
      ctrl_set_aperture( app->spi, nplc_to_aper_n( 10  ));
      app->data_ready = false;
      ctrl_reset_disable(app->spi);

      // block/wait for data
      while(!app->data_ready ) {
        app_update( app );   // change name simple update
        if(app->continuation_f) {
          printf("whoot done \n");
          return;
        }
      }

      // read A.
      Run   run_a;
      Param param_a;
      // memset(&run_a, 0, sizeof(Run));

      // read data
      ctrl_run_read(app->spi, &run_a);
      ctrl_param_read_last( app->spi, &param_a);
      assert( aper_n_to_nplc(param_a.clk_count_aper_n) == 10);



      ///////////////////////////////

      // do B
      // configure nplc
      ctrl_reset_enable(app->spi);
      ctrl_set_aperture( app->spi, nplc_to_aper_n(11));
      app->data_ready = false;
      ctrl_reset_disable(app->spi);

      // block/wait for data
      while(!app->data_ready ) {
        app_update( app );
        if(app->continuation_f) {
          printf("whoot done \n");
          return;
        }
      }

      // read B
      Run   run_b;
      Param param_b;
      // memset(&run_b, 0, sizeof(Run));


      // read data
      ctrl_run_read(app->spi, &run_b);
      ctrl_param_read_last( app->spi, &param_b);
      assert(  aper_n_to_nplc(param_b.clk_count_aper_n) == 11);

      // printf("got value should be predict %sV\n", format_float_with_commas(buf, 100, 7, m_calc_predicted_val( app-> b , &run_b , &param_b )));

        // this is the process.
        // we have both obs available...

      if(app ->b) {
        double predict_a      = m_calc_predicted_val( app->b , &run_a, &param_a );
        double predict_b      = m_calc_predicted_val( app->b , &run_b,  &param_b );


        /*
        if(mode == starting && predict_a > 10)  {
          mode = running;
        }
        */
        #if 0
        printf("%u   %.7lf,  %.7lf  %.2fuV\n", id, predict_a, predict_b, (predict_a - predict_b) * 1000000 );
        #endif

        #if 1
        char buf[100], buf2[100];
        printf("%u   %sV\t  %sV  %.2fuV\n",
          i,
          format_float_with_commas(buf, 100, 7, predict_a),
          format_float_with_commas(buf2, 100, 7, predict_b ),
          (predict_a - predict_b) * 1000000
        );
        #endif
      }






    } // obs loop.
  } // target loop
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

