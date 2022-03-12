


#include "assert.h"
#include "streams.h"  // usart_printf
#include "format.h" // format_bits
#include "usart.h"   // usart_flus()




#include "regression.h"


#include <stdbool.h>

#include <matrix.h>

#include <libopencm3/stm32/spi.h>   // SPI1 .. TODO remove. pass spi by argument

#include "app.h"





/*
  - ok. think we want an intermediate structure...
  so we can use this once

  - could also record the configuration
*/



static void cal_collect_obs(app_t *app, MAT *x, MAT *y )
{
  // gather obersevations
  // app argument is needed for data ready flag.
  // while loop has to be inner
  // might be easier to overside. and then resize.

  usart_printf("=========\n");
  usart_printf("cal loop\n");

  // rows x cols
  unsigned row = 0;

  #define MAX_OBS  30

  m_resize( x , MAX_OBS, X_COLS );      // constant + pos clk + neg clk.
  m_resize( y , MAX_OBS, 1 );

  /*
  
    OK. we need much better fine control over parameters. eg. being ablle to change on. the himux_sel. 

  */
  Params  params;
  params_set_main( &params,  1 * 20000000, 1, HIMUX_SEL_REF_LO);
  params_set_extra( &params,  10000, 700, 5500, 5500);
  params_write( &params );



  for(unsigned i = 0; /*i < 10*/; ++i )
  {
    double target;
    // switch integration configuration
    switch(i) {
      case 0:
/*
  this code should only be changing HIMUX_SEL_REF_LO to swap inputs.
*/

        params_set_main( &params,  1 * 20000000, 1, HIMUX_SEL_REF_LO);
        target = 0.0;
        params_report(&params);
        params_write(&params);
        break;

      case 1:
        // same except mux lo.
        params_set_main( &params,  1 * 20000000, 1, HIMUX_SEL_REF_HI);
        target = 7.1;
        params_report(&params);
        params_write(& params);
        break;

      default:
        // done
        usart_printf("done collcting obs\n");
        // shrink matrixes for the data
        m_resize( x , row, X_COLS   );
        m_resize( y , row, 1 );
        return;
    } // switch

    for(unsigned j = 0; j < 5; ++j ) {
      assert(row < y->m); // < or <= ????
      m_set_val( y, row + j, 0,  target );
    }

    // wait for data.
    // if we have to bail out of here... then what happens?

    row = collect_obs( app, &params, row, 2 , 5 , x );
  } // state for
}




static MAT * calibrate( app_t *app)
{

  // We have to create rather than use MNULL, else there
  // is no way to return pointers to the resized structure from the subroutine
  MAT *x = m_get(1,1); // TODO change MNULL
  MAT *y = m_get(1,1);

  cal_collect_obs (app, x, y );

  printf("x\n");
  m_foutput(stdout, x);
  usart_flush();

  printf("y\n");
  m_foutput(stdout, y);
  usart_flush();



#if 1
  MAT *b =  regression( x, y, MNULL );
  printf("b\n");
  m_foutput(stdout, b);

  usart_flush();

  MAT *predicted = m_mlt(x, b, MNULL );
  printf("predicted \n");
  m_foutput(stdout, predicted );
  usart_flush();
#endif


  {

  printf("============\n");

  // make a zeros vector same length as b. to serve as origin
  MAT *temp0 =  m_zero( m_copy( b, MNULL))  ;
  MAT *zeros = m_transp( temp0, MNULL);

  MAT *origin = m_mlt(zeros, b, MNULL );
  printf("origin predicted \n");
  m_foutput(stdout, origin );

  }


  // also want resolution by using one of the pluged in values and offset one..

  // we need a get row.
  // MAT *x = m_get(1,1);
  {
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


  // MAT *xx =  m_row_get( x, 0, MNULL );

  // MAT *xx =  m_add( x, 0, MNULL );
  // MAT *delta = m_mlt(xx, b, MNULL );

  }



  // TODO clean up mem.
  // TODO. our circular buffer does not handle overflow very nicely. - the result is truncated.

/*
  M_FREE(x);
  M_FREE(x_);
  M_FREE(y);
  M_FREE(b);
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




