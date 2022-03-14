



#include "assert.h"
#include "streams.h"  // usart_printf
#include "ice40.h"  // spi_reg_read
#include "format.h" // format_bits
#include "usart.h"   // usart_flus()
#include "util.h"   // system_millis




#include "regression.h"


#include <stdbool.h>

#include <matrix.h>

#include <libopencm3/stm32/spi.h>   // SPI1 .. TODO remove. should pass spi by uint32_t argument

#include "app.h"





/*
  different nplc - slightly different offsets
  --

  uint32_t int_n  = params->clk_count_aper_n ;
  double period   = int_n / (double ) 20000000;
  double nplc     = period / (1.0 / 50);

  // nplc_to_aper_n ()
  // aper_n_to_nplc()
*/

uint32_t nplc_to_aper_n( double nplc )
{
  double period = nplc / 50.0 ;  // seonds
  uint32_t aper = period * 20000000;
  return aper;
}


double aper_n_to_nplc( uint32_t aper)
{
  // uint32_t aper  = params->clk_count_aper_n ;
  double period   = aper / (double ) 20000000;
  double nplc     = period / (1.0 / 50);
  return nplc;
}


double aper_n_to_period( uint32_t aper)
{
  double period   = aper / (double ) 20000000;
  return period;
}

/*
void ctrl_set_pattern( uint32_t pattern )
{
  // TODO add spi argument.
  printf("set pattern %ld\n", pattern );

  spi_reg_write(SPI1, REG_PATTERN,   pattern );
}
*/

void ctrl_set_aperture( uint32_t aperture)
{
  printf("*set aperture %lu, nplc %.2f, period %.2fs\n", aperture, aper_n_to_nplc( aperture),  aper_n_to_period( aperture ));

  spi_reg_write(SPI1, REG_CLK_COUNT_APER_N_HI, (aperture >> 24) & 0xff );
  spi_reg_write(SPI1, REG_CLK_COUNT_APER_N_LO, aperture & 0xffffff  );
}

// ctrl_get_mux
// useful.  when pass control - between loops.

void ctrl_set_mux( uint32_t mux )
{

  char buf[100];
  printf("*set himux_sel %s (%lu)\n",  format_bits( buf, 4, mux ), mux);

  switch(mux) {
    case HIMUX_SEL_SIG_HI:
    case HIMUX_SEL_REF_HI:
    case HIMUX_SEL_REF_LO:
    case HIMUX_SEL_ANG:
      break;
    default:
      assert(0);
  };

  spi_reg_write(SPI1, REG_HIMUX_SEL,  mux);
}


uint32_t ctrl_get_mux( /* uint32_t spi */)
{
  return  spi_reg_read(SPI1, REG_HIMUX_SEL);
}




void ctrl_reset_enable( void )
{
  // TODO pass spi.
  // active low
  spi_reg_write(SPI1, REG_RESET,  0);
}

void ctrl_reset_disable(void)
{

  spi_reg_write(SPI1, REG_RESET,  1);
}







