


// #include <stdbool.h>

#include "assert.h"
#include "format.h" // format_bits
#include "usart.h"   // usart_flus()
#include "util.h"   // system_millis
#include "ice40.h"   // spi_ice40_reg_read()
#include "streams.h"   // fflush_on_newline()


#include "regression.h"
// #include <matrix.h>
#include "app.h"
#include "cal.h"
#include "temp.h"

#include "temp.h"


// #include "voltage-source-1/voltage-source.h"
#include "voltage-source-2/voltage-source.h"


#include "voltage-source-2/4094.h"      // REG_RAILS_ON



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
  assert( m_is_scalar( stddev ));
  double stddev_ = m_to_scalar( stddev);
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
    assert( m_is_scalar( mean ));
    double mean_ = m_to_scalar( mean);
    M_FREE(mean);

    double value = mean_;

    // push onto stats buffer
    push_buffer1( app->stats_buffer, &app->stats_buffer_i, value);


    MAT *stddev = m_stddev( app->stats_buffer, 0, MNULL );
    assert( m_is_scalar( stddev ));
    double stddev_ = m_to_scalar( stddev);
    M_FREE(stddev);

    // report
    char buf[100];
    printf("value %sV ",          format_float_with_commas(buf, 100, 7, value));
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

  unsigned model = m_rows(b);
  // printf("model %u\n", model );

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
  printf("cal model %u\n", app->cal_model);


  // ctrl_set_pattern( app->spi, 0 ) ;     // no azero.

  printf("cal slot %u\n", app->cal_idx );

  int aperture = ctrl_get_aperture(app->spi); // in clk counts
  printf("nplc   %.2lf\n",  aper_n_to_nplc( aperture ));
  printf("period %.2lfs\n", aper_n_to_period( aperture ));
  printf("buffer %u\n",    m_rows(app->buffer));

  while(true) {

    Run   run;
    Param param;

    // if first()
    // ctrl_reset_enable(app->spi);
    app->data_ready = false;
    // ctrl_reset_disable(app->spi);

    // block/wait for data
    while(!app->data_ready ) {
      app_update( app );   // change name simple update
      if(app->continuation_f) {
        return;
      }
    }

    // this triggers if not aligned.
    // assert(run.count_up);

    // read the ready data
    ctrl_run_read(app->spi, &run);
    ctrl_param_read( app->spi, &param);

    assert( ctrl_get_state( app->spi ) == STATE_RESET);

    // only report if using buffer, to reduce clutter
    if(m_rows(app->buffer) == 1)
      run_report_brief( &run);

    assert( app->cal_idx < ARRAY_SIZE(app->cal));
    Cal *cal = app->cal[ app->cal_idx ];
    if(cal) {
      assert(cal->b);
      double predict = m_calc_predicted_val( cal->b, &run, &param );
      process( app, predict );
    }


  }
}

// void run_report_brief( const Run *run );








