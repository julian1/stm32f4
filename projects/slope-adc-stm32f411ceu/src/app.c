

#include <stdbool.h>


#include "assert.h"
#include "ice40.h"  // spi_ice40_reg_read
#include "format.h" // format_bits
#include "usart.h"   // usart_flus()
#include "util.h"   // system_millis



#include "regression.h"



#include <matrix.h>

// #include <libopencm3/stm32/spi.h>   // spi .. TODO remove. should pass spi by uint32_t argument

#include "app.h"

// #include "voltage-source-1/voltage-source.h"





uint32_t nplc_to_aper_n( double nplc )
{
  double period = nplc / 50.0 ;  // seonds
  uint32_t aper = period * 20000000;
  return aper;
}


double aper_n_to_nplc( uint32_t aper_n)
{
  // uint32_t aper  = params->clk_count_aper_n ;
  double period   = aper_n / (double ) 20000000;
  double nplc     = period / (1.0 / 50);
  return nplc;
}


double aper_n_to_period( uint32_t aper_n)
{
  double period   = aper_n / (double ) 20000000;
  return period;
}


/*
void ctrl_set_pattern( uint32_t spi, uint32_t pattern )
{
  // TODO add spi argument.
  printf("set pattern %ld\n", pattern );

  spi_ice40_reg_write(spi, REG_PATTERN,   pattern );
}

*/

uint32_t ctrl_get_state( uint32_t spi )
{

  return  spi_ice40_reg_read(spi, REG_STATE);

}







void ctrl_set_aperture( uint32_t spi, uint32_t aperture)
{
  /*
  printf("*set aperture %lu, nplc %.2f, period %.2fs\n",
    aperture,
    aper_n_to_nplc( aperture),
    aper_n_to_period( aperture )
  );
  */

  spi_ice40_reg_write(spi, REG_CLK_COUNT_APER_N_HI, (aperture >> 24) & 0xff );
  spi_ice40_reg_write(spi, REG_CLK_COUNT_APER_N_LO, aperture & 0xffffff  );
}


uint32_t ctrl_get_aperture( uint32_t spi )
{

  uint32_t int_lo = spi_ice40_reg_read(spi, REG_CLK_COUNT_APER_N_LO );
  uint32_t int_hi = spi_ice40_reg_read(spi, REG_CLK_COUNT_APER_N_HI );

  uint32_t val = int_hi << 24 | int_lo;
  return val;

}




char * himux_sel_format( uint32_t mux )
{

  /*
    printf("himux_sel %s (%lu), ",    format_bits( buf, 8, run->himux_sel), run->himux_sel);
  */

  //
  switch(mux) {
    case HIMUX_SEL_SIG_HI:  return "sig   " ;
    case HIMUX_SEL_REF_HI:  return "ref-hi" ;
    case HIMUX_SEL_REF_LO:  return "ref-lo";
    case HIMUX_SEL_ANG:     return "ref-lo";

    case 0 :                return "0";  // uniinitialized

    default:
      assert(0);
      return NULL;
  };
}


void ctrl_set_mux( uint32_t spi, uint32_t mux )
{
  /*
  // char buf[100];
  // printf("*set himux_sel %s (%lu)\n",  format_bits( buf, 4, mux ), mux);
  */
  spi_ice40_reg_write(spi, REG_HIMUX_SEL,  mux);
}


uint32_t ctrl_get_mux( uint32_t spi )
{
  return  spi_ice40_reg_read(spi, REG_HIMUX_SEL);
}




void ctrl_set_var_n( uint32_t spi, uint32_t val)
{
  spi_ice40_reg_write(spi, REG_CLK_COUNT_VAR_N,  val);
}




uint32_t ctrl_get_var_n( uint32_t spi )
{
  return spi_ice40_reg_read(spi, REG_CLK_COUNT_VAR_N);
}


void ctrl_set_fix_n( uint32_t spi, uint32_t val)
{
  spi_ice40_reg_write(spi, REG_CLK_COUNT_FIX_N,  val);
}


