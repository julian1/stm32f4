


#include "assert.h"
#include "streams.h"  // usart_printf
#include "format.h" // format_bits
#include "usart.h"   // usart_flus()
#include "util.h"   // led/system_millis




#include "regression.h"

#include "app.h"


/*
// simpler way to do this.


    configure.

    // block until data.

    // block for data
    while(!app->data_ready /* && !app->halt */ ) {
      // yield();
    }
    app->data_ready = false;
 

  
    read.

  // eg. just configure, block, read.   dead simple.

  in a linear sequence.


*/




static void cal_collect_obs(app_t *app, MAT *xs, MAT *y, MAT *aperture)
{
  // rename cal_generate_modulation_permutions()

  usart_printf("=========\n");
  usart_printf("cal loop\n");


  // presize/oversize
  unsigned  max_rows =  10 * 5;
  m_resize( xs ,        max_rows, X_COLS );
  m_resize( y ,         max_rows, 1 );
  m_resize( aperture,   max_rows, 1 );


  unsigned *himux_sel_last = malloc( sizeof(unsigned) * max_rows );   // TODO need to free . or 
  unsigned himux_sel_last_n = max_rows;



  unsigned row = 0;


  // double aperture_ = 0;


  Param param;

  // I THINK it's wrong to pass the aperture and target.
  // THE row pointer allows this to be set in a higher function

  for(unsigned i = 0; /*i < 10*/; ++i )
  {
    double target;
    // switch integration configuration
    switch(i) {

      // what the hell?

      case 0:
        // 8 NPLC  ref-lo
        ctrl_reset_enable();
        ctrl_set_mux( HIMUX_SEL_REF_LO);
        ctrl_set_aperture( nplc_to_aper_n( 8) );
        ctrl_param_read( &param );
        ctrl_reset_disable();
        target = 0.0  * param.clk_count_aper_n;
        break;

      case 1:
        // 8 NPLC  ref-hi
        ctrl_reset_enable();
        ctrl_set_mux( HIMUX_SEL_REF_HI);
        ctrl_set_aperture( nplc_to_aper_n( 8));
        ctrl_param_read( &param );
        ctrl_reset_disable();
        target = 7.1 * param.clk_count_aper_n;
        break;

      case 2:
        // 9 NPLC  ref-lo
        ctrl_reset_enable();
        ctrl_set_mux( HIMUX_SEL_REF_LO);
        ctrl_set_aperture( nplc_to_aper_n( 9));
        ctrl_param_read( &param );
        ctrl_reset_disable();
        target = 0.0  * param.clk_count_aper_n;
        break;

      case 3:
        // 9 NPLC  ref-hi
        ctrl_reset_enable();
        ctrl_set_mux( HIMUX_SEL_REF_HI);
        ctrl_set_aperture( nplc_to_aper_n( 9));
        ctrl_param_read( &param );
        ctrl_reset_disable();
        target = 7.1 * param.clk_count_aper_n;
        break;

      case 4:
        // 10NPLC  ref-lo
        ctrl_reset_enable();
        ctrl_set_mux( HIMUX_SEL_REF_LO);
        ctrl_set_aperture( nplc_to_aper_n( 10));
        ctrl_param_read( &param );
        ctrl_reset_disable();
        target = 0.0  * param.clk_count_aper_n;
        break;

      case 5:
        // 10NPLC  ref-hi
        ctrl_reset_enable();
        ctrl_set_mux( HIMUX_SEL_REF_HI);
        ctrl_set_aperture( nplc_to_aper_n( 10));
        ctrl_param_read( &param );
        ctrl_reset_disable();
        target = 7.1 * param.clk_count_aper_n;
        break;


      case 6:
        // 11 NPLC mux mux ref-lo.
        ctrl_reset_enable();
        ctrl_set_mux( HIMUX_SEL_REF_LO);
        ctrl_set_aperture( nplc_to_aper_n( 11));
        ctrl_param_read( &param );
        ctrl_reset_disable();
        target = 0.0 * param.clk_count_aper_n;
        break;

      case 7:
        // 11 NPLC mux ref-hi.
        ctrl_reset_enable();
        ctrl_set_mux( HIMUX_SEL_REF_HI);
        ctrl_set_aperture( nplc_to_aper_n( 11));
        ctrl_param_read( &param );
        ctrl_reset_disable();
        target = 7.1 * param.clk_count_aper_n;
        break;


      case 8:
        // 12 NPLC  ref-lo
        ctrl_reset_enable();
        ctrl_set_mux( HIMUX_SEL_REF_LO);
        ctrl_set_aperture( nplc_to_aper_n( 12));
        ctrl_param_read( &param );
        ctrl_reset_disable();
        target = 0.0  * param.clk_count_aper_n;
        break;

      case 9:
        // 12 NPLC  ref-hi
        ctrl_reset_enable();
        ctrl_set_mux( HIMUX_SEL_REF_HI);
        ctrl_set_aperture( nplc_to_aper_n( 12));
        ctrl_param_read( &param );
        ctrl_reset_disable();
        target = 7.1 * param.clk_count_aper_n;
        break;



      default:
        // done
        usart_printf("done collcting obs\n");

        // shrink matrixes for the data collected
        m_resize( xs , row, m_cols( xs) );
        m_resize( y , row, m_cols( y) );
        return;
    } // switch


  // GAHHH.... the whole thin.g

  // I think that performing the conversion from counts to matrix. should be lifted out of the inner function/loop.

    
    unsigned row_start = row;

    // void collect_obs( app_t *app, unsigned discard_n, unsigned gather_n, unsigned *row, MAT *xs)
    collect_obs( app, &param, 2 , 5, &row, xs, himux_sel_last, himux_sel_last_n );

    for(unsigned r = row_start; r < row; ++r ) {
      // fill in y. and aperture.

      assert(r < m_rows(aperture));
      m_set_val( aperture, r, 0, param.clk_count_aper_n );

      assert(r < m_rows(y));
      m_set_val( y       , r , 0, target );
    } 

  }
}







