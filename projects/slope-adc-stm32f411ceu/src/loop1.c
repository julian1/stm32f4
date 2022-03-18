


// #include <stdbool.h>

#include "assert.h"
#include "streams.h"  // usart_printf
#include "format.h" // format_bits
#include "usart.h"   // usart_flus()
#include "util.h"   // system_millis


#include "stats.h"    // stddev. should probably implement with mesch version
                      //


#include "regression.h"
#include <matrix.h>
#include "app.h"





// OK. lets try the same. except with an autozero as well.

static void yield( app_t *app, Run *run, Param *param )
{
  UNUSED(app);

  // we have data...
  if(run->count_up || run->count_down) {
    // data is ready

    // printf("-----------------\n");

    // run_report(run);
    // param_report(param );

    // OK. on the scope we can see small timing differences... due to processing

    if(app ->b) {

        // do xs.
        MAT *xs = run_to_matrix( param,  run, MNULL );
        assert(xs);
        assert( m_rows(xs) == 1 );

        // do aperture
        MAT *aperture = m_get(1,1);
        m_set_val( aperture, 0, 0, param->clk_count_aper_n);

        // predicted
        MAT *predicted = calc_predicted( app->b, xs, aperture);
        assert(m_cols(predicted) == 1);
        // m_foutput(stdout, predicted );

        // now we want the mean as value...
        double value = m_get_val(predicted, 0, 0 );
        char buf[100];
        printf("predict %sV ", format_float_with_commas(buf, 100, 7, value));

        M_FREE(xs);
        M_FREE(aperture);
        M_FREE(predicted);
    }

    // clear to reset
    memset(run, 0, sizeof(Run));
  }

  
  update_console_cmd(app);

  static uint32_t soft_500ms = 0;
  // 500ms soft timer. should handle wrap around
  if( (system_millis - soft_500ms) > 500) {
    soft_500ms += 500;
    led_toggle();
  }


}






void loop1 ( app_t *app)
{
  usart_printf("=========\n");
  usart_printf("loop1\n");

  assert(app);

  ctrl_set_pattern( 0 ) ;     // no azero.

  while(true) {


    // configure  integrator
    ctrl_reset_enable();
    ctrl_set_mux( HIMUX_SEL_REF_HI );
    // ctrl_set_aperture( nplc_to_aper_n( 8) );
    ctrl_reset_disable();


    Run   run;
    Param param;

    // block/wait for data
    while(!app->data_ready ) {

      yield(app, &run, &param);
      // if there is another continuation to run, then bail
      if(app->continuation_f) {
        return;
      }
    }
    app->data_ready = false;

    // read the ready data
    run_read(&run);
    param_read_last( &param);

  }

}





#if 0


void loop1 ( app_t *app)
{
  usart_printf("=========\n");
  usart_printf("loop1\n");

  assert(app);

  ctrl_set_pattern( 0 ) ;     // no azero.

  while(true) {

    // how long does it take to process. it would be better to process in downtime.
    // we could do this with a coroutine.
    // one routine to get the data, and the other to process during downtime.

    // configure
    ctrl_reset_enable();
    ctrl_set_mux( HIMUX_SEL_REF_HI );
    ctrl_set_aperture( nplc_to_aper_n( 8) );
    ctrl_reset_disable();



    // block for data
    while(!app->data_ready /* && !app->halt */ ) {
      // yield( app, /* & run1, & run2 */ );
      // yield();
      // update_console_cmd(app);
      // blink led.
      // or we actually process the data in here...  by just checking if we have values.

      yield(app);

      // if there is another continuation to run, then bail
      if(app->continuation_f) {
        return;
      }


    }
    app->data_ready = false;


    // read run
    Run run;
    run_read(&run);
    run_report(&run);

    // read param
    Param param;
    param_read_last( &param);
    param_report(&param );




    if(app ->b) {
      // it would be much better, to process/ data out of band.
      //
      // do xs.
      MAT *xs = run_to_matrix( &param,  &run, MNULL );
      assert(xs);
      assert( m_rows(xs) == 1 );

      // do aperture
      MAT *aperture = m_get(1,1);
      m_set_val( aperture, 0, 0, param.clk_count_aper_n);

      MAT *predicted = calc_predicted( app->b, xs, aperture);
      assert(m_cols(predicted) == 1);
      m_foutput(stdout, predicted );

      // now we want the mean as value...
      double value = m_get_val(predicted, 0, 0 );
      char buf[100];
      printf("predict %sV ", format_float_with_commas(buf, 100, 7, value));
    }

  }

}


#endif














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



#if 0

