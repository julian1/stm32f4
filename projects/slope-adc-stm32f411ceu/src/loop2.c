


#include "assert.h"
#include "streams.h"  // usart_printf
#include "format.h" // format_bits
#include "usart.h"   // usart_flus()
#include "util.h"   // led/system_millis




#include "regression.h"

#include "app.h"




/*
  two ways to do this.
    - 1) oversize matrix. and adjust row pointer. then shrink.
    - 2) preallocate oversized - then on each row increase row dimension.

  using a row pointer is fairly simple and probably sufficient.

*/


static void collect_obs( app_t *app, unsigned *row, unsigned discard, unsigned gather, double y_, MAT *xs, MAT *aperture,  MAT *y)
{
    assert(row);
    assert(xs);
    assert(aperture);
    assert(y);

    assert( m_rows(xs) == m_rows(y ));
    assert( m_rows(aperture) == m_rows(y ));

    assert( m_cols(aperture) == 1 );
    assert( m_cols(y) == 1 );


    // obs per current configuration
    unsigned obs = 0;

    // this condition should be inse
    while(obs < discard + gather) {



      // if we got data handle it.
      if(app->data_ready) {
        // in priority
        app->data_ready = false;

        // get run details
        Run run;
        run_read(&run );
        run_report(&run, 0);


        // only if greater than
        if(obs >= discard ) {


          assert(*row < m_rows(xs));

          // do xs.
          MAT *xs1 = run_to_matrix(  &run, MNULL );
          assert(xs1);
          assert( m_rows(xs1) == 1 );

          assert(xs);
          m_row_set( xs, *row, xs1 );
          M_FREE(xs1);


          // do aperture
          MAT *app_ = run_to_aperture(  &run, MNULL );
          assert(app_);
          assert( m_rows(app_) == 1 );

          assert(aperture);
          m_row_set( aperture, *row, app_ );
          M_FREE(app_);

          // do y/target
          m_set_val( y, *row, 0, y_ );


          ++*row;
        }
        else {
          usart_printf("discard");

        }

        usart_printf("\n");
        ++obs;
      }

      // update_console_cmd(app);
      // usart_output_update(); // shouldn't be necessary, now pumped by interupts.


      // 250ms
      static uint32_t soft_250ms = 0;
      if( (system_millis - soft_250ms) > 250) {
        soft_250ms += 250;
        led_toggle();
      }


    } // while

}





/*
  - ok. think we want an intermediate structure...
  so we can use this once

  - could also record the configuration
*/