uint32_t ctrl_get_fix_n( uint32_t spi )
{
  return spi_ice40_reg_read(spi, REG_CLK_COUNT_FIX_N);
}






void ctrl_reset_enable( uint32_t spi )
{
  // TODO pass spi.
  // active low
  spi_ice40_reg_write(spi, REG_RESET,  0);
}

void ctrl_reset_disable(uint32_t spi)
{
  spi_ice40_reg_write(spi, REG_RESET,  1);
}







void ctrl_run_read( uint32_t spi, Run *run, bool verbose )
{
  /*
  pass spi argument.
  change name adc_meas_read()   or intg_meas_read()
    etc.
  */
  assert(run);

if(verbose) {
    // use separate lines (to make it easier to filter - for plugging into stats).
    run->count_var_up       = spi_ice40_reg_read(spi, REG_COUNT_UP );
    run->count_var_down     = spi_ice40_reg_read(spi, REG_COUNT_DOWN );

    run->count_fix_up       = spi_ice40_reg_read(spi, REG_COUNT_FIX_UP);
    run->count_fix_down     = spi_ice40_reg_read(spi, REG_COUNT_FIX_DOWN);

    run->count_flip         = spi_ice40_reg_read(spi, REG_COUNT_FLIP);

    run->count_pos_trans    = spi_ice40_reg_read(spi, REG_COUNT_TRANS_UP );
    run->count_neg_trans    = spi_ice40_reg_read(spi, REG_COUNT_TRANS_DOWN );

    // WE could record slow_rundown separate to normal rundown.
    // same as  clk_count_mux_rd
    // TODO REMOVE
    // run->clk_count_rundown  = spi_ice40_reg_read(spi, REG_CLK_COUNT_RUNDOWN );

    run->count_flip         = spi_ice40_reg_read(spi, REG_COUNT_FLIP);
  }

  run->himux_sel = spi_ice40_reg_read(spi, REG_HIMUX_SEL );


  run->clk_count_mux_neg  = spi_ice40_reg_read(spi, REG_CLK_COUNT_MUX_NEG);
  run->clk_count_mux_pos  = spi_ice40_reg_read(spi, REG_CLK_COUNT_MUX_POS);
  run->clk_count_mux_rd   = spi_ice40_reg_read(spi, REG_CLK_COUNT_MUX_RD);

  // may be being, returned by the pattern controller

}




void run_show( const Run *run, bool verbose )
{
  assert(run);

  if(verbose) {
    // char buf[100];
    // printf("himux_sel %s (%lu), ",    format_bits( buf, 8, run->himux_sel), run->himux_sel);

    printf("var_up/down %lu %lu, ",   run->count_var_up, run->count_var_down );

    printf("fix_up/down %lu %lu, ",   run->count_fix_up,  run->count_fix_down);

    printf("pos_trans_up/down %lu %lu, ", run->count_pos_trans,  run->count_neg_trans);


    printf("count_flip %lu, ",        run->count_flip);
    // printf("clk_count_rundown %lu, ", run->clk_count_rundown);

    printf("clk_count_mux_neg %lu, ", run->clk_count_mux_neg);
    printf("clk_count_mux_pos %lu, ", run->clk_count_mux_pos);
    printf("clk_count_mux_rd %lu, ",  run->clk_count_mux_rd);
  }

  else {
    printf("himux_sel %s, ",          himux_sel_format( run->himux_sel));

    printf("clk_count_mux_neg %lu, ", run->clk_count_mux_neg);
    printf("clk_count_mux_pos %lu, ", run->clk_count_mux_pos);
    printf("clk_count_mux_rd %lu, ",  run->clk_count_mux_rd);

  }
  // printf("meas_count %lu, ", run->meas_count);
}




