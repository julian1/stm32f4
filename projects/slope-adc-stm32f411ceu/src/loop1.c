



#include "assert.h"
#include "streams.h"  // usart_printf
#include "format.h" // format_bits
#include "usart.h"   // usart_flus()
#include "util.h"   // system_millis


#include "stats.h"    // stddev. should probably implement with mesch version
                      //


#include "regression.h"


#include <stdbool.h>

#include <matrix.h>

#include <libopencm3/stm32/spi.h>   // SPI1 .. TODO remove. pass spi by argument

#include "app.h"


/*
  - We should be passing the params. and the calibration constant to use.  as argument.
  - we kind of want to be able to set parameters... to observe the effect

  - especially the input source.
  - remember we are not using a stack...
  - number of obs. for smoothing.

  - main feature.  params and cal need to be the same. or passed.
  -- set the parameters. calibrate. run the ordinary loop.
  -----------

  maybe just define this loop structure statically somewhere?
  And then this can work.

  -- OR.
    define/ put it in app.  but pass it explicitly. meaning we can have several.
*/




//void loop1(app_t *app, MAT *b)



// TODO change name predict. to est. or estimator .


/*
  - rather than functions calling functions.
  - easier to use event function.
  -------------------------------------

  OK. OR alternatively should we use the pattern controller. on the fpga.
  and then record himux in the arrays.

  Then available
  Issue is configuration.
  ---------------------------------------

  It could make the software very nice.

  Is there a way to
    write himux sel directly.   - eg. for calibrating.

  pattern controller.
    in one pattern should just always set modulation himux_sel to register_bank himux_sel.
  ---------------

  it is vastly nicer. not to have to write the fpga. in ordinary operation.
  --------------

  - anything that gets changed by the pattern controller  - needs to be moved from Param to Run. data structure.
  ------------
  issue with pattern_controller.
    - is that we have to duplicate every variable. - so that mcu can read the valid parameter for the run just completed.
    - or else read everything quicly enough. that its correct for the last modulation, before the pattern controller changes things.
      but this doesn't work, because it switches on the interupt.


*/


static void collect_obs_azero( app_t *app, Param *param, unsigned discard_n, unsigned gather_n, unsigned *row, MAT *xs /* , unsigned himux_sel Params_out */)
{
/*
  can pass himux_sel.
  or nplc
  or whatever parameter is being varied under the strategy.

  // we cannot do a subtraction here. these are raw counts.
  // need to project the values using calibration coeff.
  // then partition the matrix, into two , then do a row by row subtraction.
  ---------------

  If we are going to be doing complicated matrix calculation etc.
  then we really need to perform the read, and set in the interupt.
  - alternatively when writing - we are performing a reset here. so it should work.

  We could even pass a straight array of Params.
  that would make the calling interface the same. 
  ------------

  HMMMM...
  really not sure.
  - writing these stupid fill in loops.
  ------------

  just enabling the pattern controller to do this would simplify stuff.
  and passing an arry with himux_sel.

*/

  assert(row);

  // note/record the signal to use. eg. sig or ref-hi.
  unsigned signal = param->himux_sel;

  unsigned obs = 0;
  unsigned himux_sel[ 100 ];

  while(obs < gather_n ) {
    assert(row);
    assert(xs);
    UNUSED(discard_n);

    // ref-lo first.
    ctrl_reset_enable();
    ctrl_set_mux( HIMUX_SEL_REF_LO);
    ctrl_reset_disable();

    unsigned row_before = *row;
    collect_obs( app, param, 1 , 1, row, xs );

    for( unsigned r = row_before; r < *row; ++r)
      himux_sel[ r] = HIMUX_SEL_REF_LO ;

    ///////////

    // now signal/ ref hi
    ctrl_reset_enable();
    ctrl_set_mux( signal /*HIMUX_SEL_REF_HI */);  // change to sig-hi
    ctrl_reset_disable();

    row_before = *row;
    collect_obs( app, param, 1 , 1, row, xs );

    for( unsigned r = row_before; r < *row; ++r)
      himux_sel[ r] = signal;


    ++obs;
  }
}