static MAT * calibrate( app_t *app)
{

  /*
    moved this code.
  */

  // We have to create rather than use MNULL, else there
  // is no way to return pointers to the resized structure from the subroutine
  MAT *xs = m_get(1,1); // TODO change MNULL
  MAT *y = m_get(1,1);
  MAT *aperture = m_get(1,1);

  cal_collect_obs (app, xs, y, aperture );

/*
  printf("x\n");
  m_foutput(stdout, x);
  usart_flush();

  printf("y\n");
  m_foutput(stdout, y);
  usart_flush();
*/

  // regression to calc the betas.

  MAT *b =  regression( xs, y, MNULL );
  printf("b\n");
  m_foutput(stdout, b);
  usart_flush();

  assert( m_rows(b) == m_cols( xs) ); // calibration coeff is horizontal matrix.

  calc_predicted( b, xs, aperture);


  // TODO clean up mem.

/*
  M_FREE(x);
  M_FREE(x_);
  M_FREE(y);
  // M_FREE(b);
  M_FREE(predicted);

*/

  return b;
}



void loop2( app_t *app)
{
  MAT *b = calibrate( app);

  app->b = b;


  // return


}






static void calc_implied_resoltion(  MAT *x, MAT *b )
{
  /*
    old code, don't think this works anymore.
    without also having an aperture argument
    ---
    resolution will in fact depend on aperture.

  */
  // also want resolution by using one of the pluged in values and offset one..

  // we need a get row.
  // MAT *x = m_get(1,1);

  printf("============\n");
  printf("resolution/implied count\n");

  MAT *xx =  m_row_get( x, 0, MNULL );
  printf("xx\n");
  m_foutput(stdout, xx );

  MAT *ones =  m_ones( m_copy( xx, MNULL))  ;
  printf("ones\n");
  m_foutput(stdout, ones);

  // add delta of  1.
  MAT *deltaxx =  m_add( xx, ones, MNULL );
  printf("deltaxx \n");
  m_foutput(stdout, deltaxx );

  // predict
  MAT *predicted0 = m_mlt(xx, b, MNULL );
  printf("predicted0\n");
  m_foutput(stdout, predicted0 );

  // predict
  MAT *predicted1 = m_mlt(deltaxx, b, MNULL );
  printf("predicted1\n");
  m_foutput(stdout, predicted1 );

  MAT *diff =  m_sub( predicted0, predicted1, MNULL );
  printf("diff\n");
  m_foutput(stdout, diff);

  // TODO rename diff to resolution.
  assert(diff->m == 1 && diff->n == 1);
  double value = m_get_val( diff, 0, 0 );
  // TODO predicted, rename. estimator?
  char buf[100];
  printf("diff %s\n", format_float_with_commas(buf, 100, 9, value));

  // implied count
  int count = (10.0 + 10.0) / value;

  printf("count %u\n", count );



}


