void app_loop2 ( app_t *app )
{
  printf("=========\n");
  printf("app_loop2 - cal loop using permutation of nplc/aperture\n");

  printf("cal model %u\n", app->cal_model);


  // clear last for mem
  if(app->last) {
    M_FREE(app->last);
  }



  // unsigned nplc_[] = { 9, 10, 11, 12 };
  // unsigned nplc[] = { 8, 9, 10, 11, 12, 13  };
  unsigned nplc[] = { 8, 9, 10, 11, 12, 13, 14, 15, 16  };
  // double y  = 0;

  unsigned obs_n = 7; // 7

  // may want a row pointer as well.
  unsigned  max_rows =  obs_n * ARRAY_SIZE(nplc) * 2;

  unsigned cols = 0;
  switch ( app->cal_model) {
    case 3: cols = 3; break;
    case 4: cols = 4; break;  // + intercept
    case 5: cols = 4; break;  // + flip_count
    default: assert(0);
  };

  MAT *xs       = m_get(max_rows, cols );


  MAT *y        = m_get(max_rows, 1);
  MAT *aperture = m_get(max_rows, 1); // required for predicted



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
      assert( ctrl_get_state( app->spi ) == STATE_RESET_START);
      ctrl_reset_disable(app->spi);
      assert( ctrl_get_state( app->spi ) == STATE_RESET);

      for(unsigned i = 0; i < obs_n; ++i) {

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

        ctrl_run_read(   app->spi, &run);
        ctrl_param_read( app->spi, &param);

        run_report(&run);
        /*
        // existing for calibration we won't be using b
        if(app ->b) {
          double predict = m_calc_predicted_val( app->cal, &run, &param );
          printf("val(current cal) %lf", predict );
        } */

        if(i < 2) {
          printf("discard");
        }
        else {

          // record xs
          assert(row < m_rows(xs));
          MAT *whoot = param_run_to_matrix( &param, &run, cols , MNULL );
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


  R regression;

  m_regression( xs, y, &regression );


  // predicted must be adjusted by aperture
  MAT *predicted =  m_calc_predicted( regression.b, xs, aperture );
  printf("\npredicted\n");
  m_foutput(stdout, predicted);
  usart1_flush();

  // TODO change name regression_report.
  r_report( &regression, stdout);


/*
  // we can calculate this at any anytime.
  // albeit dependency on PL(freq).
  double sigma_div_aperture = regression.sigma / nplc_to_aper_n( 10 ) * 1000000;  // in uV.

  printf("stderr(V) %.2fuV  (nplc10)\n", sigma_div_aperture);

  report this in cal_report( cal )
*/


  ///////////////////////
  // create the cal structure

  Cal *cal = cal_create();
  cal->slot   = 0;
  cal->b      = m_copy( regression.b, MNULL );    // reallocate matrix.
  ctrl_param_read( app->spi, &cal->param);
  cal->sigma2 = regression.sigma2;
  cal->temp   = adc_temp_read10();

  // alloc - so we can free() predictably
  cal->comment = strdup( app->cal_comment );
  cal->id     = app->cal_id_count ;

  cal_report( cal );


  ///////////////////////
  // set it. for app slot
  // we store in slot 0;
  // use a function. cal_set???? no because ownership not clear.
  // should switch and save new cal in slot 0. by default?
  printf("\nswitching to and storing in cal slot 0\n");

  app->cal_idx = 0;

  if( app->cal[ app->cal_idx]) {
    cal_free( app->cal[ app->cal_idx ]);
  }

  app->cal[ app->cal_idx ] = cal;

  r_free( &regression );


  // combine xs, y and store in last.
  app->last = m_hconcat(xs, y, MNULL);

  M_FREE(xs);
  M_FREE(y);


}











void app_loop3 ( app_t *app /* void (*pyield)( appt_t * )*/  )
{
  /*
    EXTR. I think auto-zero is worse. only because of quantitization.  eg. 0 - 0.6uV  difference when might only be 0.3 - 0.2uV.

  value -0.000,000,6V stddev(10) 0.40uV,
  value -0.000,000,6V stddev(10) 0.30uV,
  value 0.000,000,0V stddev(10) 0.31uV,
  value -0.000,000,6V stddev(10) 0.31uV,
  value 0.000,000,0V stddev(10) 0.31uV,
  value -0.000,000,6V stddev(10) 0.30uV,
  value 0.000,000,6V stddev(10) 0.40uV,
  value 0.000,000,0V stddev(10) 0.39uV,
  value 0.000,000,0V stddev(10) 0.39uV,
  value -0.000,000,6V stddev(10) 0.40uV,
  value 0.000,000,6V stddev(10) 0.46uV,
  value 0.000,000,6V stddev(10) 0.47uV,
  value -0.000,000,6V stddev(10) 0.51uV,
  value -0.000,000,6V stddev(10) 0.51uV,
  value 0.000,000,6V stddev(10) 0.55uV,
  value -0.000,000,6V stddev(10) 0.55uV,
  value 0.000,000,0V stddev(10) 0.51uV,

  */

  // auto-zero

  printf("=========\n");
  printf("app_loop3 autozero\n");

  assert(app);

  // ctrl_set_pattern( app->spi, 0 ) ;     // no azero on device.

  /*
    autozero - should use two zero values, between read.
    have tmp.
    1) do ref-hi/sig .
    2) then lo. and convert using 3 values.
    3) then copy lo to temp.   and use for the next input.
  */

  // memset(&run_zero, 0, sizeof(Run));
  // memset(&run_sig, 0, sizeof(Run));

  // record the mux input to use
  unsigned mux_sel = spi_ice40_reg_read(app->spi, REG_HIMUX_SEL );


  while(true) {

      // configure ref_lo
    ctrl_reset_enable(app->spi);
    ctrl_set_mux( app->spi, HIMUX_SEL_REF_LO );
    app->data_ready = false;
    assert( ctrl_get_state( app->spi ) == STATE_RESET_START);
    ctrl_reset_disable(app->spi);
    assert( ctrl_get_state( app->spi ) == STATE_RESET);

    // block/wait for data
    while(!app->data_ready ) {
      app_update( app );   // change name simple update
      if(app->continuation_f) {
        return;
      }
    }

    Run   run_zero;
    Param param_zero;

    // read data
    ctrl_run_read(app->spi, &run_zero);
    ctrl_param_read( app->spi, &param_zero);
    assert(param_zero.himux_sel ==  HIMUX_SEL_REF_LO );


    // configure mux_sel
    ctrl_reset_enable(app->spi);
    ctrl_set_mux( app->spi,   mux_sel );
    app->data_ready = false;
    assert( ctrl_get_state( app->spi ) == STATE_RESET_START);
    ctrl_reset_disable(app->spi);
    assert( ctrl_get_state( app->spi ) == STATE_RESET);

    // block/wait for data
    while(!app->data_ready ) {
      app_update( app );
      if(app->continuation_f) {
        return;
      }
    }

    Run   run_sig;
    Param param_sig;

    // read data
    ctrl_run_read(app->spi, &run_sig);
    ctrl_param_read( app->spi, &param_sig);
    assert(param_sig.himux_sel == mux_sel);

    // printf("got value should be predict %sV\n", format_float_with_commas(buf, 100, 7, m_calc_predicted_val( app-> b , &run_sig , &param_sig )));
    assert(run_zero.count_up && run_sig.count_up ) ;

      // we have both obs available...


    assert( app->cal_idx < ARRAY_SIZE(app->cal));
    Cal *cal = app->cal[ app->cal_idx ];
    if(cal) {
      assert(cal->b);
      double predict_zero   = m_calc_predicted_val( cal->b , &run_zero, &param_zero );
      double predict_sig    = m_calc_predicted_val( cal->b , &run_sig,  &param_sig );
      double predict        = predict_sig - predict_zero;
      process( app, predict );
    }
  }
}







#if 0
void app_voltage_source_set( app_t *app, double value )
{
  // using cap.
  // this has to read the adc - which makes it a lot more specific
  // than general app_( app_t *app)  code.
  // so put here, instead of app.c
  // change name current voltage
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
#endif



void app_voltage_source_set( app_t *app, double value )
{
  // using voltage_source_2
  spi_voltage_source_2_set_val(app->spi_voltage_source, 0 , value );

}



double app_simple_read( app_t *app)
{
  /*
    what cal to use for this?
  */
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
  // ctrl_set_aperture( app->spi, nplc_to_aper_n(10));
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
  ctrl_param_read(app->spi, &param);

  // we have both obs available...
  assert(run.count_up);

  assert( app->cal_idx < ARRAY_SIZE(app->cal));
  Cal *cal = app->cal[ app->cal_idx ];
  assert(cal);
  assert(cal->b);


  double predict = m_calc_predicted_val( cal->b, &run, &param );
  return predict;
}





/*

*/



void app_loop4 ( app_t *app   )
{
  // loop44 INL.
  /*
    we could save the generated matrix against app. and then print out the last run at any time. if wanted.
  */
  /*
    can permute.
    (1) nplc
    (2) var pos / var neg
    (3) var in relation to fix
    (4) all of the above

    probably better to keep nplc constant.

    EXTR.
    we need to try a single discard . for each A,B.
    see if rundown slope DA is contributing to the final result.
  */

  printf("=========\n");
  printf("app_loop4\n");

  assert(app);


  if( !spi_voltage_source_2_in_on(&app->spi_4094_reg)) {
    usart1_printf("spi_voltage_source_2 not on\n");
    return;
  }



  // clear last for mem
  if(app->last) {
    M_FREE(app->last);
  }



  // mux signal input
  ctrl_reset_enable(app->spi);
  // ctrl_set_pattern( app->spi, 0 ) ;
  ctrl_set_mux( app->spi, HIMUX_SEL_SIG_HI );
  ctrl_set_aperture( app->spi, nplc_to_aper_n( 10  ));
  ctrl_reset_disable(app->spi);


  unsigned row = 0;

  // 22V range.
  double target_[] = {
    11, 10.5, 10, 9.5, 9, 8.5, 8, 7.5, 7, 6.5, 6, 5.5, 5, 4.5, 4, 3.5, 3, 2.5, 2, 1.5, 1, 0.5, 0,
    -0.5, -1, -1.5, -2, -2.5, -3, -3.5, -4, -4.5, -5, -5.5, -6, -6.5, -7, -7.5, -8, -8.5, -9, -9.5, -10, -10.5, -11 } ;

  // more obs == more spread due to DA.
  unsigned obs_n = 50;

  // size array
  MAT *m = m_get( ARRAY_SIZE(target_) * obs_n , 5 );


  for(unsigned i = 0; i < ARRAY_SIZE(target_); ++i)
  {

    double target = target_[i];

    // change to voltage
    printf("voltage set %.1f\n", target );
    app_voltage_source_set( app, target );

#if 1
    // sleep to let DA settle.
    unsigned sleep = 3;  // for dac
    // unsigned sleep = i == 0 ? 60 : 30;
    // unsigned sleep = i == 0 ? 120 : 60;
    // unsigned sleep = i == 0 ? (180 * 2) : 180;
    printf("sleep %us\n", sleep );
    app_simple_sleep( app, sleep * 1000 );
#endif

    /* - I think we probably want to be able to do a loop of 2. to not take the first value.
    */

    // 10 obs
    for(unsigned obs = 0; obs < obs_n; ++obs)
    {

      /*
        we can switch A versus B using ? : expression here
        No. it's easier to gather data separately.
      */

      // do A
      Cal *cal_a = app->cal[4] ;
      assert( cal_a );
      param_report( &cal_a->param );
      printf("\n");

      ctrl_reset_enable(app->spi);
      ctrl_set_aperture( app->spi,  cal_a->param.clk_count_aper_n);
      ctrl_set_var_n( app->spi,     cal_a->param.clk_count_var_n);
      ctrl_set_fix_n( app->spi,     cal_a->param.clk_count_fix_n);
      app->data_ready = false;
      ctrl_reset_disable(app->spi);

      // block/wait for data
      while(!app->data_ready ) {
        app_update( app );   // change name simple update
        if(app->continuation_f) {
          printf("bail done \n");
          return;
        }
      }

      // read A.
      Run   run_a;
      Param param_a;
      ctrl_run_read(   app->spi, &run_a);
      ctrl_param_read( app->spi, &param_a);

      // shouldn't need this
      assert( param_a.clk_count_aper_n  == cal_a->param.clk_count_aper_n);
      assert( param_a.clk_count_var_n   == cal_a->param.clk_count_var_n );
      assert( param_a.clk_count_fix_n   == cal_a->param.clk_count_fix_n);


      ///////////////////////////////

      // do B
      Cal *cal_b = app->cal[5] ;
      assert( cal_b );
      param_report( &cal_b->param );
      printf("\n");


      ctrl_reset_enable(app->spi);
      ctrl_set_aperture( app->spi,  cal_b->param.clk_count_aper_n);
      ctrl_set_var_n( app->spi,     cal_b->param.clk_count_var_n);
      ctrl_set_fix_n( app->spi,     cal_b->param.clk_count_fix_n);
      app->data_ready = false;
      ctrl_reset_disable(app->spi);

      // block/wait for data
      while(!app->data_ready ) {
        app_update( app );
        if(app->continuation_f) {
          printf("bail done \n");
          return;
        }
      }

      // read B
      Run   run_b;
      Param param_b;
      ctrl_run_read(   app->spi, &run_b);
      ctrl_param_read( app->spi, &param_b);

      // shouldn't need this
      assert( param_b.clk_count_aper_n  == cal_b->param.clk_count_aper_n);
      assert( param_b.clk_count_var_n   == cal_b->param.clk_count_var_n );
      assert( param_b.clk_count_fix_n   == cal_b->param.clk_count_fix_n);

      ///////////////////////
      // work out A,B difference

      assert(cal_a->b && cal_b->b );

      double predict_a  = m_calc_predicted_val( cal_a->b, &run_a, &param_a );
      double predict_b  = m_calc_predicted_val( cal_b->b, &run_b, &param_b );
      double delta      = (predict_a - predict_b) * 1000000; // in uV.

      char buf[100], buf2[100];
      printf("%u   %sV\t  %sV  %.2fuV\n",
        i,
        format_float_with_commas(buf, 100, 7, predict_a),
        format_float_with_commas(buf2, 100, 7, predict_b ),
        delta
      );

      // push to matrix
      assert(row < m_rows(m));
      assert(m_cols(m) == 5);
      m_set_val( m, row, 0, i + 1 );        // id. start at 1 for matlab/octave index function
      m_set_val( m, row, 1, target );   // don't really need.
      m_set_val( m, row, 2, predict_a );
      m_set_val( m, row, 3, predict_b );
      m_set_val( m, row, 4, delta );

      ++row;

    } // obs loop.

  } // target loop



  // shrink matrix
  m_resize( m, row, m_cols(m));


  // set to flush
  ffnctl( stdout, ffnctl( stdout, 0) | FILE_SYNC_ON_NEWLINE );
  m_octave_foutput( stdout, NULL, m);

  ffnctl( stdout, ffnctl( stdout, 0) & ~FILE_SYNC_ON_NEWLINE );

  app->last = m;

  // switch the blink interval to fast. to indicate done.


  printf("resetting voltage");
  app_voltage_source_set( app, 11 );

  app->led_blink_interval = 250;

}


static void app_loop5 ( app_t *app   )
{
  /*
    no. this isn't needed. just save slot4. then modify var_n,fix_n  and save slot 5.
    that way both slots have same b.

    ------
    use the same calibration coefficients between A, B. eg. from cal slot 4. 
    but vary var_n fix_n  with an offset. 

  */
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

