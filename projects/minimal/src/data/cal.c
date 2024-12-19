/*
  we have to try the 2 var model again, with offset.
    eg. +ref,-ref, and offset for Vos+ref-lo diff.
  - there's also a question - should gnd current comp be used - to compensate Vos - and zero the offset.

  - eg. tie signal input to star- lo , and integrate.  and it should go nowhere.
*/
/*
  remember stderr is check of fit.
    and reflects noise
      but also it is a basic check of linearity.  for the runup parameters.
*/

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


#include <peripheral/spi-ice40.h>
#include <device/fpga0_reg.h>

#include <lib2/util.h>    // yield_with_msleep

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






void data_cal_show( data_t *data )
{
  if( !data->model_b) {
    printf("no model\n");
    return;
  }


  m_foutput( stdout, data->model_b );

  printf("model_id    %u\n", data->model_id );
  printf("model_spec  %u\n", data->model_spec );

  printf("stderr(V)   %.2fuV  (nplc10)\n", data->model_sigma_div_aperture * 1e6); // in uV.

#if 0

  // none of this works with a two variable model.
  // print some stats
  uint32_t aperture_          = nplc_to_aperture( 10, data->line_freq );

  double last_b   = m_get_val(data->model_b ,   0,   m_rows( data->model_b) - 1);
  double   res        = fabs( last_b / aperture_ ); // in V
  // could also work out the implied count here.

  printf("res         %.3fuV  ", res * 1e6 );  // resolution  in uV.
  printf("digits      %.2f (nplc 10)", log10( 10.f / res));   // ie. decimal=10 not +-11V
  // printf("bits %.2f ", log2( res));           // correct?   or should be aperture / slobe_b ?

#endif

}




void data_cal(
  data_t *data ,

  // uint32_t spi,
  spi_ice40_t * spi,    // TODO change name spi_ice40 and type. of spi_t.
  spi_t *spi_4094,
  spi_t *spi_ad5446,


  _mode_t *mode,
  unsigned model_spec,
  volatile uint32_t *system_millis,
  void (*yield)( void * ),
  void * yield_ctx
)
{
  assert(data);
  assert(data->magic == DATA_MAGIC) ;
  assert(mode);


  printf("whoot cal() \n");



  // note that we are comitted, at this point, by setting fields on data
  data->model_spec = model_spec;


  const unsigned obs_to_take_n = 7; // how many obs to take
  const unsigned nplc[] = { 8, 9, 10, 11, 12, 13, 14, 15, 16  };
  // unsigned  max_rows =  obs_to_take_n * ARRAY_SIZE(nplc) * 2 /** ARRAY_SIZE(params)*/;
  unsigned  max_rows =  obs_to_take_n * ARRAY_SIZE(nplc) * 2 ;
  // unsigned max_rows = 10;



  // storage
  MAT *xs       = m_get(max_rows, model_spec_cols( data->model_spec ));
  MAT *y        = m_get(max_rows, 1);
  MAT *aperture = m_get(max_rows, 1); // required to calc predicted
  MAT *row      = NULL;

/*
  m_truncate_rows(xs, 0);
  m_truncate_rows(y, 0);
  m_truncate_rows(aperture, 0);
*/

/*
  // setup input relays - for dcv-source
  mode->first.K405 = SR_SET;     // select dcv. TODO change if support himux.
  assert( mode->first.K406 == SR_SET);   // accum relay off
  mode->first.K407 = SR_RESET;   // select dcv-source on
*/
/*
  mode->first .K407 = SR_SET;    // select dcv-source on ch1.
  mode->first .K405 = SR_SET;     // select ch1. to feed through to accum cap.
  mode->first .K406 = SR_SET;   // select accum cap
*/

  // dec 2024.
  // set up input relays.
  mode_set_dcv_source_ref( mode, 0 );
  mode_set_dcv_source_channel( mode, 1 ); // dcv


  // TODO. use mode_set_seq function.
 //  void mode_set_seq( _mode_t *mode, uint32_t seq_mode , uint8_t arg0, uint8_t arg1 )

  // set up sequence acquision
  mode->reg_mode = MODE_SA_ADC;
  mode->sa.reg_sa_p_seq_n  = 2;
  mode->sa.reg_sa_p_seq0 = (PC01 << 4) | S1;          // dcv,  update dec 2024.
  mode->sa.reg_sa_p_seq1 = mode->sa.reg_sa_p_seq0;    // the same



  // mode->trig_sa = 1;
  mode_set_trigger( mode, true);


  // this isnt' that nice. versus pushing a reserve sized array but is reasonably simple.
  // just overside the matrix and use push_row
  unsigned row_idx = 0;



  for(unsigned h = 0; h < ARRAY_SIZE(nplc); ++h)
  {

    printf("nplc %u\n", nplc[h]);

    // setup adc nplc
    mode->adc.reg_adc_p_aperture = nplc_to_aperture( nplc[ h] , data->line_freq ); // fix jul 2024.


    // ref hi/ref lo
    for(unsigned j = 0; j < 2; ++j ) {


      double y_ = 0;
      if(j == 0) {
        y_ = 7;   // ref-hi / 7V
        mode_set_dcv_source_ref(  mode, 7);
      } else {
        y_ = 0;  // ref-lo / 0V
        mode_set_dcv_source_ref(  mode, 0);
      }

      // start adc,
      printf("spi_mode_transition_state()\n");


      spi_mode_transition_state( spi, spi_4094, spi_ad5446, mode, system_millis);


      // let things settle from spi emi burst, and board DA settle, amp to come out of lockup.
      // equivalent to discarding values
      printf("sleep\n");
      yield_with_msleep( 1 * 1000, system_millis, yield, yield_ctx);


      // take obs loop
      for(unsigned i = 0; i < obs_to_take_n; ++i ) {

        // wait for adc data, on interupt
        while( !data->adc_interupt_valid ) {
          if(yield)
            yield( yield_ctx);
        }
        data->adc_interupt_valid = false;

        // embed a 8 bit. counter ini the reg_status and use it for the measure.
        // uint32_t status =            spi_ice40_reg_read32( app->spi, REG_STATUS );
        uint32_t clk_count_mux_reset  = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_RESET);   // time refmux is in reset. useful check. not adc initialization time.
        uint32_t clk_count_mux_neg    = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_NEG);
        uint32_t clk_count_mux_pos    = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_POS);
        uint32_t clk_count_mux_rd     = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_RD);
        uint32_t clk_count_mux_sig    = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_MUX_SIG);

        printf("counts %6lu %lu %lu %lu %6lu",
          clk_count_mux_reset,
          clk_count_mux_sig,
          clk_count_mux_neg, clk_count_mux_pos, clk_count_mux_rd
        );
        printf("\n");

        // consider rename. this is more model_encode_row_from_counts()) - according to the model.
        // at least adc_counts_to_model()
        // taking model as first arg

        row = run_to_matrix( clk_count_mux_neg, clk_count_mux_pos, clk_count_mux_rd, model_spec_cols(data->model_spec), row);

        mat_set_row( xs,       row_idx,  row ) ;
        vec_set_val( y,        row_idx,   y_  *  clk_count_mux_sig );
        vec_set_val( aperture, row_idx, clk_count_mux_sig);
        ++row_idx;

        /*
        m_push_row( xs,       row ) ;
        vec_push_row_val( y,  y_  *  clk_count_mux_sig );
        vec_set_val( aperture, row_idx, clk_count_mux_sig);
        */
      } // i
    } // j
  } // h


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


  uint32_t aperture_          = nplc_to_aperture( 10, data->line_freq );
  data->model_sigma_div_aperture   = regression.sigma / aperture_;