// void collect_obs( app_t *app, Param *param, unsigned discard_n, unsigned gather_n, unsigned *row, MAT *xs,  unsigned *himux_sel_last, unsigned himux_sel_last_n );
static void collect_obs_azero( app_t *app, Param *param, unsigned discard_n, unsigned gather_n, unsigned *row, MAT *xs, unsigned *himux_sel_last, unsigned himux_sel_last_n )
{

  assert(row);

  // note/record the signal to use. eg. sig or ref-hi.
  unsigned signal = param->himux_sel;

  while(gather_n-- > 0) {
    assert(row);
    assert(xs);
    UNUSED(discard_n);

    // ref-lo first.
    ctrl_reset_enable();
    ctrl_set_mux( HIMUX_SEL_REF_LO);
    ctrl_reset_disable();

    collect_obs( app, param, 1 , 1, row, xs, himux_sel_last, himux_sel_last_n );

    ///////////
    // now signal/ ref hi
    ctrl_reset_enable();
    ctrl_set_mux( signal /*HIMUX_SEL_REF_HI */);  // change to sig-hi
    ctrl_reset_disable();

    collect_obs( app, param, 1 , 1, row, xs, himux_sel_last, himux_sel_last_n );

  }
}
#endif



#if 0
void loop1 ( app_t *app)
{
  usart_printf("=========\n");
  usart_printf("loop1\n");

  assert(app);

  ctrl_set_pattern( 0 ) ;     // no azero.
  // ctrl_set_pattern( 10 ) ;    // azero. // PATTERN_AZERO


  unsigned row = 0;
  printf("\n");


  Run   run[10];
  Param param[10];


  Run2  run2;
  run2.run      = run;
  run2.param    = param;
  run2.n        = 10;

  // need to free. or allocate using alloca()
  run2.xs       = m_get(10 , X_COLS );
  run2.aperture = m_get(10 , 1 );


  // fill these in as we receive data. then process accordingly.
  Run2 zero_slot;
  Run2 meas_slot;
  Run2 gain_slot;


  // void collect_obs( app_t *app, unsigned discard_n, unsigned gather_n, unsigned *row,  Run2 *run2 )
  // collect_obs( app,  0, 2, &row, &run2 );

  /*
    - strategy. store in slots.
    - if have enough data. then convert.
    - if not enough. wait for more.
  */

  while(true) {



    // if we got data handle it.
    if(app->data_ready) {
      // in priority
      app->data_ready = false;

      // everything read and organized in one place.

      // get run details
      Run run;
      run_read(&run);
      run_report(&run);

      Param param;
      param_read_last( &param);
      param_report(&param );


      // do xs.
      MAT *xs1 = run_to_matrix( &param,  &run, MNULL );
      assert(xs1);
      assert( m_rows(xs1) == 1 );

      m_row_set( run2->xs, *row, xs1 );
      // M_FREE(xs1);

      // do aperture
      m_set_val( run2->aperture, *row, 0, param.  clk_count_aper_n);

      // IMOPRTANT Not clear we even need the Run2 structure.  just need the run and params used. and store in slots.


      // there's no easy halt.... or halt will leak memory...
      if(app ->b) {

          MAT *predicted = calc_predicted( app->b, run2.xs, run2.aperture);
          assert(m_cols(predicted) == 1);

          m_foutput(stdout, predicted );

          // now we want the mean as value...

          for(unsigned i  = 0; i < m_rows(predicted); ++i ) {

            double value = m_get_val(predicted, i, 0 );
            // TODO predict, rename. estimator?
            char buf[100];
            printf("predict %sV ", format_float_with_commas(buf, 100, 7, value));
          }
      }



   } // app ready



  }




    /* EXTR. IF we ran run_to_matrix()  here. then we would not need to pass down the xs and aperture arguments.
      Not sure we even want collect_obs()...
      because we need to synchronize iinputs.   eg. to read the zero value first, then the signal value.
      this could be done more effectively by reading each value o

      - change name collect_obs()  to wait_for_obs()
      -- Or else just do the blocking here. and then can handle the halt state.
      ------------

      Eg.   we want a loop
      with two slots. one for for ref-lo , and one for signal.
      and we just fill the value as we receive it. then we test if we have values for both.
      and do the computation.
      then clear them .
      ----
      this approach might be able to do both azero, and normal at the same time.
      eg. if get two signal, and there is already a signal stored - then process without the azero
    */



    // if there is another continuation to run, then bail
    // leaky...?
    // change this to a halt_task flag.
    if(app->continuation_f) {
      return;
    }



    // shrink oversized matrixes down.
    m_resize( run2.xs,       row, m_cols( run2.xs) );
    m_resize( run2.aperture, row, m_cols( run2.aperture));



  }


}
#endif



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



