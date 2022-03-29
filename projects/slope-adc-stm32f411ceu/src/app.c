

#include <stdbool.h>


#include "assert.h"
#include "ice40.h"  // spi_reg_read
#include "format.h" // format_bits
#include "usart.h"   // usart_flus()
#include "util.h"   // system_millis



#include "regression.h"



#include <matrix.h>

// #include <libopencm3/stm32/spi.h>   // spi .. TODO remove. should pass spi by uint32_t argument

#include "app.h"

#include "voltage-source.h"





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
  assert(spi == spi);
  // TODO add spi argument.
  printf("set pattern %ld\n", pattern );

  spi_reg_write(spi, REG_PATTERN,   pattern );
}

*/

uint32_t ctrl_get_state( uint32_t spi )
{

  return  spi_reg_read(spi, REG_STATE);

}







void ctrl_set_aperture( uint32_t spi, uint32_t aperture)
{
  assert(spi == spi);
  /*
  printf("*set aperture %lu, nplc %.2f, period %.2fs\n",
    aperture,
    aper_n_to_nplc( aperture),
    aper_n_to_period( aperture )
  );
  */

  spi_reg_write(spi, REG_CLK_COUNT_APER_N_HI, (aperture >> 24) & 0xff );
  spi_reg_write(spi, REG_CLK_COUNT_APER_N_LO, aperture & 0xffffff  );
}


uint32_t ctrl_get_aperture( uint32_t spi )
{
  assert(spi == spi);

  uint32_t int_lo = spi_reg_read(spi, REG_CLK_COUNT_APER_N_LO );
  uint32_t int_hi = spi_reg_read(spi, REG_CLK_COUNT_APER_N_HI );

  uint32_t val = int_hi << 24 | int_lo;
  return val;

}




char * himux_sel_format( uint32_t mux )
{
  //
  switch(mux) {
    case HIMUX_SEL_SIG_HI:  return "sig" ;
    case HIMUX_SEL_REF_HI:  return "ref-hi" ;
    case HIMUX_SEL_REF_LO:  return "ref-lo";
    case HIMUX_SEL_ANG:     return "ref-lo";

    default:
      assert(0);
      return NULL;
  };
}


void ctrl_set_mux( uint32_t spi, uint32_t mux )
{
  assert(spi == spi);
  /*
  // char buf[100];
  // printf("*set himux_sel %s (%lu)\n",  format_bits( buf, 4, mux ), mux);
  */
  spi_reg_write(spi, REG_HIMUX_SEL,  mux);
}


uint32_t ctrl_get_mux( uint32_t spi )
{
  assert(spi == spi);
  return  spi_reg_read(spi, REG_HIMUX_SEL);
}




void ctrl_set_var_n( uint32_t spi, uint32_t val)
{
  spi_reg_write(spi, REG_CLK_COUNT_VAR_N,  val);
}




uint32_t ctrl_get_var_n( uint32_t spi )
{
  return spi_reg_read(spi, REG_CLK_COUNT_VAR_N);
}


void ctrl_set_fix_n( uint32_t spi, uint32_t val)
{
  spi_reg_write(spi, REG_CLK_COUNT_FIX_N,  val);
}


uint32_t ctrl_get_fix_n( uint32_t spi )
{
  return spi_reg_read(spi, REG_CLK_COUNT_FIX_N);
}






void ctrl_reset_enable( uint32_t spi )
{
  assert(spi == spi);
  // TODO pass spi.
  // active low
  spi_reg_write(spi, REG_RESET,  0);
}

void ctrl_reset_disable(uint32_t spi)
{
  assert(spi == spi);
  spi_reg_write(spi, REG_RESET,  1);
}







void ctrl_run_read( uint32_t spi, Run *run )
{
  /*
  pass spi argument.
  change name adc_meas_read()   or intg_meas_read()
    etc.
  */
  assert(spi == spi);
  assert(run);

  // use separate lines (to make it easier to filter - for plugging into stats).
  run->count_up         = spi_reg_read(spi, REG_COUNT_UP );
  run->count_down       = spi_reg_read(spi, REG_COUNT_DOWN );

  run->count_trans_up   = spi_reg_read(spi, REG_COUNT_TRANS_UP );
  run->count_trans_down = spi_reg_read(spi, REG_COUNT_TRANS_DOWN );

  run->count_fix_up     = spi_reg_read(spi, REG_COUNT_FIX_UP);
  run->count_fix_down   = spi_reg_read(spi, REG_COUNT_FIX_DOWN);


  // WE could record slow_rundown separate to normal rundown.
  run->clk_count_rundown = spi_reg_read(spi, REG_CLK_COUNT_RUNDOWN );
}




void run_report( const Run *run )
{
  assert(run);

  printf("count_up/down %lu %lu, ", run->count_up, run->count_down );
  printf("trans_up/down %lu %lu, ", run->count_trans_up,  run->count_trans_down);
  printf("fix_up/down %lu %lu, ",   run->count_fix_up,  run->count_fix_down);
  // printf("count_flip %u, ",       run->count_flip);

  printf("clk_count_rundown %lu, ", run->clk_count_rundown);

  // printf("meas_count %lu, ", run->meas_count);
}


void run_report_brief( const Run *run )
{
  assert(run);
  printf("count_up/down %lu %lu, ", run->count_up, run->count_down );
  printf("fix_up/down %lu %lu, ",   run->count_fix_up,  run->count_fix_down);
  printf("clk_count_rundown %lu, ", run->clk_count_rundown);
 
}