void run_read( Run *run )
{
  /*
  change name adc_meas_read()   or intg_meas_read()
    etc.

  */

  assert(run);
  /*
    it looks like many variables to read over spi.
    but it only takes 10% of the reset time.
    ---
    reading each time. allows fpga to control.

  */

  // use separate lines (to make it easier to filter - for plugging into stats).
  run->count_up         = spi_reg_read(SPI1, REG_COUNT_UP );
  run->count_down       = spi_reg_read(SPI1, REG_COUNT_DOWN );

/*
  // something really weird - with wrong values. happens
  // doesn't seem to happen with nplc=10, or nplc=25, happens once nplc=40

  uint32_t count_up         = spi_reg_read(SPI1, REG_COUNT_UP );
  uint32_t count_down       = spi_reg_read(SPI1, REG_COUNT_DOWN );

  assert(run->count_up == count_up );
  assert(run->count_down == count_down );
*/

  // run->count_trans_up     = spi_reg_read(SPI1, REG_COUNT_TRANS_UP );
  // run->count_trans_down   = spi_reg_read(SPI1, REG_COUNT_TRANS_DOWN );

  run->count_fix_up     = spi_reg_read(SPI1, REG_COUNT_FIX_UP);
  run->count_fix_down   = spi_reg_read(SPI1, REG_COUNT_FIX_DOWN);


  // WE could record slow_rundown separate to normal rundown.
  run->clk_count_rundown = spi_reg_read(SPI1, REG_CLK_COUNT_RUNDOWN );

  ///////////////
  // from params
  // eg. defer to fpga which is source/ and may modulate these

  // run->meas_count       = spi_reg_read(SPI1, REG_MEAS_COUNT );

  uint32_t int_lo = spi_reg_read(SPI1, REG_CLK_COUNT_APER_N_LO );
  uint32_t int_hi = spi_reg_read(SPI1, REG_CLK_COUNT_APER_N_HI );
  run->clk_count_aper_n = int_hi << 24 | int_lo;

  run->clk_count_fix_n  = spi_reg_read(SPI1, REG_CLK_COUNT_FIX_N);

  run->clk_count_var_pos_n = spi_reg_read(SPI1, REG_CLK_COUNT_VAR_POS_N);

  // do this for the moment. albeit it shouldn't be needed
  // when wrapping parameter changes with reset.
  // run->himux_sel = spi_reg_read(SPI1, REG_HIMUX_SEL );


}




void run_report( Run *run, bool extra )
{
  assert(run);

  // usart_printf("count_up %u, ",         run->count_up );
  // usart_printf("count_down %u, ",       run->count_down );

  usart_printf("count_up/down %u %u, ", run->count_up, run->count_down );
  // usart_printf("trans_up/down %u %u, ", run->count_trans_up,  run->count_trans_down);
  usart_printf("fix_up/down %u %u, ",   run->count_fix_up,  run->count_fix_down);
  // usart_printf("count_flip %u, ",       run->count_flip);

  usart_printf("clk_count_rundown %u, ", run->clk_count_rundown);

  if(extra) {
    usart_printf("meas_count %lu, ", run->meas_count);

    usart_printf("clk_count_aper_n %lu, ", run->clk_count_aper_n);

    usart_printf("clk_count_fix_n %lu, ", run->clk_count_fix_n);
    usart_printf("clk_count_var_pos_n %lu, ", run->clk_count_var_pos_n);

    char buf[100];
    printf("himux_sel %s",  format_bits( buf, 4, run->himux_sel ));

  }

}






MAT * run_to_matrix( /*Params *params,*/ Run *run, MAT * out )
{

/*
  EXTR.
    we want to read the var_pos_n etc. after *each* run.
    BECAUSE. we want to allow the pattern controller to permute
*/

  // return a three variable row vector

  // UNUSED(params);

  if(out == MNULL)
    out = m_get(1,1); // TODO fix me. this is ok.


  // negative current / slope up
  double x0 = (run->count_up   * run->clk_count_var_pos_n) + (run->count_fix_up   * run->clk_count_fix_n) ;

  // positive current. slope down.
  double x1 = (run->count_down * run->clk_count_var_pos_n) + (run->count_fix_down * run->clk_count_fix_n) ;

  double x2 = run->clk_count_rundown;


  // three variable
  m_resize(out, 1, X_COLS);



  if( X_COLS == 3) {
    // m_set_val( out, 0, 0,  1.f ); // ones/
    m_set_val( out, 0, 0,  x0 );
    m_set_val( out, 0, 1,  x1  );
    m_set_val( out, 0, 2,  x2  );
  } else if (X_COLS == 4) {
    m_set_val( out, 0, 0,  1.f ); // ones/
    m_set_val( out, 0, 1,  x0 );
    m_set_val( out, 0, 2,  x1  );
    m_set_val( out, 0, 3,  x2  );
  } else assert( 0);

  return out;
}