// hmmm weights are all off...
#if 0

__attribute__((naked)) void dummy_function(void)
{
   __asm(".global __initial_sp\n\t"
         ".global __heap_base\n\t"
//         ".global __heap_limit\n\t"
         ".equ __initial_sp, STACK_BASE\n\t"
         ".equ __heap_base, HEAP_BASE\n\t"
 //        ".equ __heap_limit, (HEAP_BASE+HEAP_SIZE)\n\t"
   );
}
#endif



    /*
      EXTR
        fixed pos == fixed neg. so only record once - and it becomes a constant.
      -------
        8****
        - i think as soon as we hit int period. eg. 1PLC. or 200ms. we must turn of input immediately.
        - not wait until we we come to the end of a phase (var) . and then test whether we finished.
        - eg. we must have the time of the input signal - to be absolutely constant between measurements.
           - regardless we use fast rundown or slow rundown.

          - this means. being able to switch ref currents and signal independently.

        ***

        =========================
        - should try to get the two parameter thing working. with rundown.   (non slow).
          eg.  fix pos + var pos + rundown.   and fix neg + var neg.
          ----------
          no. because in the rundown the input is turned off. so it must be a different variable. but maybe should be left on.
        =========================

        - combine fix and var.  eg. so have total pos total neg in raw clock counts, and then add the raw rundown.
        - OR get the fpga - to count it up - in raw counts.
        - would need to 0
        - IMPORTNAT - then we have just two variables. if not using slow rundown. and three if are.

        - our flip_count is wrong. and confusing things. with fixed amount.

        - change names count_up , count_down.   count_fix_up,  count_var_up etc.  or cout_fix_pos.

        - make sure not including rundown in counts.

        - include the fix pos and neg counts
            even though they are equal.
            they are equal time.
            but they are *not* equal current.
            ---------

            for a certain integration time/period.    they will be constant.
            but we cannot create permutations with different integration times - if do not include.

        - for a certain time. the pos + neg should be a constant.

        - OR. instead of using counts. multiply by the times.
            then we could generate permutations.
            and then include the count * the limit.

        - perhaps we need three points. and generating extra values by running  at multiple of nplc is insufficient.
            for degrees freedom.

        - could record slow_rundown as separate var to rundown. and thus
          handle both options in the same calibration data.

          - actually this is quite interesting - because it would generate the 0 data points.

        - can/should  add a dummy observation.
            eg. count_up 0 , count_down 0, rundown 0 == 0

        - or perhaps better. without the slow slope.
            eg. just plug in 0 for the rundown.

        - think should probably not have constant.

        - we need a function run -> x_ vector.

        - perhaps try entire calibration without slow slope. as a first test.
        - and then secondary calibration. using the predicted values as input.

    */















  //MAT *x = concat_ones( x_, MNULL );

/*
  printf("x\n");
  m_foutput(stdout, x_ );

  printf("y\n");
  m_foutput(stdout, y );
*/

  // IMIPOORTANT -  perhaps the issue is cbuffer is overflowing????
  // yes seems ok.

  // printf("m is %u n",  x_->m );

/*
  // ths ones code looks buggy.
  MAT *ones = m_ones( m_copy( x_, MNULL  ));
  printf("ones\n");
  m_foutput(stdout, ones );
*/

/*
  MAT *j = m_get( x_-> m, 1 );
  MAT *ones = m_ones( j );
  printf("ones\n");
  m_foutput(stdout, ones );
*/

/*
  MAT *j = m_get( x_-> m, 1 );
  MAT *ones = m_ones( j );

  printf("ones\n");
  m_foutput(stdout, ones );
*/