/*
  having separate param, run offers maximum flexibility in design.
    - can read current params off device.
    - can avoid reading of device, and just update local params from last value known to be set by mcu,
    - can read last used params off of device. (eg. if device is source of permuted variables).
*/

void ctrl_param_read( uint32_t spi, Param *param)
{
  uint32_t int_lo = spi_ice40_reg_read(spi, REG_CLK_COUNT_APER_N_LO );
  uint32_t int_hi = spi_ice40_reg_read(spi, REG_CLK_COUNT_APER_N_HI );
  param->clk_count_aper_n = int_hi << 24 | int_lo;

  param->clk_count_fix_n  = spi_ice40_reg_read(spi, REG_CLK_COUNT_FIX_N);

  param->clk_count_var_n = spi_ice40_reg_read(spi, REG_CLK_COUNT_VAR_N);

  // Why do we do this? makes it easier to report.
  // but should be in run
  // param->himux_sel = spi_ice40_reg_read(spi, REG_HIMUX_SEL );

}



void ctrl_param_write( uint32_t spi, Param *param)
{
  ctrl_set_aperture( spi, param->clk_count_aper_n);
  ctrl_set_var_n( spi,    param->clk_count_var_n);
  ctrl_set_fix_n( spi,    param->clk_count_fix_n);
}


bool param_equal( Param *param_a , Param *param_b)
{
  return
     param_a->clk_count_aper_n   == param_b->clk_count_aper_n
     && param_a->clk_count_var_n == param_b->clk_count_var_n
     && param_a->clk_count_fix_n == param_b->clk_count_fix_n;
}


void param_show( const Param *param)
{
  printf("clk_count_aper_n %lu, ", param->clk_count_aper_n);
  printf("clk_count_fix_n %lu, ", param->clk_count_fix_n);
  printf("clk_count_var_n %lu, ", param->clk_count_var_n);

  // char buf[100];
  // printf("himux_sel %s (%lu), ", format_bits( buf, 8, param->himux_sel), param->himux_sel);
}


/*
  - passing argument is problematic.
  - use m_cols(b) for loop1 use.

  - when used for calibration. pass the argument.

*/


