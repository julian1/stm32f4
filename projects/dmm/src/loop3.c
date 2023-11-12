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


#include "usart.h"
#include "util.h"   // ARRAY_SIZE
#include "reg.h"
// #include "mux.h"
#include "mode.h"

#include "app.h"

#include "spi-ice40.h"


#include "calc.h"




void app_cal( app_t *app )
{


  const unsigned nplc[] = { 8, 9, 10, 11, 12, 13, 14, 15, 16  };
  const unsigned obs_n = 7; // 7



  // may want a row pointer as well.
  unsigned  max_rows =  obs_n * ARRAY_SIZE(nplc) * 2 /** ARRAY_SIZE(params)*/;

  // unsigned cols = 4;
/*
  switch ( cal->model) {
    case 2: cols = 2; break;
    case 3: cols = 3; break;
    case 4: cols = 4; break;  // + intercept
    case 5: cols = 4; break;  // + flip_count
    default: assert(0);
  };
*/


  printf("model cols %u\n", app->model_cols );

  /*
      we could store / presist the matrix vars - on a structure in app.
      to reduce allocation.
      alternatively test - allocation/deallocation counts around this function.
  */
  MAT *xs       = m_get(max_rows, app->model_cols );
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
        mode->reg_direct.azmux    = AZMUX_PCOUT;     // signal
        mode->reg_direct.himux    = HIMUX_HIMUX2;    // himux2
        mode->reg_direct.himux2   = HIMUX2_REF_HI;    // ref-hi.
        // y_ = 7;  / lt1021/7V
        y_ = 7.1;   // ltz

      } else if( j == 1) {
        printf("mux lo\n");
        mode->reg_direct.azmux    = AZMUX_REF_LO;     // use ref lo. for calibration.
        mode->reg_direct.himux    = HIMUX_HIMUX2;    // doesn't matter. himux2
        mode->reg_direct.himux2   = HIMUX2_REF_HI;    // doesn't matter. ref-hi.  should probably be lo.
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
      app->adc_measure_valid = false;
      printf("trigger/restart\n");
      spi_ice40_reg_write32(app->spi, REG_SA_ARM_TRIGGER, 1 );    // trigger. signal acquisition
      }

      // loop obs
      for(unsigned i = 0; i < obs_n; ++i ) {

        // block on interupt.
        while(! app->adc_measure_valid );
        app->adc_measure_valid = false;

        if(i < 2)
          continue;

        // construct matrix directly. without needing Run struct.

        // embed a 8 bit. counter ini the reg_status and use it for the measure.
        // uint32_t status =            spi_ice40_reg_read32( app->spi, REG_STATUS );

        uint32_t clk_count_mux_reset  = spi_ice40_reg_read32( app->spi, REG_ADC_CLK_COUNT_REFMUX_RESET);
        uint32_t clk_count_mux_neg    = spi_ice40_reg_read32( app->spi, REG_ADC_CLK_COUNT_REFMUX_NEG);
        uint32_t clk_count_mux_pos    = spi_ice40_reg_read32( app->spi, REG_ADC_CLK_COUNT_REFMUX_POS);
        uint32_t clk_count_mux_rd     = spi_ice40_reg_read32( app->spi, REG_ADC_CLK_COUNT_REFMUX_RD);
        uint32_t clk_count_mux_sig    = spi_ice40_reg_read32( app->spi, REG_ADC_CLK_COUNT_MUX_SIG);

      /*  - OK. it doesn't matter whether aperture is for one more extra clk cycle. or one less.  eg. the clk termination condition.
          instead what matters is that the count is recorded in the same way, as for the reference currents.
          eg. so should should always refer to the returned count value, not the aperture ctrl register.

          uint32_t clk_count_mux_sig = spi_ice40_reg_read32( app->spi, REG_ADC_P_APERTURE );
      */

        printf("counts %6lu %lu %lu %6lu %lu", clk_count_mux_reset, clk_count_mux_neg, clk_count_mux_pos, clk_count_mux_rd, clk_count_mux_sig);


        if(true || app->verbose) { 
          uint32_t stat_count_refmux_pos_up = spi_ice40_reg_read32( app->spi, REG_ADC_STAT_COUNT_REFMUX_POS_UP);
          uint32_t stat_count_refmux_neg_up = spi_ice40_reg_read32( app->spi, REG_ADC_STAT_COUNT_REFMUX_NEG_UP);
          uint32_t stat_count_cmpr_cross_up = spi_ice40_reg_read32( app->spi, REG_ADC_STAT_COUNT_CMPR_CROSS_UP);

          printf(", stats %lu %lu %lu", stat_count_refmux_pos_up, stat_count_refmux_neg_up, stat_count_cmpr_cross_up );
        } 

        printf("\n");


        row = run_to_matrix(
          clk_count_mux_neg,
          clk_count_mux_pos,
          clk_count_mux_rd,
          app->model_cols,
          row
        );

        // printf("b\n");
        // m_foutput( stdout, row );

        mat_set_row( xs,       row_idx,  row ) ;

        // we create aperture - so that the regression
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

  ///////////////////////////////////////////////////////
  // calc_cal( app, y, xs, aperture );

  assert(m_rows(y) == m_rows(xs));
  assert(m_rows(y) == m_rows(aperture));

  R regression;

  m_regression( xs, y, &regression );   // rename reg_regression()...
  usart1_flush();

  // TODO change name regression_report.
  // or _output..
  r_regression_show( &regression, stdout);
   usart1_flush();



  {
    // print some stats
    uint32_t aperture_ = nplc_to_aper_n( 10, app->lfreq );

    double sigma_div_aperture = regression.sigma / aperture_  * 1e6; // 1000000;  // in uV.

    printf("stderr(V) %.2fuV  (nplc10)\n", sigma_div_aperture);

    double last_b_coefficient = m_get_val(regression.b ,   0,   m_rows(regression.b) - 1);

    // todo move code from funcion there. i think.
    show_slope_b_detail( aperture_, last_b_coefficient);
  }


    // m_copy does not change dims.

  app->b = m_resize(app->b,  m_rows( regression.b ), m_cols( regression.b));

  // assert(app->b.m == regression->b.m && app->b.n == regression->b.n);
  assert(app->b->m == regression.b->m && app->b->n == regression.b->n);

  // note the predicted values are in the regression structure.
  app->b       = m_copy( regression.b, app->b );

  // free regression
  r_free( &regression );

  //////////////////////////




  m_free(row);

  m_free(xs);
  m_free(y);
  m_free(aperture);

  // Should probably do a reset to initial again.
  // but we need more control over the input muxing.

  // reset the nplc
  mode->reg_adc_p_aperture = nplc_to_aper_n( 10, app->lfreq );    // dynamic. problem. maybe 50,60Hz. or other.
  spi_ice40_reg_write_n(app->spi, REG_ADC_P_APERTURE, &mode->reg_adc_p_aperture, sizeof( mode->reg_adc_p_aperture) );

  // leave running as is
  // turn off. -
  spi_ice40_reg_write32(app->spi, REG_SA_ARM_TRIGGER, 0 );      // arm to halt.


  printf("finished loop3\n");

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
