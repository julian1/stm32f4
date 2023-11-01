
#include <assert.h>
#include <stdint.h>



#include "calc.h"





void m_stats_print( MAT *buffer /* double *mean, double *stddev */ )
{
/*
    should just take some - doubles as arguments. .printing

    needs to return values, and used with better formatting instructions , that are not exposed here.
    format_float_with_commas()

*/

  assert(buffer);
  assert( m_cols(buffer) == 1);

  // take the mean of the buffer.
  MAT *mean = m_mean( buffer, MNULL );
  assert( m_is_scalar( mean ));
  double mean_ = m_to_scalar( mean);
  M_FREE(mean);



  MAT *stddev = m_stddev( buffer, 0, MNULL );
  assert( m_is_scalar( stddev ));
  double stddev_ = m_to_scalar( stddev);
  M_FREE(stddev);

  // report
  // char buf[100];
  // printf("value %sV ",          format_float_with_commas(buf, 100, 7, value));

  // printf("mean(%u) %.2fuV, ", m_rows(buffer),   mean_ * 1e6 );   // multiply by 10^6. for uV
  printf("mean(%u) %.7fV, ", m_rows(buffer),   mean_  );

  printf("stddev(%u) %.2fuV, ", m_rows(buffer), stddev_  * 1e6 );   // multiply by 10^6. for uV

  // printf("\n");


}





bool push_buffer1( MAT *buffer, unsigned *i, double value)
{
  // better name  m_push_scalar()

  assert(buffer);
  assert(i);
  assert( m_cols(buffer) == 1 );

  bool full = false;
  if(*i >= m_rows(buffer)) {
    full = true;
    *i = 0;
  }

  m_set_val( buffer, *i, 0, value );
  ++(*i);

  return full;
}




MAT * run_to_matrix(
    // const Run *run,
    uint32_t clk_count_mux_neg,
    uint32_t clk_count_mux_pos,
    uint32_t clk_count_mux_rd,
    unsigned model,
    MAT * out
)
{
  // change name ,   adc_counts_to_matrix() ?


  /*
    we have aperture stored in the Param.
    - it's easy to pull off device.  so perhaps it should be moved to Run. or else store in both Run and Param.
    - OR. it should always be passed here. - because it is a fundamental data on device, and for calculating predicted..
    ------
    - not sure. we want to test a model with aperture as independent var.

  */

  // TODO can we move this inside each if clause?
  if(out == MNULL)
    out = m_get(1,1);


  if(model == 2) {
    /*
      more constrained.
      rundown that has both currents on - just sums
      this is nice because doesn't require anything on fpga side.
    */
    double x0_ = clk_count_mux_neg + clk_count_mux_rd;
    double x1_ = clk_count_mux_pos + clk_count_mux_rd;

    out = m_resize(out, 1, 2);
    m_set_val( out, 0, 0,  x0_ );
    m_set_val( out, 0, 1,  x1_  );
  }

  else if( model == 3) {

    out = m_resize(out, 1, 3);
    m_set_val( out, 0, 0,  clk_count_mux_neg );
    m_set_val( out, 0, 1,  clk_count_mux_pos );
    m_set_val( out, 0, 2,  clk_count_mux_rd );
  }

/*
  EXTR.
    - try adding apperture as independent variable.
    try a model that includes aperture. ie. if there are small changes between nplc=1, nplc=10
    then perhaps just including aperture.

*/
  else if ( model == 4) {

    out = m_resize(out, 1, 4);
    m_set_val( out, 0, 0,  1.f ); // ones, offset
    m_set_val( out, 0, 1,  clk_count_mux_neg );
    m_set_val( out, 0, 2,  clk_count_mux_pos );
    m_set_val( out, 0, 3,  clk_count_mux_rd);
  }

#if 0
  else if( model == 5) {

    out = m_resize(out, 1, 4);
    m_set_val( out, 0, 0,  x0 );
    m_set_val( out, 0, 1,  x1  );
    m_set_val( out, 0, 2,  x2  );
    m_set_val( out, 0, 3,  x3  ); // flip_count
  }
#endif


  else assert( 0);

  return out;
}






void mat_set_row (  MAT *xs, unsigned row_idx,   MAT *whoot )
{
  // set row. or push row.
  assert(xs);
  assert(whoot);
  assert(row_idx < m_rows(xs));


  assert( m_cols(whoot) == m_cols(xs) );
  assert( m_rows(whoot) == 1  );

  m_row_set( xs, row_idx, whoot );

}


void vec_set_val (  MAT *xs, unsigned row_idx,   double x)
{
  assert(xs);
  assert( m_cols(xs) == 1  );
  assert(row_idx < m_rows(xs));

  m_set_val( xs , row_idx, 0, x );

}







MAT * m_calc_predicted( const MAT *b, const MAT *x, const MAT *aperture)
{
  /*
    - careful - this function may crash on embedded - due to memoryeneeds of m_mlt().
      actually it shouldn't be too bad - compared with decomposition.
    do matrix multiply, and adjust by the aperture.
  */

  // don't free input arguments
  // b is 4x1, x is nx4

  assert( m_cols(x) == m_rows( b) );
  assert( m_rows(x) == m_rows( aperture) );

  // matrix multiply
  MAT *predicted = m_mlt(x, b, MNULL );

  MAT *corrected = m_element_div( predicted, aperture, MNULL );

  M_FREE(predicted );

/*
  printf("corrected\n");
  m_foutput(stdout, corrected);
  usart1_flush();
*/

  return corrected;
}


#if 0
// we want a single file for all these functions
// also no dynamic allocation for Cal structure.


void calc_cal( app_t *app,  MAT *y, MAT *xs, MAT *aperture  )
{
  // Not sure about this function.
  // because we should not expose app here.


  // this doesn't actually need the aperture.
  // except if trying to

  UNUSED(app);
  UNUSED(aperture);

  assert(m_rows(y) == m_rows(xs));
  assert(m_rows(y) == m_rows(aperture));

  // rename app_calc_cal()
  /*
      calc_calibration_from_data
    better name. do_calibration.
    note uses replaces existing calibration from slot.
  */


  R regression;

  m_regression( xs, y, &regression );   // rename reg_regression()...


   usart1_flush();

  // TODO change name regression_report.
  // or _output..
  r_regression_show( &regression, stdout);

   usart1_flush();

  // note the predicted values are in the regression structure.


  // app->b       = m_copy( regression.b, MNULL );
  app->b       = m_copy( regression.b, app->b );

  // free regression
  r_free( &regression );



#if 0
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

  // TODO FIXME. should be deallocated in same plae/function as allocated.
  M_FREE(xs);
  M_FREE(y);
#endif
}

#endif

