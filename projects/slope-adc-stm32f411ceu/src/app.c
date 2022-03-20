



#include "assert.h"
#include "streams.h"  // printf
#include "ice40.h"  // spi_reg_read
#include "format.h" // format_bits
#include "usart.h"   // usart_flus()
#include "util.h"   // system_millis




#include "regression.h"


#include <stdbool.h>

#include <matrix.h>

#include <libopencm3/stm32/spi.h>   // SPI1 .. TODO remove. should pass spi by uint32_t argument

#include "app.h"






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


void ctrl_set_pattern( uint32_t spi, uint32_t pattern )
{
  assert(spi == SPI1);
  // TODO add spi argument.
  printf("set pattern %ld\n", pattern );

  spi_reg_write(SPI1, REG_PATTERN,   pattern );
}


void ctrl_set_aperture( uint32_t spi, uint32_t aperture)
{
  assert(spi == SPI1);
  /*
  printf("*set aperture %lu, nplc %.2f, period %.2fs\n",
    aperture,
    aper_n_to_nplc( aperture),
    aper_n_to_period( aperture )
  );
  */

  spi_reg_write(SPI1, REG_CLK_COUNT_APER_N_HI, (aperture >> 24) & 0xff );
  spi_reg_write(SPI1, REG_CLK_COUNT_APER_N_LO, aperture & 0xffffff  );
}


uint32_t ctrl_get_aperture( uint32_t spi )
{
  assert(spi == SPI1);

  uint32_t int_lo = spi_reg_read(SPI1, REG_CLK_COUNT_APER_N_LO );
  uint32_t int_hi = spi_reg_read(SPI1, REG_CLK_COUNT_APER_N_HI );

  uint32_t val = int_hi << 24 | int_lo;
  return val;

}




static char * himux_sel_format( uint32_t mux )
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
  assert(spi == SPI1);
  /*
  // char buf[100];
  // printf("*set himux_sel %s (%lu)\n",  format_bits( buf, 4, mux ), mux);
  */
  spi_reg_write(SPI1, REG_HIMUX_SEL,  mux);
}


uint32_t ctrl_get_mux( uint32_t spi )
{
  assert(spi == SPI1);
  return  spi_reg_read(SPI1, REG_HIMUX_SEL);
}




void ctrl_reset_enable( uint32_t spi )
{
  assert(spi == SPI1);
  // TODO pass spi.
  // active low
  spi_reg_write(SPI1, REG_RESET,  0);
}

void ctrl_reset_disable(uint32_t spi)
{
  assert(spi == SPI1);
  spi_reg_write(SPI1, REG_RESET,  1);
}







void ctrl_run_read( uint32_t spi, Run *run )
{
  /*
  pass spi argument.
  change name adc_meas_read()   or intg_meas_read()
    etc.
  */
  assert(spi == SPI1);
  assert(run);

  // use separate lines (to make it easier to filter - for plugging into stats).
  run->count_up         = spi_reg_read(SPI1, REG_COUNT_UP );
  run->count_down       = spi_reg_read(SPI1, REG_COUNT_DOWN );

  run->count_trans_up   = spi_reg_read(SPI1, REG_COUNT_TRANS_UP );
  run->count_trans_down = spi_reg_read(SPI1, REG_COUNT_TRANS_DOWN );

  run->count_fix_up     = spi_reg_read(SPI1, REG_COUNT_FIX_UP);
  run->count_fix_down   = spi_reg_read(SPI1, REG_COUNT_FIX_DOWN);


  // WE could record slow_rundown separate to normal rundown.
  run->clk_count_rundown = spi_reg_read(SPI1, REG_CLK_COUNT_RUNDOWN );



}




void run_report( const Run *run )
{
  assert(run);

  // printf("count_up %u, ",         run->count_up );
  // printf("count_down %u, ",       run->count_down );

  printf("count_up/down %u %u, ", run->count_up, run->count_down );
  printf("trans_up/down %u %u, ", run->count_trans_up,  run->count_trans_down);
  printf("fix_up/down %u %u, ",   run->count_fix_up,  run->count_fix_down);
  // printf("count_flip %u, ",       run->count_flip);

  printf("clk_count_rundown %u, ", run->clk_count_rundown);

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
  assert(spi == SPI1);

  uint32_t int_lo = spi_reg_read(SPI1, REG_CLK_COUNT_APER_N_LO );
  uint32_t int_hi = spi_reg_read(SPI1, REG_CLK_COUNT_APER_N_HI );
  param->clk_count_aper_n = int_hi << 24 | int_lo;

  param->clk_count_fix_n  = spi_reg_read(SPI1, REG_CLK_COUNT_FIX_N);

  param->clk_count_var_pos_n = spi_reg_read(SPI1, REG_CLK_COUNT_VAR_POS_N);

  param->himux_sel = spi_reg_read(SPI1, REG_HIMUX_SEL );

}





void ctrl_param_read_last( uint32_t spi, Param *param)
{
  assert(spi == SPI1);

  // but nothing permutes this.
  uint32_t int_lo = spi_reg_read(SPI1, REG_LAST_CLK_COUNT_APER_N_LO );
  uint32_t int_hi = spi_reg_read(SPI1, REG_LAST_CLK_COUNT_APER_N_HI );
  param->clk_count_aper_n = int_hi << 24 | int_lo;

  param->clk_count_fix_n  = spi_reg_read(SPI1, REG_LAST_CLK_COUNT_FIX_N);

  param->clk_count_var_pos_n = spi_reg_read(SPI1, REG_LAST_CLK_COUNT_VAR_POS_N);

  // This is.
  param->himux_sel = spi_reg_read(SPI1, REG_LAST_HIMUX_SEL ); // **last
}


void param_report( const Param *param)
{
  printf("clk_count_aper_n %lu, ", param->clk_count_aper_n);
  printf("clk_count_fix_n %lu, ", param->clk_count_fix_n);
  printf("clk_count_var_pos_n %lu, ", param->clk_count_var_pos_n);

  char buf[100];
  printf("himux_sel %s (%lu), ", format_bits( buf, 8, param->himux_sel), param->himux_sel);
}




MAT * param_run_to_matrix( const Param *param, const Run *run, MAT * out )
{
  assert(run);

/*
  EXTR.
    we want to read the var_pos_n etc. after *each* run.
    BECAUSE. we want to allow the pattern controller to permute
*/


  if(out == MNULL)
    out = m_get(1,1); // TODO fix me. this is ok.


  // negative current / slope up
  double x0 = (run->count_up   * param->clk_count_var_pos_n) + (run->count_fix_up   * param->clk_count_fix_n) ;

  // positive current. slope down.
  double x1 = (run->count_down * param->clk_count_var_pos_n) + (run->count_fix_down * param->clk_count_fix_n) ;

  double x2 = run->clk_count_rundown;

  // four variable model
  uint32_t cols = 4; 

  m_resize(out, 1, cols );


  if( m_cols( out) == 3) {    // X_COLS == 3

    m_set_val( out, 0, 0,  x0 );
    m_set_val( out, 0, 1,  x1  );
    m_set_val( out, 0, 2,  x2  );
  } else if ( m_cols(out) == 4) { // X_COLS == 4

    m_set_val( out, 0, 0,  1.f ); // ones, offset
    m_set_val( out, 0, 1,  x0 );
    m_set_val( out, 0, 2,  x1  );
    m_set_val( out, 0, 3,  x2  );
  } else assert( 0);

  return out;
}






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







