/*
  EXTR. REVIEW
  need to consider - if should use AZ mode here, to reduce flicker, to get a better stderr, and improved fit.
  or interleve data collection with ref-hi/ref-lo -
  ---
  EXTR. remember we can always use the azmux - to get the lo. via the lo-mux.
  the fpga has control over the azmux. so it doesn't need the slow spi mode transition.


  the only issue - really seems to be that the input path is different. and doesn't equalize the relays - with seebeck effect of disjoint metals.

*/
/*
  replicate function app_cal(app ) .  from dmm


  can pass the yield function here, down from app,
  but note - the yield should not process adc raw data / counts.

  as a strategy - it is simpler/less state interaction - if this function is written to take over control/and block and manage everything relayed to cal.
  ------

  OK. issue is that cal has to set up the mode. to control ref voltages etc.
  BUT. perhaps we can handle from here?


*/

#include <stdio.h>
#include <assert.h>
#include <math.h>     // fabs


#include <ice40-reg.h>
#include <peripheral/spi-ice40.h>

#include <lib2/util.h>    // msleep

#include <mode.h>
#include <util.h> // nplc_to_aperture()

#include <data/data.h>

#include <data/matrix.h> // m_set_row()
#include <data/regression.h>





// we will need to pass the spi also.


/*
  - could pass functions to set up the two ref voltages.
  - and setup the measurement function (az, not az ) to use.

*/

/*
    could be used in app also.
*/

// we need system mills
// HMMMM.  there's quite a bit of data we need.
// where are we going to store the cal?
// in data.

// perhaps line_freq should go in data...





static void print_slope_b_detail( unsigned aperture, double slope_b )
{
  // better name print_slope_b_detail.

  // unsigned aperture =   nplc_to_aper_n( 10 );

  // double   slope_b    = m_get_val(cal->b, slope_idx, 0 );   // rows
  double   res        = fabs( slope_b / aperture ); // in V
  // could also work out the implied count here.

  printf("res       %.3fuV  ", res * 1000000);  // resolution  in uV.
  printf("digits %.2f ", log10( 10.f / res));   // ie. decimal=10 not +-11V
  // printf("bits %.2f ", log2( res));           // correct?   or should be aperture / slobe_b ?
  printf("  (nplc10)\n");
}




