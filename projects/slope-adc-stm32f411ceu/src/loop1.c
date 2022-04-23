


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
#include "spi1.h" // to set handler


#include "voltage-source-1/voltage-source.h"
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

static double m_calc_predicted_val(  MAT *b , Run *run, unsigned aper_n )
{
  /*
    extr. we can use model = m_cols(b). for this.

    might be easier to pass aperture as scalar... no . because

  */

  unsigned model = m_rows(b);
  // printf("model %u\n", model );

  // do xs.
  MAT *xs = run_to_matrix( run, model, MNULL );
  assert(xs);
  assert( m_rows(xs) == 1 );

  // create aperture structure
  MAT *aperture = m_get(1,1);
  m_set_val( aperture, 0, 0, /*param->clk_count_aper_n */ aper_n );

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



///////////////////////////

struct X1
{
  Run     run;
  bool    data_ready;
  app_t   *app;   // for spi/ verbose... etc
} ;
typedef struct X1 X1;



static void app_loop1_spi1_interupt( X1 *x)
{
  /*
    interupt handler context.
    read data without pause
  */
  app_t *app = x->app;
  ctrl_run_read(app->spi, &x->run, app->verbose);
  x->data_ready = true;
}





void app_loop1 ( app_t *app )
{
  printf("=========\n");
  printf("app_loop1 - values\n");

  printf("cal slot %u\n", app->cal_slot_idx);




  assert( app->cal_slot_idx < ARRAY_SIZE(app->cal));
  Cal *cal = app->cal[ app->cal_slot_idx ];

  if(!cal) {
    printf("no cal for slot\n");
    return;
  }

  /* IMPRTANT.

      - for loop1 / loop3 - we want to be able to vary the aperture freely. so just use current state of device.
      - BUT for loop2/ loop4 etc. we want to use the aperture associated with the cal.

      - note that aperture can be changed from cmd_buffer this is a bug.
          we should check/read aperture in the actual measurement loop.
  */

  // if want to support dynamiclly changing aperture - then needs to read every run
  unsigned aperture = ctrl_get_aperture(app->spi);

  // write current cal modulation parameters (var_n,fix_n), but not aperture
  ctrl_reset_enable(app->spi);
  // ctrl_param_write( app->spi, &cal->param);
  ctrl_set_var_n( app->spi,  cal->param.clk_count_var_n);
  ctrl_set_fix_n( app->spi,  cal->param.clk_count_fix_n);
  ctrl_reset_disable(app->spi);


  printf("model  %u\n",     cal->model);
  printf("nplc   %.2lf\n",  aper_n_to_nplc( aperture ));
  printf("period %.2lfs\n", aper_n_to_period( aperture ));
  printf("buffer %u\n",     m_rows(app->buffer));
  printf("stats  %u\n",     m_rows(app->stats_buffer));
  param_show( & cal->param );
  printf("\n");


  X1   x;
  memset(&x, 0, sizeof(x));
  x.app = app;
  // x.data_ready = false;

  // set handler
	spi1_interupt_handler_set(  (void (*)(void *))  app_loop1_spi1_interupt, &x );

  while(!app->halt_func) {

    // block/wait for data
    while(!x.data_ready ) {
      app_update( app );   // change name simple update
      // mem leak?
      if(app->halt_func) {
        break;
      }
    }

    // we got and read the data, so clear the flag to be ready
    x.data_ready = false;


    // only report if using buffer, to reduce clutter
    if(m_rows(app->buffer) == 1)
      run_show( &x.run, app->verbose );

    if(cal) {

      // printf("cal slot %u", app->cal_slot_idx );

      assert(cal->b);
      double predict = m_calc_predicted_val( cal->b, &x.run, aperture ); // wrong if aperture changes.
      process( app, predict );
    }
  }

  // restore handler
	spi1_interupt_handler_set(  (void (*)(void *))  app_spi1_interupt, app );
}




///////////////////////////////////////


struct X3
{
  bool  do_value;

  Run     zero;
  Run     run;    // change name meas
  Run     zero_last;

  unsigned himux_sel;

  bool    data_ready;
  app_t   *app;   // for spi/ verbose... etc
} ;
typedef struct X3 X3;



static void app_loop3_spi1_interupt( X3 *x)
{
  /*
    interupt handler context.
    read data without pause
  */
  app_t *app = x->app;

  if(x->do_value) {
    // read value
    ctrl_run_read(app->spi, &x->run, app->verbose);
    ctrl_set_mux( app->spi, HIMUX_SEL_REF_LO );
    x->do_value = false;


  } else {
    // move zero, to last
    // read zero
    x->zero_last = x->zero;   // deep copy
    ctrl_run_read(app->spi, &x->zero, app->verbose);
    ctrl_set_mux( app->spi, x->himux_sel );
    x->do_value = true;

    x->data_ready = true;
  }
}






void app_loop3 ( app_t *app   )
{

  // auto-zero

  printf("=========\n");
  printf("app_loop3 autozero\n");

  assert(app);


  assert( app->cal_slot_idx < ARRAY_SIZE(app->cal));
  Cal *cal = app->cal[ app->cal_slot_idx ];
  if(!cal) {
    printf("no cal\n");
    return;
  }

  // ctrl_set_pattern( app->spi, 0 ) ;     // no azero on device.

  /*
    autozero - should use two zero values, between read.
    have tmp.
    1) do ref-hi/sig .
    2) then lo. and convert using 3 values.
    3) then copy lo to temp.   and use for the next input.
  */


  // write current cal modulation parameters, but not aperture
  ctrl_reset_enable(app->spi);
  // ctrl_param_write( app->spi, &cal->param);
  ctrl_set_var_n( app->spi,  cal->param.clk_count_var_n);
  ctrl_set_fix_n( app->spi,  cal->param.clk_count_fix_n);
  ctrl_reset_disable(app->spi);


  // record the mux input to use,
  unsigned mux_sel = spi_ice40_reg_read(app->spi, REG_HIMUX_SEL );

  // if want to support dynamiclly changing aperture - then needs to read every run
  unsigned aperture = ctrl_get_aperture(app->spi);

  X3   x;
  memset(&x, 0, sizeof(x));
  x.app = app;
  x.himux_sel = mux_sel;

  // set handler
	spi1_interupt_handler_set(  (void (*)(void *))  app_loop3_spi1_interupt, &x );

  while(!app->halt_func) {

    // block/wait for data
    while(!x.data_ready ) {
      app_update( app );   // change name simple update
      // mem leak?
      if(app->halt_func) {
        break;
      }
    }

    // we got and read the data, so clear the flag to be ready
    x.data_ready = false;

    assert(cal->b);
    double predict_zero_last   = m_calc_predicted_val( cal->b , &x.zero_last, aperture );  // param only needed for aperture.
    double predict_zero   = m_calc_predicted_val( cal->b , &x.zero, aperture );  // param only needed for aperture.
    double predict_sig    = m_calc_predicted_val( cal->b , &x.run,  aperture );

    double predict        = predict_sig - ((predict_zero + predict_zero_last)  / 2.f);

    if(app->verbose) {

      run_show(& x.zero_last, app->verbose);
      printf("%f\n", predict_zero_last );

      run_show(& x.run, app->verbose );
      printf("%f\n", predict_sig );

      run_show(& x.zero, app->verbose);
      printf("%f\n", predict_zero );
    }

    process( app, predict );

  }

  // restore handler
	spi1_interupt_handler_set(  (void (*)(void *))  app_spi1_interupt, app );

}


















/*
  ok. we need to prompt for a value....

*/

void calc_cal( app_t *app,  MAT *y, MAT *xs, MAT *aperture  )
{
  /*
      calc_calibration_from_data
    better name. do_calibration.
    note uses replaces existing calibration from slot.
  */


  R regression;

  m_regression( xs, y, &regression );   // rename reg_regression()...


  // predicted must be adjusted by aperture
  MAT *predicted =  m_calc_predicted( regression.b, xs, aperture );
  printf("\npredicted\n");
  m_foutput(stdout, predicted);
  usart1_flush();

  // TODO change name regression_report.
  regression_show( &regression, stdout);


  ///////////////////////
  // set it. for app slot
  printf("\nstoring (but no save) to cal slot %u\n", app->cal_slot_idx);

  // should be passed explicitly...
  // old cal.
  Cal *cal = app->cal[ app->cal_slot_idx ];
  assert(cal);

  ///////////////////////
  // create the cal2 structure
  Cal *cal2     = cal_create();
  cal2->slot    = app->cal_slot_idx;
  cal2->b       = m_copy( regression.b, MNULL );    // reallocate matrix.
  ctrl_param_read( app->spi, &cal2->param);
  cal2->sigma2  = regression.sigma2;
  cal2->temp    = adc_temp_read10();

  cal2->id      = ++(app->cal_id_max);

  // appropriate fields from old cal
  cal2->comment = strdup( cal->comment );
  cal2->model   = cal->model;

  cal_show( cal2 );


  ///////////////////////
  // free old cal
  assert( cal == app->cal[ app->cal_slot_idx ] );
  cal_free( app->cal[ app->cal_slot_idx  ]);
  cal = NULL;


  // update with new cal
  app->cal[ app->cal_slot_idx] = cal2;


  // free regression
  r_free( &regression );


  // combine xs, y and store in last.
  app->last = m_hconcat(xs, y, MNULL);

  M_FREE(xs);
  M_FREE(y);
}






void app_loop2 ( app_t *app )
{
  printf("=========\n");
  printf("app_loop2 - cal loop using permutation of nplc/aperture\n");

  printf("Using cal slot %u . for model and var_n,fix_n \n",  app->cal_slot_idx );

  /*
    alsways work with slot 0?
  */

  assert( app->cal_slot_idx < ARRAY_SIZE(app->cal));
  Cal *cal = app->cal[  app->cal_slot_idx ];
  assert(cal);
  // TODO initially, if no cal. then should create a default.


  // clear last for mem
  if(app->last) {
    M_FREE(app->last);
  }

  // write current cal modulation parameters (var_n,fix_n), but not aperture
  ctrl_reset_enable(app->spi);
  // ctrl_param_write( app->spi, &cal->param);
  ctrl_set_var_n( app->spi,  cal->param.clk_count_var_n);
  ctrl_set_fix_n( app->spi,  cal->param.clk_count_fix_n);
  ctrl_reset_disable(app->spi);


  printf("model %u\n", cal->model);
  param_show( & cal->param );
  printf("\n");

  // unsigned nplc_[] = { 9, 10, 11, 12 };
  // unsigned nplc[] = { 8, 9, 10, 11, 12, 13  };
  // unsigned nplc[] = { 8, 9, 10, 11, 12, 13, 14, 15, 16  };
  unsigned nplc[] = { 8, 9, 10, 11, 12, 13, 14, 15 };
  // double y  = 0;

  unsigned obs_n = 7; // 7
  // unsigned obs_n = 25; // 7

  /*
  just use the input parameter...

  */
#if 0
  Param params[] = {
    {
      .clk_count_aper_n = 0,
      .clk_count_fix_n = 20,
      .clk_count_var_n = 140,
      .old_serialization = 0,
    },
    {
      .clk_count_aper_n = 0,
      .clk_count_fix_n = 16,
      .clk_count_var_n = 144,
      .old_serialization = 0,
    },
    {
      .clk_count_aper_n = 0,
      .clk_count_fix_n = 24,
      .clk_count_var_n = 136,
      .old_serialization = 0,
    }

  };
#endif

#if 1
  // use the current cal input parameters
  Param params[1];
  params[0] = cal->param; // deep copy
#endif


  // may want a row pointer as well.
  unsigned  max_rows =  obs_n * ARRAY_SIZE(nplc) * 2 * ARRAY_SIZE(params);

  unsigned cols = 0;
  switch ( cal->model) {
    case 2: cols = 2; break;
    case 3: cols = 3; break;
    case 4: cols = 4; break;  // + intercept
    case 5: cols = 4; break;  // + flip_count
    default: assert(0);
  };

  MAT *xs       = m_get(max_rows, cols );
  MAT *y        = m_get(max_rows, 1);
  MAT *aperture = m_get(max_rows, 1); // required for predicted


  X1   x;
  memset(&x, 0, sizeof(x));
  x.app = app;
  // x.data_ready = false;


  // set handler
	spi1_interupt_handler_set(  (void (*)(void *))  app_loop1_spi1_interupt, &x );



  unsigned row = 0;

  // loop aperture nplc
  for(unsigned h = 0; h < ARRAY_SIZE(nplc); ++h)
  {
    int nplc_ = nplc[h];
    uint32_t aperture_ = nplc_to_aper_n( nplc_ );  // move this up a loop.
    printf("nplc   %u\n", nplc_    );

    ctrl_set_aperture( app->spi, aperture_);

    // loop mux
    for(unsigned j = 0; j < 2; ++j)
    {
      uint32_t mux = j == 0 ? HIMUX_SEL_REF_LO : HIMUX_SEL_REF_HI;
      double   y_  = j == 0 ? 0                : 7.1;

      printf("mux %s\n", himux_sel_format( mux));


      // ctrl_reset_enable(app->spi);
      ctrl_set_mux( app->spi, mux );
      // ctrl_reset_disable(app->spi);

      // params
      for(unsigned k = 0; k < ARRAY_SIZE(params); ++k) {

        Param *param = &params[ k ] ;

        param_show( param );
        printf("\n");

        // permute modulation params
        ctrl_reset_enable(app->spi);
        ctrl_set_var_n( app->spi, param->clk_count_var_n );
        ctrl_set_fix_n( app->spi, param->clk_count_fix_n );

        // set this explicitly
        x.data_ready = false;
        ctrl_reset_disable(app->spi);


        // obs
        for(unsigned i = 0; i < obs_n; ++i) {


          // block/wait for data
          while(!x.data_ready ) {
            app_update( app );   // change name simple update
            // mem leak?
            if(app->halt_func) {
              break;
            }
          }

          // we got and read the data, so clear the flag to be ready
          x.data_ready = false;


          #if 0
          ctrl_reset_enable(app->spi);
          app->data_ready = false;
          ctrl_reset_disable(app->spi);

          // block/wait for data
          while(!app->data_ready ) {
            app_update( app );
            if(app->halt_func) {
              return;
            }
          }

          // read the data and params.
          Run   run;
          // Param param;

          ctrl_run_read(   app->spi, &run, app->verbose);
          // ctrl_param_read( app->spi, &param);
          #endif

          // param_show(&param);
          run_show( &x.run, app->verbose );

          if(i < 2) {
            printf("discard");
          }
          else {

            // record xs
            assert(row < m_rows(xs));
            MAT *whoot = run_to_matrix( &x.run, cols , MNULL );
            assert(whoot);
            assert( m_cols(whoot) == m_cols(xs) );
            assert( m_rows(whoot) == 1  );

            // printf("\n");
            // m_foutput(stdout, whoot );
            m_row_set( xs, row, whoot );
            M_FREE(whoot);

            // record aperture
            assert(row < m_rows(aperture));
            // assert( param->clk_count_aper_n  == aperture_ );
            // m_set_val( aperture, row, 0, param->clk_count_aper_n );
            m_set_val( aperture, row, 0, aperture_ );


            // record y, as target * aperture
            assert(row < m_rows(y));
            m_set_val( y       , row , 0, y_  *  aperture_ );

            // increment row.
            ++row;
          }
          printf("\n");

        } // k
      } // i
    } // j
  } // h


  // restore handler
	spi1_interupt_handler_set(  (void (*)(void *))  app_spi1_interupt, app );



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


  calc_cal( app, y, xs, aperture );


}

















void app_voltage_source_1_set( app_t *app, double value )
{
  // using cap.
  // this has to read the adc - which makes it a lot more specific
  // than general app_( app_t *app)  code.
  // so put here, instead of app.c
  // change name current voltage

  double current = app_simple_read( app);

  if( value > current ) {

    // voltage_source_1_set_dir( int val ) ;
    voltage_source_1_set_dir(1);
    while(1) {
      current = app_simple_read( app);
      // printf("val %lf\n", current);
      printf(".");
      if(current > value)
        break;

      if(app->halt_func) {
        break;
      }
    }

    // should be renamed set_dir
    voltage_source_1_set_dir(0);


  } else {

    voltage_source_1_set_dir(-1);
    while(1) {
      current = app_simple_read( app);
      printf(".");
      // printf("val %lf\n", current);
      if(current < value)
        break;
      if(app->halt_func) {
        break;
      }
    }

    voltage_source_1_set_dir(0);
  }

  printf("\n");
}



void app_voltage_source_2_set( app_t *app, double value )
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

    // no need to test halt flag here, because this is fast
  }

  ctrl_run_read(app->spi, &run, app->verbose);
  ctrl_param_read(app->spi, &param);

  // we have both obs available...
  // assert(run.count_var_up);

  assert( app->cal_slot_idx < ARRAY_SIZE(app->cal));
  Cal *cal = app->cal[ app->cal_slot_idx ];
  assert(cal);
  assert(cal->b);


  double predict = m_calc_predicted_val( cal->b, &run, cal ->param.clk_count_aper_n);
  return predict;
}