/*
  {
    // print some stats


    printf("stderr(V) %.2fuV  (nplc10)\n", data->model_sigma_div_aperture * 1e6);   // in uV.

    double last_b_coefficient   = m_get_val(regression.b ,   0,   m_rows(regression.b) - 1);

    data_print_slope_b_detail( aperture_, last_b_coefficient);
  }
*/


  // copy to data->b
  // set data->b size first, because m_copy() does not change dims.
  // TODO - better to just copy. then reallocate and shrink
  data->model_b = m_resize(data->model_b,  m_rows( regression.b ), m_cols( regression.b));
  assert(data->model_b->m == regression.b->m && data->model_b->n == regression.b->n);

  // note the predicted values are still in the regression structure.
  data->model_b       = m_copy( regression.b, data->model_b );

  // free regression
  r_free( &regression );

  //////////////////////////



  m_free(row);
  m_free(xs);
  m_free(y);
  m_free(aperture);


  // print some stats
  data_cal_show( data );

}

// ok. we want the ability to save the cal. so can reuse.
// want to fix the amp inductor.
// high variance - indicates perhaps flicker noise.

/*


  mar 27.
    r      1.0000000000                            12.15am.       Ok. this looks a lot better.
    stderr(V) 0.67uV  (nplc10)
    res       0.233uV  digits 8.63   (nplc10)
    calling spi_mode_transition_state()

  -------
  stderr(V) 0.49uV  (nplc10)
  res       0.043uV  digits 9.37   (nplc10)     12.12pm

        weird?????  9 digtss.


  mar 27 2024.
    after changing lt1021 to ltz. and running about 15mins.  unshielded

    stderr(V) 0.91uV  (nplc10)                          11.30am.
    res       0.271uV  digits 8.57   (nplc10)

    stderr(V) 1.01uV  (nplc10)
    res       0.174uV  digits 8.76   (nplc10)




  -------------------
  stderr(V) 1.03uV  (nplc10)
  res       0.023uV  digits 8.65   (nplc10)

  no change. nice.
  stderr(V) 1.61uV  (nplc10)
  res       0.030uV  digits 8.52   (nplc10)

  after adding inductor
  stderr(V) 1.51uV  (nplc10)
  res       0.022uV  digits 8.65   (nplc10)

  no change
  stderr(V) 3.19uV  (nplc10)
  res       0.036uV  digits 8.45   (nplc10)


  no change.
  stderr(V) 1.12uV  (nplc10)
  res       0.028uV  digits 8.55   (nplc10)
  calling spi_mode_transition_state()


  no change from previous. better.
  stderr(V) 1.64uV  (nplc10)
  res       0.026uV  digits 8.59   (nplc10)


  after disconnecting leads.
  stderr(V) 3.40uV  (nplc10)
  res       0.025uV  digits 8.61   (nplc10)


  using all nplc, and - with mso and scope leads connected.   and no amplifier inductor. and lt1021 ref, no shielding. uncleaned

  stderr(V) 4.17uV  (nplc10)
  res       0.020uV  digits 8.69   (nplc10)
  calling spi_mode_transition_state()

  max system bytes =      14500
  system bytes     =      14500
  in use bytes     =       1364
  sp 0x2004ff0c   327436
  calling spi_mode_transition_state()




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
  mode->trig_sa = 1;  // turn on adc  // perhapkkkkk
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