void data_cal( data_t *data , uint32_t spi, _mode_t *mode,  volatile uint32_t *system_millis   /* void (*yield)( void * ) */ )
{
  assert(data);
  assert(data->magic == DATA_MAGIC) ;
  assert(mode);



  // set. in default?
  data->model_cols = 3;


  // unsigned  max_rows =  obs_n * ARRAY_SIZE(nplc) * 2 /** ARRAY_SIZE(params)*/;
  unsigned max_rows = 10;

  printf("whoot cal() \n");



  // storage
  MAT *xs       = m_get(max_rows, data->model_cols );
  MAT *y        = m_get(max_rows, 1);
  MAT *aperture = m_get(max_rows, 1); // required to calc predicted
  MAT *row      = NULL;

/*
  m_truncate_rows(xs, 0);
  m_truncate_rows(y, 0);
  m_truncate_rows(aperture, 0);
*/


  // setup input relays - for dcv-source
  mode->first.K405 = LR_SET;     // select dcv. TODO change if support himux.
  assert( mode->first.K406 == LR_SET);   // accum relay off
  mode->first.K407 = LR_RESET;   // select dcv-source on


  // set up sequence acquision
  mode->reg_mode = MODE_SA_ADC;
  mode->sa.reg_sa_p_seq_n  = 2;
  mode->sa.reg_sa_p_seq0 = (PC01 << 4) | S3;        // dcv,
  mode->sa.reg_sa_p_seq1 = mode->sa.reg_sa_p_seq0 ;         // the same
  mode->trigger_source_internal = 1;


  // setup adc nplc
  mode->adc.reg_adc_p_aperture = nplc_to_aperture( 10, data->line_freq );;




  // this is horrible.
  // just overside the matrix and use push_row
  unsigned row_idx = 0;

  // ref hi/ref lo
  for(unsigned j = 0; j < 2; ++j ) {


    double y_ = 0;
    if(j == 0) {
      y_ = 7;   // ref-hi / 7V
      mode_set_ref_source(  mode, 7);
    } else {
      y_ = 0;  // ref-lo / 0V
      mode_set_ref_source(  mode, 0);
    }


    printf("spi_mode_transition_state()\n");
    spi_mode_transition_state( spi, mode, system_millis);

    // note, adc is triggered/running here, even as we sleep

    printf("sleep\n");
    //  let things settle from spi emi burst, and board DA settle.
    msleep(1 * 1000, system_millis);


    // take obs loop
    for(unsigned i = 0; i < 5; ++i ) {


      // block on interupt.
      while(! data->adc_measure_valid ) {
        // yield( yield_ctx);
      }
      data->adc_measure_valid = false;


      // embed a 8 bit. counter ini the reg_status and use it for the measure.
      // uint32_t status =            spi_ice40_reg_read32( app->spi, REG_STATUS );

      uint32_t clk_count_mux_reset  = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_RESET);   // time refmux is in reset. useful check. not adc initialization time.
      uint32_t clk_count_mux_neg    = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_NEG);
      uint32_t clk_count_mux_pos    = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_POS);
      uint32_t clk_count_mux_rd     = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_RD);
      uint32_t clk_count_mux_sig    = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_MUX_SIG);


      printf("counts %6lu %lu %lu %6lu %lu", clk_count_mux_reset, clk_count_mux_neg, clk_count_mux_pos, clk_count_mux_rd, clk_count_mux_sig);

      printf("\n");

      // consider rename. this is more model_encode_row_from_counts()) - according to the model.
      // taking model as first arg
      row = run_to_matrix( clk_count_mux_neg, clk_count_mux_pos, clk_count_mux_rd, data->model_cols, row);


      mat_set_row( xs,       row_idx,  row ) ;
      vec_set_val( y,        row_idx,   y_  *  clk_count_mux_sig );
      vec_set_val( aperture, row_idx, clk_count_mux_sig);
      ++row_idx;

      /*
      m_push_row( xs,       row ) ;
      vec_push_row_val( y,  y_  *  clk_count_mux_sig );
      vec_set_val( aperture, row_idx, clk_count_mux_sig);
      */


    }

  }


  // shrink matrixes to size collected data
  m_resize( xs, row_idx, m_cols( xs) );
  m_resize( y,  row_idx, m_cols( y) );
  m_resize( aperture, row_idx, m_cols( aperture) ); // we don't use aperture


  printf("xs\n");
  m_foutput( stdout, xs );
  // usart1_flush();  // block until data flushed.
                      // helps, but issue is uart circular buffers still overflow
                      // could also just sleep for a bit.


  ///////////////////////////////////////////////////////
  // calc_cal( app, y, xs, aperture );

  assert(m_rows(y) == m_rows(xs));
  assert(m_rows(y) == m_rows(aperture));

  regression_t regression;

  m_regression( xs, y, &regression );   // rename reg_regression()...
  // usart1_flush();

  // TODO change name regression_report.
  // or _output..
  r_regression_show( &regression, stdout);
   //usart1_flush();



  {
    // print some stats
    uint32_t aperture_ = nplc_to_aperture( 10, data->line_freq );

    double sigma_div_aperture = regression.sigma / aperture_  * 1e6; // 1000000;  // in uV.

    printf("stderr(V) %.2fuV  (nplc10)\n", sigma_div_aperture);

    double last_b_coefficient = m_get_val(regression.b ,   0,   m_rows(regression.b) - 1);

    print_slope_b_detail( aperture_, last_b_coefficient);
  }




}

/*
  mar 17. initial code.  with just  10obs.

  stderr(V) 1.04uV  (nplc10)
  res       0.040uV  digits 8.40   (nplc10)
  calling spi_mode_transition_state()

*/






/*  - OK. it doesn't matter whether aperture is for one more extra clk cycle. or one less.  eg. the clk termination condition.
    instead what matters is that the count is recorded in the same way, as for the reference currents.
    eg. so should should always refer to the returned count value, not the aperture ctrl register.

    uint32_t clk_count_mux_sig = spi_ice40_reg_read32( app->spi, REG_ADC_P_APERTURE );
*/




/* / would it be better to interleave ref-hi/ref-lo
// not sure - need to write the whole 4094 state.  so it takes time.
// but could do a RM like input.
    - eg. dcv-source-1  on dcv input relay  .   so switch ref-hi/lo.
    really not sure - if couldn't just use the lo. via the azmux.
*/


/*
  // set up the dcv source
  mode_set_dcv_source( mode, 10 );

  // setup the adc
  mode->reg_mode = MODE_SA_ADC;       // set mode adc.
  mode->trigger_source_internal = 1;  // turn on adc  // perhapkkkkk
                                      // TODO. perhaps set to 0. and then manually trigger, with direct reg write.

*/


  /*
    note to change the cal reference voltage - eg. ref_hi, ref_lo.  we have to do a full 4094 spi xfer.
    which is a full transmission.
    so this is different from previous calibration routine, where could write the hi-mux directly from .
    ----

    EXTR.   is there a reason not to sample the lo - from the azmux ?????????
    hmmmm....
    because its not ref-lo.
    and because different input path - will have different metal disjunction paths and offsets - from relays.
  */

  // we want to manipulate the seqac trigger - manually.


