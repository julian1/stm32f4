/*
  - long running process.
  - just take control of the main loop.
  as alternative to function yielding.
  ------
  Advantage.   for long running storage for loop - we can isoalte the memory and don't even need to pass it.
*/


#include <libopencm3/stm32/gpio.h>    // led

#include <stdio.h>    // printf, scanf
#include <string.h>   // strcmp, memset
#include <assert.h>



#include "usart.h"  // flush
#include "app.h"
#include "util.h"
#include "mux.h"
#include "reg.h"
#include "mode.h"   // we need to set up 4094 state *and* reg_direct


#include "spi-ice40.h"

#include "regression.h"
#include "calc.h"

// loop3 structure. should probably be in own header file.





MAT * run_to_matrix( // const Run *run,
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






static void mat_set_row (  MAT *xs, unsigned row_idx,   MAT *whoot )
{
  // set row. or push row.
  assert(xs);
  assert(whoot);
  assert(row_idx < m_rows(xs));


  assert( m_cols(whoot) == m_cols(xs) );
  assert( m_rows(whoot) == 1  );

  m_row_set( xs, row_idx, whoot );

}


static void vec_set_val (  MAT *xs, unsigned row_idx,   double x)
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


// we want a single file for all these functions
// also no dynamic allocation for Cal structure.


static void calc_cal( app_t *app,  MAT *y, MAT *xs, MAT *aperture  )
{

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


#if 0
  // this code seems to crash.
  // EXTR. predicted is already calculated in the Regression. 
  // predicted must be adjusted by aperture
  MAT *predicted =  m_calc_predicted( regression.b, xs, aperture );

  printf("\npredicted\n");
  m_foutput(stdout, predicted);
    usart1_flush();

  M_FREE(predicted);
#endif

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









void app_loop3( app_t *app )
{


  const unsigned nplc[] = { 8, 9, 10, 11, 12, 13, 14, 15, 16  };
  const unsigned obs_n = 7; // 7


  // may want a row pointer as well.
  unsigned  max_rows =  obs_n * ARRAY_SIZE(nplc) * 2 /** ARRAY_SIZE(params)*/;

  unsigned cols = 4;
/*
  switch ( cal->model) {
    case 2: cols = 2; break;
    case 3: cols = 3; break;
    case 4: cols = 4; break;  // + intercept
    case 5: cols = 4; break;  // + flip_count
    default: assert(0);
  };
*/
  /*
      we could store / presist the matrix vars - on a structure in app.
      to reduce allocation.
      alternatively test - allocation/deallocation counts around this function.
  */
  MAT *xs       = m_get(max_rows, cols );
  MAT *y        = m_get(max_rows, 1);
  MAT *aperture = m_get(max_rows, 1); // required for predicted


  UNUSED(y);
  // UNUSED(aperture);


  ////////////////////////////////////-----------------
  // this overrides nplc/aperture.
  *app->mode_current = *app->mode_initial;
  //  alias to ease syntax
  Mode *mode = app->mode_current;
  // ampliier 1x. default?
  // input relays are all off

  mode->reg_mode = MODE_NO_AZ;

  mode->reg_direct.sig_pc_sw_ctl  = SW_PC_SIGNAL;   // pc switch muxes signal.
  mode->reg_direct.azmux          = S1;             // azmux muxes pc-out // EXTR. review.  not muxing ref-lo here.
  mode->reg_direct.himux2 = S5 ;    // ref-hi.
  mode->reg_direct.himux  = S2 ;    // himux2

  mode->reg_adc_p_aperture = nplc_to_aper_n( 10, app->lfreq );    // dynamic. problem. maybe 50,60Hz. or other.

  // do the state transition
  app_transition_state( app->spi, mode,  &app->system_millis );

  ////////////////////////////////


  // TODO review. we could perhaps premultiply the y by the aperture?

  unsigned row_idx = 0;
  MAT *row = NULL;


  // loop aperture/nplc
  for(unsigned h = 0; h < ARRAY_SIZE(nplc); ++h)
  {
    unsigned nplc_ = nplc[h];
    printf("nplc   %u\n", nplc_ );
    // uint32_t aperture_ = nplc_to_aper_n( nplc_ );  // move this up a loop.
    // uint32_t nplc_to_aper_n( double nplc, uint32_t lfreq )

    uint32_t p_aperture  = nplc_to_aper_n( nplc_, app->lfreq );

    spi_ice40_reg_write32(app->spi, REG_ADC_P_APERTURE , p_aperture);      // arm to halt.


    // loop hi/lo signal mux
    for(unsigned j = 0; j < 2; ++j)
    {

      double y_ = 0;

      if( j == 0) {
        printf("mux hi\n");
        mode->reg_direct.azmux    = S1;     // azmux muxes pc-out // EXTR. review.  not muxing ref-lo here.
        mode->reg_direct.himux    = S2 ;    // himux2
        mode->reg_direct.himux2   = S5 ;    // ref-hi.
        y_ = 7;

      } else if( j == 1) {
        printf("mux lo\n");
        mode->reg_direct.azmux    = S7;     // ref lo.
        mode->reg_direct.himux    = S2 ;    // himux2
        mode->reg_direct.himux2   = S5 ;    // ref-hi.
        y_ = 0;
      } else assert( 0);

      spi_ice40_reg_write_n(app->spi, REG_DIRECT, &mode->reg_direct, sizeof( mode->reg_direct) );

       // params
      // for(unsigned k = 0; k < ARRAY_SIZE(params)  /*&& !app->halt_func */; ++k) {


      // this would be in the  loop when changing parameters
      // don't think this is quite right.
      // we need a better reset mechanism.
      // or arm/halt is correct. - because we want the condition in which we are able to signal go.
      {
      spi_ice40_reg_write32(app->spi, REG_SA_ARM_TRIGGER, 0 );      // arm to halt.
      printf("arm and block\n");
      while( !  (spi_ice40_reg_read32(app->spi, REG_STATUS) & (1<<8) )) ;  // wait for adc to be ready/valid.
      printf("adc measure valid/ done\n");
      app->adc_drdy = false;
      printf("trigger/restart\n");
      spi_ice40_reg_write32(app->spi, REG_SA_ARM_TRIGGER, 1 );    // trigger. signal acquisition
      }

      // loop obs
      for(unsigned i = 0; i < obs_n; ++i ) {

        // block on interupt.
        while(! app->adc_drdy );
        app->adc_drdy = false;

        if(i < 2)
          continue;

        // construct matrix directly. without needing Run struct.
        uint32_t clk_count_mux_neg = spi_ice40_reg_read32( app->spi, REG_ADC_CLK_COUNT_MUX_NEG);
        uint32_t clk_count_mux_pos = spi_ice40_reg_read32( app->spi, REG_ADC_CLK_COUNT_MUX_POS);
        uint32_t clk_count_mux_rd  = spi_ice40_reg_read32( app->spi, REG_ADC_CLK_COUNT_MUX_RD);
        uint32_t clk_count_mux_sig = spi_ice40_reg_read32( app->spi, REG_ADC_CLK_COUNT_MUX_SIG);


        printf("loop3 data  %lu %lu %lu %lu\n", clk_count_mux_neg, clk_count_mux_pos, clk_count_mux_rd, clk_count_mux_sig);

        row = run_to_matrix(
          clk_count_mux_neg,
          clk_count_mux_pos,
          clk_count_mux_rd,
          cols,
          row
        );

        // printf("b\n");
        // m_foutput( stdout, row );

        mat_set_row( xs,       row_idx,  row ) ;

        vec_set_val( aperture, row_idx, clk_count_mux_sig);

        vec_set_val( y,        row_idx,   y_  *  clk_count_mux_sig );


        ++row_idx;
      }
    } // j hi/lo
  }   // h nplc




  // shrink matrixes to size collected data
  m_resize( xs, row_idx, m_cols( xs) );
  m_resize( y,  row_idx, m_cols( y) );
  m_resize( aperture, row_idx, m_cols( aperture) ); // we don't use aperture


  printf("xs\n");
  m_foutput( stdout, xs );
  usart1_flush(); // block


  // printf("aperture/mux_sig\n");
  // m_foutput( stdout, aperture );


  // think this should probably return the regression and the cal, or just the b..
  // or else pull the code up.
  // and we handle deallocation top level.

  calc_cal( app, y, xs, aperture );


  m_free(row);

  m_free(xs);
  m_free(y);
  m_free(aperture);

  // leave running as is
  // turn off. -
  spi_ice40_reg_write32(app->spi, REG_SA_ARM_TRIGGER, 0 );      // arm to halt.

  return ;
}








  // need to support fpga core reset. to ensure parameters are being respected.
  // or else sleeping.

  /*
    - Ok. am not sure we really need any of this.
    - we just pause, until we get a trigger.

    - DURING RECONFIGURATION.
        - just reconfigure. and wait for the adc to signal.
        - or a separate reset. flag but *not* a edge triggered halt.

        - Or we just need a valid clear.

  */

  // we do want the arm, and trigger. for external triggering etc.
  // but this is different to 'tarm'.
  // rather than call it arm. might be better to call wait.


  // the lo can/should be taken.  probably by changing the az-mux.
  // or even use the az mode. to take ref-hi, and ref-lo.
/*

  uint32_t spi = app->spi;

  // now do fpga state
  mux_ice40(spi);

  // set mode
  spi_ice40_reg_write32(spi, REG_MODE, MODE_NO_AZ );


  F f;
  memset(&f, 0, sizeof(f));

  // so need to set up the muxes - for the sample
  f.led0 = app->led_state;

  mux_ice40(app->spi);
  spi_ice40_reg_write_n(app->spi, REG_DIRECT, &f, sizeof(f) );


  // spi_ice40_reg_write_n(spi, REG_DIRECT,  &mode->reg_direct,  sizeof( mode->reg_direct) );

  // write the aperture. REG_ADC_P_APERTURE
  //  spi_ice40_reg_write32(spi, REG_ADC_P_APERTURE, mode->reg_adc_p_aperture );
*/



#if 0
static void loop3_update_soft_500ms(app_t *app)
{
  assert(app);

  app->led_state = ! app->led_state;

  // blink mcu led
  // be explicit. don't hide top-level state.
  if(app->led_state)
    gpio_clear( app->led_port, app->led_out);
  else
    gpio_set(   app->led_port, app->led_out);


  // hearbeat and led flash
  mux_ice40(app->spi);

  // use a magic number to blink the led. and test comms
  uint32_t magic = app->led_state ? 0b010101 : 0b101010 ;
  spi_ice40_reg_write32( app->spi, REG_LED, magic);
  uint32_t ret = spi_ice40_reg_read32( app->spi, REG_LED);
  if(ret != magic ) {

    printf("comms bads\n");
/*
    // comms no good
    char buf[ 100] ;
    printf("no comms, wait for ice40 v %s\n",  format_bits(buf, 32, ret ));
    app->comms_ok = false;
*/
    // REVIEW
    // return
  }

}
#endif


/*

typedef struct Loop3  {

  uint32_t  whoot;

} Loop3 ;
*/