void loop1 ( app_t *app)
{
  usart_printf("=========\n");
  usart_printf("loop1\n");

  assert(app);
  // assert( HIMUX_SEL_REF_LO ==  0b1011  );

/*
  // don't need static.
  float predict_ar[ 10 ] ;
  size_t n = 5;   // can change this.
  int i = 0;

  memset( predict_ar, 0, sizeof( predict_ar));
*/


  /////////////////
  unsigned  max_rows =  10 ;
  MAT *xs =       m_get(max_rows, X_COLS );
  MAT *aperture = m_get(max_rows, 1 );


  /* params are relatively unchanging...
    the authoritative source of state can still be the fpga.
    should probably hold in reset while read, to avoid emi influencing integrator...
  */
  ctrl_reset_enable();
  Param param;
  param_read( &param);
  ctrl_reset_disable();



  while(true) {

    // normally will gather only one obs.
    // EXTR write another collec_obs . that uses auto-zero.
    // OR.    Would be ea
    // discard == 0, gather 1.

    unsigned row = 0;
    printf("\n");


    unsigned row_start = row;

    // void collect_obs( app_t *app, Param *param, unsigned discard_n, unsigned gather_n, unsigned *row, MAT *xs);
    // collect_obs( app, &param, 2 , 5, &row, xs );

    collect_obs_azero( app, &param, 0, 1, &row, xs);


    for(unsigned r = row_start; r < row; ++r ) {
      // fill aperture, from param.
      assert(r < m_rows(aperture));
      m_set_val( aperture, r, 0, param.clk_count_aper_n );
    }

    // shrink
    m_resize( xs,       row, m_cols( xs) );
    m_resize( aperture, row, m_cols( aperture));


    // there's no easy halt.... or halt will leak memory...
    if(app ->b) {

        MAT *predicted = calc_predicted( app->b, xs, aperture);
        assert(m_cols(predicted) == 1);

        m_foutput(stdout, predicted );

        // now we want the mean as value...


/*
        double value = m_get_val(predicted, 0, 0 );


        // TODO predict, rename. estimator?
        char buf[100];
        printf("predict %sV ", format_float_with_commas(buf, 100, 7, value));
*/

    }

  }


}




#if 0
  // TODO move to app_t structure?.
  static uint32_t soft_500ms = 0;

  while(true) {


    if(app->data_ready) {

      // TODO - this to use the collect_obs() func - to get multiple observations. then average for desired period.
      // then stddev()
      // clear
      app->data_ready = false;

      // in priority
      Run run;
      run_read(&run );
      run_report(&run, 0);

      if(app ->b) {

          // run_to_matrix should have the aperture?

          MAT *x = run_to_matrix( &run, MNULL );
          assert(x );

          // run_to_aperture()...
          // MAT *aperture = m_get(1, 1);
          // m_set_val( aperture, 0, 0, run.clk_count_aper_n );


          MAT *aperture = run_to_aperture(&run, MNULL);


          MAT *predicted = calc_predicted( app->b, x, aperture);

          double value = m_get_val(predicted, 0, 0 );


          // TODO predict, rename. estimator?
          char buf[100];
          printf("predict %sV ", format_float_with_commas(buf, 100, 7, value));


          predict_ar[ i++ % n ] = value;
          usart_printf("stddev(%u) %.2fuV, ", n, stddev(predict_ar, n) * 1000000 );   // multiply by 10^6. for uV ?

          // usart_flush();

          M_FREE(x);
          M_FREE(aperture);
          M_FREE(predicted);
      }

      usart_printf("\n");
    }



    // pump the main processing stuff
    // most of this could be surrendered.
    // do processing.
    update_console_cmd(app);

    // 500ms soft timer. should handle wrap around
    if( (system_millis - soft_500ms) > 500) {
      soft_500ms += 500;
      led_toggle();
    }

    // if there is another continuation to run, then bail
    if(app->continuation_f) {
      return;
    }
  }
#endif




    // Except when switching parameters, doing auto zero, we will only get one at a time.
    // nevermind it is still useful
    // doesn't matter.

    // NO. we can pass in a hires select parameter. as to what to sample.

    /*
        - extr. having the row as a pointer. makes it very easy to collect multiple values

        - also in the same way we pass in the target. we can pass in other information.
          to get encoded in an output array.
          array does *not* need to be a MAT matrix. could be unsigned *flags;
    */