#if 0
MAT * run_to_matrix( Params *params, Run *run, MAT * out )
{
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

  UNUSED(params);

  if(out == MNULL)
    out = m_get(1,1); // TODO fix me.

#if 0
  // compute value
  m_resize(out, 1, 3);
  m_set_val( out, 0, 0,  run->count_up );
  m_set_val( out, 0, 1,  run->count_down );
  m_set_val( out, 0, 2,  run->count_fix_up );
#endif

  /*
    compared with an adc. we don't have to treat the residual charge on rundown - as an extra independent variable.
  */

  // slow rundown uses both

  double x0 = 1.0f;
  UNUSED(x0);

  // negative current / slope up
  double x1 = (run->count_up   * params->clk_count_var_neg_n) + (run->count_fix_up   * params->clk_count_fix_n) + run->clk_count_rundown;

  // not sure if we want to do this. may have to calibrate for a period. which would be ugly.
  x1 /= params-> clk_count_aper_n ;

  // positive current. slope down.
  double x2 = (run->count_down * params->clk_count_var_pos_n) + (run->count_fix_down * params->clk_count_fix_n) + run->clk_count_rundown;

  x2 /= params-> clk_count_aper_n ;

#if 1
  // 2 variable model.
  m_resize(out, 1, 2);
  m_set_val( out, 0, 0,  x1  );
  m_set_val( out, 0, 1,  x2  );
#endif

#if 0
  // three variable
  m_resize(out, 1, X_COLS);
  m_set_val( out, 0, 0,  1.f );
  m_set_val( out, 0, 1,  x1  );
  m_set_val( out, 0, 2,  x2  );
#endif


  return out;
}
#endif






#if 0
void params_read( Params * params )
{
  // params->reg_led           = spi_reg_read(SPI1, REG_LED);

  uint32_t int_lo = spi_reg_read(SPI1, REG_CLK_COUNT_APER_N_LO );
  uint32_t int_hi = spi_reg_read(SPI1, REG_CLK_COUNT_APER_N_HI );
  params->clk_count_aper_n   = int_hi << 24 | int_lo;

  params->clk_count_reset_n  = spi_reg_read(SPI1, REG_CLK_COUNT_RESET_N);
  params->clk_count_fix_n   = spi_reg_read(SPI1, REG_CLK_COUNT_FIX_N);
  // params->clk_count_var_n   = spi_reg_read(SPI1, REG_CLK_COUNT_VAR_N);
  params->clk_count_var_pos_n   = spi_reg_read(SPI1, REG_CLK_COUNT_VAR_POS_N);
  params->clk_count_var_neg_n   = spi_reg_read(SPI1, REG_CLK_COUNT_VAR_NEG_N);

  params->use_slow_rundown  = spi_reg_read(SPI1, REG_USE_SLOW_RUNDOWN);
  params->himux_sel         = spi_reg_read(SPI1, REG_HIMUX_SEL);

  params->meas_count       = spi_reg_read(SPI1, REG_MEAS_COUNT );


}



void params_report(Params * params )
{
  char buf[10];

  usart_printf("-------------\n");
  // usart_printf("reg_led           %s\n", format_bits( buf, 4, params->reg_led ) );

  uint32_t int_n  = params->clk_count_aper_n ;
  double period   = aper_n_to_period( int_n);
  double nplc     = aper_n_to_nplc( int_n);
  double samples_per_second = 1.0 / period;


  usart_printf("clk_count_aper_n   %u\n", int_n );
  usart_printf("nplc              %.2f\n", nplc);
  usart_printf("period            %fs\n", period);
  usart_printf("samples/s         %.1f\n", samples_per_second);

  usart_printf("clk_count_reset_n  %u\n", params->clk_count_reset_n);
  usart_printf("clk_count_fix_n   %u\n", params->clk_count_fix_n);

  // usart_printf("clk_count_var_n   %u\n", params->clk_count_var_n);
  usart_printf("clk_count_var_pos_n   %u\n", params->clk_count_var_pos_n);
  usart_printf("clk_count_var_neg_n   %u\n", params->clk_count_var_neg_n);


  // this doesn't look right...
  // double mod_freq = 20000000.f / ( (params->clk_count_var_n  + params->clk_count_var_n) * 2 );
  // usart_printf("nom mod freq      %.0fHz\n", mod_freq );

  usart_printf("use_slow_rundown  %u\n", params->use_slow_rundown);


  //////////
  // char buf[100] char * format_bits(char *buf, size_t width, uint32_t value)
  usart_printf("himux_sel         %s ", format_bits( buf, 4, params->himux_sel));

  switch( params->himux_sel) {
    case HIMUX_SEL_SIG_HI: usart_printf("sig-hi");  break;
    case HIMUX_SEL_REF_HI: usart_printf("ref-hi");  break;
    case HIMUX_SEL_REF_LO: usart_printf("ref-lo");  break;
    case HIMUX_SEL_ANG:    usart_printf("ang");  break;
  }
  usart_printf("\n");


  // uint32_t n = spi_reg_read(SPI1, REG_MEAS_COUNT );
  usart_printf("meas_count        %u\n", params->meas_count );




}