// data is wrong. until the buffers are full.
#if 0
  // computed via octave
  // double v = (-6.0000e+00 * 1) + (4.6875e-02 * count_up) + ( -3.1250e-02 * count_down) + (-4.5475e-12 * clk_count_rundown);
  double v = (-6.0000e+00 * 1) + (4.6875e-02 * count_up) + ( -3.1250e-02 * count_down) + (-4.5475e-7 * clk_count_rundown);
  usart_printf("v %.7f, ", v );
#endif


#if 0
  static float clk_count_rundown_ar[ 10 ] ;
  size_t n = 5;
  // static int i = 0;

  float mean_;
  UNUSED(mean_);
  ////////////////////////
  ///////// stats

  // usart_printf("imodn %u ", i % n);

  {
  assert(n <= ARRAY_SIZE(clk_count_rundown_ar));

  clk_count_rundown_ar[ i % n ] =  clk_count_rundown;
  usart_printf("stddev_rundown(%u) %.2f, ", n, stddev(clk_count_rundown_ar, n) );


  mean_ = mean(clk_count_rundown_ar, n);
  usart_printf("mean (%u) %.2f, ", n, mean_ );
  }

#endif
#if 0
  {
  static float means[ 10 ];
  assert(n <= ARRAY_SIZE(means));
  means[ i % n  ] = mean_;
  usart_printf("stddev_means(%u) %.2f ", n, stddev(means, n));
  }


  double v2 = (-6.0000e+00 * 1) + (4.6875e-02 * count_up) + ( -3.1250e-02 * count_down) + (-4.5475e-7 * mean_ );
  usart_printf("v %.7f, ", v2 );
#endif






#if 0

  char buf[10];
  usart_printf("whoot %s\n", format_bits( buf, 10,  (0xf & ~(1 << 3))  ));
  usart_printf("whoot %s\n", format_bits( buf, 10, 0xf ));
#endif


#if 0
  // encapsutate into a function.
  uint32_t t = 5 * 20000000;
  spi_reg_write(SPI1, REG_CLK_COUNT_APER_N_HI, (t >> 24) & 0xff );
  spi_reg_write(SPI1, REG_CLK_COUNT_APER_N_LO, t & 0xffffff  );
  spi_reg_write(SPI1, REG_USE_SLOW_RUNDOWN, 0 );
  // spi_reg_write(SPI1, REG_HIMUX_SEL, HIMUX_SEL_REF_LO );
  spi_reg_write(SPI1, REG_HIMUX_SEL, HIMUX_SEL_REF_HI );
#endif


#if 0
  uint32_t ret;

  ///////////////////////////////////////////
  // write the mux select
  // himux_sel = 4'b1101;     // ref i
  spi_reg_write(SPI1, REG_HIMUX_SEL , 0b1101 ); // doesn't work to set reg_himux_sel
  ret = spi_reg_read(SPI1, REG_HIMUX_SEL);
  assert(ret == 0b1101 );

  spi_reg_write(SPI1, REG_CLK_COUNT_INIT_N, 20000 ); // doesn't work to set reg_himux_sel
  ret = spi_reg_read(SPI1, REG_CLK_COUNT_INIT_N );
  assert(ret == 20000);

/*
  spi_reg_write(SPI1, REG_CLK_COUNT_INIT_N, 20000 ); // doesn't work to set reg_himux_sel
  ret = spi_reg_read(SPI1, REG_CLK_COUNT_INIT_N );
  assert(ret == 20000);
*/
#endif



#if 0
  {

  printf("============\n");

  // make a zeros vector same length as b. to serve as origin
  MAT *temp0 =  m_zero( m_copy( b, MNULL))  ;
  MAT *zeros = m_transp( temp0, MNULL);

  MAT *origin = m_mlt(zeros, b, MNULL );
  printf("origin predicted \n");
  m_foutput(stdout, origin );

  }
#endif



  // MAT *xx =  m_row_get( x, 0, MNULL );

  // MAT *xx =  m_add( x, 0, MNULL );
  // MAT *delta = m_mlt(xx, b, MNULL );