///////////////////////////////////////////

struct X4
{
  bool  do_a;

  Param   *param_a;
  Run     run_a;


  Param   *param_b;
  Run     run_b;

  bool    data_ready;
  app_t   *app;   // for spi/ verbose... etc
} ;
typedef struct X4 X4;



static void app_loop4_spi1_interupt( X4 *x)
{
  /*
    interupt handler context.
    read data without pause
  */
  app_t *app = x->app;

  // should do_a first. so init it

  if(x->do_a) {
    // read a, then set to param b
    ctrl_run_read(app->spi, &x->run_a, app->verbose);
    ctrl_param_write( app->spi, x->param_b );
    x->do_a = false;

  } else {
    // read b, then set to param a
    ctrl_run_read(app->spi, &x->run_b, app->verbose);
    ctrl_param_write( app->spi, x->param_a );
    x->do_a = true;

    x->data_ready = true;
  }
}




void app_loop4 ( app_t *app,  unsigned cal_slot_a,  unsigned cal_slot_b  )
{
  /*
    EXTR.
      this routine - could also be used for calibration - just record the raw counts . and then run a best-fit regression.
      also could be done off, the mcu.

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

#if 0
  if( !spi_voltage_source_2_in_on(&app->spi_4094_reg)) {
    usart1_printf("spi_voltage_source_2 not on\n");
    return;
  }
#endif

  // indicate running
  app->led_blink_interval = 500;

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

/*
  double target_[] = {
    -11, -10.5, -10, -9.5, -9, -8.5, -8, -7.5, -7, -6.5, -6, -5.5, -5, -4.5, -4, -3.5, -3, -2.5, -2, -1.5, -1, -0.5, -0,
    0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10, 10.5, 11 } ;
*/

/*
  a larger voltage jump means more DA. so don't make coarser.
  double target_[] = {
    11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
    -1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11 } ;
*/


  // more obs == more spread due to DA.
  unsigned obs_n = 50;


  // OK. I think it would be clearaer to pass the arguments.

  // cal A
  assert( cal_slot_a < ARRAY_SIZE(app->cal));
  Cal *cal_a = app->cal[ cal_slot_a ] ;
  assert( cal_a );
  printf("cal a, slot %u\n", cal_slot_a);
  param_show( & cal_a->param );
  printf("\n");


  // cal B
  assert( cal_slot_b < ARRAY_SIZE(app->cal));
  Cal *cal_b = app->cal[ cal_slot_b ] ;
  assert( cal_b );
  printf("cal b, slot %u\n", cal_slot_b);
  param_show( & cal_b->param );
  printf("\n");


  X4  x;
  memset(&x, 0, sizeof(x));
  x.app = app;
  x.param_a = &cal_a->param;
  x.param_b = &cal_b->param;
  x.do_a = true;  // do a first


  // size array
  MAT *m = m_get( ARRAY_SIZE(target_) * obs_n , 5 );
  if(!m) {
    printf("failed to allocate\n");
    return;
  }


  for(unsigned i = 0; i < ARRAY_SIZE(target_); ++i)
  {

    double target = target_[i];

    // change to voltage
    printf("voltage set %.1f\n", target );
    app_voltage_source_1_set( app, target );

    // sleep to let DA settle.
    // unsigned sleep = 3;  // for dac
    unsigned sleep = i == 0 ? (30 * 10) : 30;
    printf("sleep %us\n", sleep );
    app_simple_sleep( app, sleep * 1000 );

    /* - I think we probably want to be able to do a loop of 2. to not take the first value.
    */
    // record first, and last, to be able to estimate DA
    double first = 0, last = 0;;


    // do a first
    x.do_a = true;
    // set handler
    spi1_interupt_handler_set(  (void (*)(void *))  app_loop4_spi1_interupt, &x );


    // 10 obs
    for(unsigned obs = 0; obs < obs_n; ++obs)
    {

      printf("blocking\n");

      // block/wait for data
      while(!x.data_ready ) {
        app_update( app );   // change name simple update
        // mem leak?
        if(app->halt_func) {
          break;
        }
      }

      // we got and read the data, so clear the flag to be ready
      x.data_ready = false;

      // work out A,B difference

      assert(cal_a->b && cal_b->b );

      double predict_a  = m_calc_predicted_val( cal_a->b, &x.run_a, cal_a->param.clk_count_aper_n );
      double predict_b  = m_calc_predicted_val( cal_b->b, &x.run_b, cal_b->param.clk_count_aper_n );
      double delta      = (predict_a - predict_b) * 1000000; // in uV.

      if(app->verbose) {

        run_show( &x.run_a, app->verbose);
        printf("%f\n", predict_a );

        run_show( &x.run_b, app->verbose );
        printf("%f\n", predict_b );
      }

      // simple spread chack
      if(obs == 0)          first = predict_a;
      if(obs == obs_n - 1)  last  = predict_a;


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

    // restore handler
    spi1_interupt_handler_set(  (void (*)(void *))  app_spi1_interupt, app );



    printf("a first %f last %f diff %f.2uV \n" , first, last, (last - first) * 1000000 );

  } // target loop



  // shrink matrix
  m_resize( m, row, m_cols(m));


  // set to flush
  ffnctl( stdout, ffnctl( stdout, 0) | SYNC_OUTPUT_ON_NEWLINE );
  m_octave_foutput( stdout, NULL, m);

  ffnctl( stdout, ffnctl( stdout, 0) & ~SYNC_OUTPUT_ON_NEWLINE );

  app->last = m;

  // switch the blink interval to fast. to indicate done.

  // reset voltage - for DA. ssame as when start.
  printf("resetting voltage to 0");
  app_voltage_source_1_set( app, 0);

  // indicate done
  app->led_blink_interval = 250;

}


#if 0
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
#endif





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