static void cal_collect_obs(app_t *app, MAT *xs, MAT *y, MAT *aperture)
{

  usart_printf("=========\n");
  usart_printf("cal loop\n");


  // WRONG. should be presized.... before passing here

  unsigned row = 0;

  unsigned  max_rows =  10 * 5;

  m_resize( xs ,        max_rows, X_COLS );
  m_resize( y ,         max_rows, 1 );
  m_resize( aperture,   max_rows, 1 );



  double aperture_ = 0;

  for(unsigned i = 0; /*i < 10*/; ++i )
  {
    double target;
    // switch integration configuration
    switch(i) {

      // what the hell?

      case 0:
        // 8 NPLC  ref-lo
        aperture_ = nplc_to_aper_n( 8);
        ctrl_reset_enable();
        ctrl_set_mux( HIMUX_SEL_REF_LO);
        ctrl_set_aperture( aperture_ );
        ctrl_reset_disable();
        target = 0.0  * aperture_;
        break;

      case 1:
        // 8 NPLC  ref-hi
        aperture_ = nplc_to_aper_n( 8);
        ctrl_reset_enable();
        ctrl_set_mux( HIMUX_SEL_REF_HI);
        ctrl_set_aperture( aperture_  );
        ctrl_reset_disable();
        target = 7.1 * aperture_;
        break;

      case 2:
        // 9 NPLC  ref-lo
        aperture_ = nplc_to_aper_n( 9);
        ctrl_reset_enable();
        ctrl_set_mux( HIMUX_SEL_REF_LO);
        ctrl_set_aperture( aperture_ );
        ctrl_reset_disable();
        target = 0.0  * aperture_;
        break;

      case 3:
        // 9 NPLC  ref-hi
        aperture_ = nplc_to_aper_n( 9);
        ctrl_reset_enable();
        ctrl_set_mux( HIMUX_SEL_REF_HI);
        ctrl_set_aperture( aperture_  );
        ctrl_reset_disable();
        target = 7.1 * aperture_;
        break;

      case 4:
        // 10NPLC  ref-lo
        aperture_ = nplc_to_aper_n( 10);
        ctrl_reset_enable();
        ctrl_set_mux( HIMUX_SEL_REF_LO);
        ctrl_set_aperture( aperture_ );
        ctrl_reset_disable();
        target = 0.0  * aperture_;
        break;

      case 5:
        // 10NPLC  ref-hi
        aperture_ = nplc_to_aper_n( 10);
        ctrl_reset_enable();
        ctrl_set_mux( HIMUX_SEL_REF_HI);
        ctrl_set_aperture( aperture_  );
        ctrl_reset_disable();
        target = 7.1 * aperture_;
        break;


      case 6:
        // 11 NPLC mux mux ref-lo.
        aperture_ = nplc_to_aper_n( 11);
        ctrl_reset_enable();
        ctrl_set_mux( HIMUX_SEL_REF_LO);
        ctrl_set_aperture( aperture_ );
        ctrl_reset_disable();
        target = 0.0  * aperture_;
        break;

      case 7:
        // 11 NPLC mux ref-hi.
        aperture_ = nplc_to_aper_n( 11);
        ctrl_reset_enable();
        ctrl_set_mux( HIMUX_SEL_REF_HI);
        ctrl_set_aperture( aperture_  );
        ctrl_reset_disable();
        target = 7.1 * aperture_;
        break;


      case 8:
        // 12 NPLC  ref-lo
        aperture_ = nplc_to_aper_n( 12);
        ctrl_reset_enable();
        ctrl_set_mux( HIMUX_SEL_REF_LO);
        ctrl_set_aperture( aperture_ );
        ctrl_reset_disable();
        target = 0.0  * aperture_;
        break;

      case 9:
        // 12 NPLC  ref-hi
        aperture_ = nplc_to_aper_n( 12);
        ctrl_reset_enable();
        ctrl_set_mux( HIMUX_SEL_REF_HI);
        ctrl_set_aperture( aperture_  );
        ctrl_reset_disable();
        target = 7.1 * aperture_;
        break;



      default:
        // done
        usart_printf("done collcting obs\n");

        // shrink matrixes for the data
        m_resize( xs , row, X_COLS   );
        m_resize( y , row, 1 );
        return;
    } // switch


    // static unsigned collect_obs( app_t *app, unsigned row, unsigned discard, unsigned gather, double y_, MAT *x, MAT *aperture,  MAT *y)

    collect_obs( app, &row, 2 , 5 , target, xs, aperture, y );

  } // state for
}



// generally useable
// need jj




static MAT * calibrate( app_t *app)
{

  /*
    moved this code.
  */

  // We have to create rather than use MNULL, else there
  // is no way to return pointers to the resized structure from the subroutine
  MAT *x = m_get(1,1); // TODO change MNULL
  MAT *y = m_get(1,1);
  MAT *aperture = m_get(1,1);

  cal_collect_obs (app, x, y, aperture );

/*
  printf("x\n");
  m_foutput(stdout, x);
  usart_flush();

  printf("y\n");
  m_foutput(stdout, y);
  usart_flush();
*/

  // regression to calc the betas.

  MAT *b =  regression( x, y, MNULL );
  printf("b\n");
  m_foutput(stdout, b);
  usart_flush();

  assert( m_rows(b) == m_cols( x) ); // calibration coeff is horizontal matrix.

  calc_predicted( b, x, aperture);


  // TODO clean up mem.

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