MAT * run_to_matrix( const Run *run, unsigned model, MAT * out )
{
  assert(run);


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
    double x0_ = run->clk_count_mux_neg + run->clk_count_mux_rd;
    double x1_ = run->clk_count_mux_pos + run->clk_count_mux_rd;

    out = m_resize(out, 1, 2);
    m_set_val( out, 0, 0,  x0_ );
    m_set_val( out, 0, 1,  x1_  );
  }

  else if( model == 3) {

    out = m_resize(out, 1, 3);
    m_set_val( out, 0, 0,  run->clk_count_mux_neg );
    m_set_val( out, 0, 1,  run->clk_count_mux_pos );
    m_set_val( out, 0, 2,  run->clk_count_mux_rd );
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
    m_set_val( out, 0, 1,  run->clk_count_mux_neg );
    m_set_val( out, 0, 2,  run->clk_count_mux_pos );
    m_set_val( out, 0, 3,  run->clk_count_mux_rd);
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







////////////////




MAT * m_calc_predicted( const MAT *b, const MAT *x, const MAT *aperture)
{
  /*
    do matrix multiply, and adjust by the aperture.
  */

  // don't free input arguments
  // b is 4x1, x is nx4

  assert( m_cols(x) == m_rows( b) );
  assert( m_rows(x) == m_rows( aperture) );

  // matrix multiply
  MAT *predicted = m_mlt(x, b, MNULL );

  MAT	*corrected = m_element_div( predicted, aperture, MNULL );

  M_FREE(predicted );

/*
  printf("corrected\n");
  m_foutput(stdout, corrected);
  usart1_flush();
*/

  return corrected;

}






void app_update_led(app_t *app)
{
  assert(app);

  // 500ms soft timer. should handle wrap around
  if( (system_millis - app->led_tick_count) > app->led_blink_interval ) {
    app->led_tick_count += app->led_blink_interval;
    led_toggle();
  }
}




void app_update( app_t * app )
{
  app_update_console_cmd(app);
  app_update_led(app);
}



//////














void app_simple_sleep( app_t * app, uint32_t period )
{
  // not static
  uint32_t soft_timer = system_millis;

  uint32_t _1s_timer = system_millis;

  while(true) {
    // keep pumping messages
    app_update( app );

    if( (system_millis - soft_timer ) > period ) {

      printf("\n");
      return;
    }

    // print dots
    if( (system_millis - _1s_timer) > 1000) {
      printf(".");
      _1s_timer += 1000;
    }
  }

}




void app_loop_main(app_t *app)
{
  // change name app_loop_main.

  printf("=========\n");
  printf("app loop main\n");

  while(true) {

    app->halt_func = false;

    app_update(app);

/*
    if(app->continuation_f) {
      printf("jump to continuation\n");
      void (*tmppf)(void *) = app->continuation_f;
      app->continuation_f = NULL;
      tmppf( app->continuation_ctx );

      printf("continuation done\n");
      printf("> ");
    }
*/
  }
}





void app_spi1_default_interupt(app_t *app )
{
  //  about the msot important function
  // app->data_ready = true;
  /*
    could blink the led.
  */


}






  /*



    0.  very useful feature - how biased resistor ladder - means that modulation will cycle around to a stop point
          where 4 phase modulation ends up above  the cross, ready for rundown.
          and fix pos and fix neg are equal.
          - and it will do this in a finite amount of time.

        3458a though. can do rundown from either side of the zero-cross. using a slow slope resistor. that's why doesn't need bias.


      EXTR. IMPORTANT.
    1. must calculate the estimator values before average rather than average raw inputs (rundown count etc) then cal estimated.

      - because the modulation could flutter around the hi count values. so that an average
      does not accurately capture the combination with slow rundown count.
      ------------

      EXTR IMPORTANT
      *******
    2. rather than represent the slow rundown as an independent field .
      it could be represented - as a clk count of *both* pos and neg.
      likewise the fast rundown

      eg. total current =
      (fix pos + var pos + rundown pos) + (fix neg + var neg  + rundown neg )
      = c + a * pos + b * neg

      rather than 3 independent var linear regression, it collapses to a 2 var regression.

      So that the calculation collpases to just a two independent variable linear regression.
      And if fast runddown is used then it is just 0 clk.

    3.
      we may want a constant... / ones. functional specification.

    4.
      - i think there is a valid implicit origin point -  count pos = 0, count neg ==  reflo == 0. should equal 0.
        included in regression sample data.

      - we should compute the predicted origin . with 0 counts. see what it says.
      and check val

    5.
      - think we need to simplify/combine  the cal_loop and main loop.
      - have a generalized loop. that can just add to an empty array.  and then return control after a loop count
    6.
      - with simplified sum model. can do inl on 0V and 7.1V and
          - permutations on integration time.
          - permutations on fixed time. permutations on var time.

          eg. the having the model work and be invariant to signal time/ pos/var - is an incredibly important property.

        - this is a conseuqence of k2002/Kleinstein design -final rundown contribution - is just addition of +ve and -ve weight * clock.
              except we take both.

        - i think if residual adc is used - it has to be incorporated as an extra term in a regression model..
          eg.
              y = b0 + b1 * (var pos + fix pos) + b2 (var neg + fix neg) + b3 * residual;

          versus
              y = b0 + b1 * (var pos + fix pos + rundown) + b2 (var neg + fix neg + rundown);


    7.
      - so we can characterize INL - with permutations - and therefore test different functinoal specifications for calibration.

    8.
      can destermine the resolution.
      by just plugging in values to regression.
      eg.   predict( ...,  rundown) - predict( ..., rundown + 1);
      and from there determine count.
      -------
      think would need values calculated near the input range of the integrator. eg. +-10V.  But could use 7.1V as proxy.

    =======
  */