/*
  having separate param, run offers maximum flexibility in design.
    - can read current params off device.
    - can avoid reading of device, and just update local params from last value known to be set by mcu,
    - can read last used params off of device. (eg. if device is source of permuted variables).
*/

void ctrl_param_read( uint32_t spi, Param *param)
{
  assert(spi == spi);

  uint32_t int_lo = spi_reg_read(spi, REG_CLK_COUNT_APER_N_LO );
  uint32_t int_hi = spi_reg_read(spi, REG_CLK_COUNT_APER_N_HI );
  param->clk_count_aper_n = int_hi << 24 | int_lo;

  param->clk_count_fix_n  = spi_reg_read(spi, REG_CLK_COUNT_FIX_N);

  param->clk_count_var_n = spi_reg_read(spi, REG_CLK_COUNT_VAR_N);

  // Why do we do this? makes it easier to report.
  param->himux_sel = spi_reg_read(spi, REG_HIMUX_SEL );

}




#if 0
void ctrl_param_read_last( uint32_t spi, Param *param)
{
  assert(spi == spi);

  // but nothing permutes this.
  uint32_t int_lo = spi_reg_read(spi, REG_LAST_CLK_COUNT_APER_N_LO );
  uint32_t int_hi = spi_reg_read(spi, REG_LAST_CLK_COUNT_APER_N_HI );
  param->clk_count_aper_n = int_hi << 24 | int_lo;

  param->clk_count_fix_n  = spi_reg_read(spi, REG_LAST_CLK_COUNT_FIX_N);

  param->clk_count_var_n = spi_reg_read(spi, REG_LAST_CLK_COUNT_VAR_N);

  // This is.
  param->himux_sel = spi_reg_read(spi, REG_LAST_HIMUX_SEL ); // **last
}
#endif


void param_report( const Param *param)
{
  printf("clk_count_aper_n %lu, ", param->clk_count_aper_n);
  printf("clk_count_fix_n %lu, ", param->clk_count_fix_n);
  printf("clk_count_var_n %lu, ", param->clk_count_var_n);

  char buf[100];
  printf("himux_sel %s (%lu), ", format_bits( buf, 8, param->himux_sel), param->himux_sel);
}


/*
  - passing argument is problematic.
  - use m_cols(b) for loop1 use.

  - when used for calibration. pass the argument.

*/


MAT * param_run_to_matrix( const Param *param, const Run *run, unsigned model, MAT * out )
{
  assert(run);

/*
  - we don't seem to be able to modify the count limits - without causing 1mV/20V difference. eg. 100ppm .  
  - could indicate bad INL issues?
  - or somehting else going on. related to geometry of slope and rundown. 
  - but we may not need to even multiply this. instead just plug the raw count_up/count_down to the regression .
  - because the length of the variable - is different regardless if it is up or down. and they are not equal?

  ----
  EXTR.
    we want to read the var_n etc. after *each* run.
    BECAUSE. we want to allow the pattern controller to permute

  - on scope. cannot see many little reverse periods. when integrating 0V. ok. o 

    we see the down at +ref.
*/


  if(out == MNULL)
    out = m_get(1,1); // TODO fix me. this is ok.


  // negative current / slope up
  double x0 = (run->count_up   * param->clk_count_var_n) + (run->count_fix_up   * param->clk_count_fix_n) ;

  // positive current. slope down.
  double x1 = (run->count_down * param->clk_count_var_n) + (run->count_fix_down * param->clk_count_fix_n) ;

  double x2 = run->clk_count_rundown;

  // four variable model
  // uint32_t cols = 4;

  m_resize(out, 1, model );


  // model == 2, no rundown.

  if( model == 3) {    // X_COLS == 3

    m_set_val( out, 0, 0,  x0 );
    m_set_val( out, 0, 1,  x1  );
    m_set_val( out, 0, 2,  x2  );
  }
  else if ( model == 4) { // X_COLS == 4

    m_set_val( out, 0, 0,  1.f ); // ones, offset
    m_set_val( out, 0, 1,  x0 );
    m_set_val( out, 0, 2,  x1  );
    m_set_val( out, 0, 3,  x2  );
  } 
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
  if( (system_millis - app->soft_500ms) > 500) {
    app->soft_500ms += 500;
    led_toggle();
  }

}


void app_update( app_t * app )
{
  app_update_console_cmd(app);
  app_update_led(app);
}





void app_simple_sleep( app_t * app, uint32_t period )
{
  // not static
  uint32_t soft_timer = system_millis;

  uint32_t _1s_timer = system_millis;

  while(true) {
    // keep pumping messages
    app_update_console_cmd(app);
    app_update_led(app);

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





void app_loop_dispatcher(app_t *app)
{
  printf("=========\n");
  printf("continuation dispatcher\n");
  printf("> ");

  while(true) {

    app_update_console_cmd(app);
    app_update_led( app);


    if(app->continuation_f) {
      printf("jump to continuation\n");
      void (*tmppf)(void *) = app->continuation_f;
      app->continuation_f = NULL;
      tmppf( app->continuation_ctx );

      printf("continuation done\n");
      printf("> ");
    }
  }

}






void app_spi1_interupt(app_t *app )
{
  UNUSED(app);
  app->data_ready = true;
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