bool params_equal( Params *params0,  Params *params1 )
{

  return
    params0->clk_count_aper_n  ==  params1->clk_count_aper_n
    && params0->use_slow_rundown == params1->use_slow_rundown
    && params0->himux_sel        == params1->himux_sel

    && params0->clk_count_reset_n == params1->clk_count_reset_n
    && params0->clk_count_fix_n  == params1->clk_count_fix_n

    // && params0->clk_count_var_n  == params1->clk_count_var_n
    && params0->clk_count_var_pos_n  == params1->clk_count_var_pos_n
    && params0->clk_count_var_neg_n  == params1->clk_count_var_neg_n
  ;
}



void params_write( Params *params )
{

  usart_printf("write params\n");

  // write the main parameter to device
  spi_reg_write(SPI1, REG_CLK_COUNT_APER_N_HI, (params->clk_count_aper_n >> 24) & 0xff );
  spi_reg_write(SPI1, REG_CLK_COUNT_APER_N_LO, params->clk_count_aper_n & 0xffffff  );
  spi_reg_write(SPI1, REG_USE_SLOW_RUNDOWN, params->use_slow_rundown );
  spi_reg_write(SPI1, REG_HIMUX_SEL, params->himux_sel );

  // write the extra parameters to device
  spi_reg_write(SPI1, REG_CLK_COUNT_RESET_N , params->clk_count_reset_n );
  spi_reg_write(SPI1, REG_CLK_COUNT_FIX_N,   params->clk_count_fix_n );

  // spi_reg_write(SPI1, REG_CLK_COUNT_VAR_N,   params->clk_count_var_n );
  spi_reg_write(SPI1, REG_CLK_COUNT_VAR_POS_N,   params->clk_count_var_pos_n );
  spi_reg_write(SPI1, REG_CLK_COUNT_VAR_NEG_N,   params->clk_count_var_neg_n );

  Params  params2;
  params_read( &params2 );

  // ensure write successful.
  assert(params_equal( params,  &params2 ));
}





void params_set_main( Params *params,  uint32_t clk_count_aper_n, bool use_slow_rundown, uint8_t himux_sel )
{
  params->clk_count_aper_n  = clk_count_aper_n;
  params->use_slow_rundown = use_slow_rundown;
  params->himux_sel        = himux_sel;
}


void params_set_extra( Params *params,  uint32_t clk_count_reset_n, uint32_t  clk_count_fix_n, uint32_t clk_count_var_pos_n, uint32_t clk_count_var_neg_n)
{
  params->clk_count_reset_n = clk_count_reset_n;
  params->clk_count_fix_n  = clk_count_fix_n;

  //
  params->clk_count_var_pos_n  = clk_count_var_pos_n;
  params->clk_count_var_neg_n  = clk_count_var_neg_n;

  // IMPORTNAT.
  // there is a third kind of permutation - altering fix_pos_n and fix_neg_n individually.
}

#endif



