


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





static bool push_buffer1( MAT *buffer, unsigned *i, double value)
{
  /*
    Todo should return whether we reached the end...
  */
#if 0

  assert( m_cols(buffer) == 1 );
  unsigned idx = *i % m_rows(buffer);
  (*i)++;
  assert( idx < m_rows( buffer));
  m_set_val( buffer, idx, 0, value );
#endif

  assert(buffer);
  assert(i);

  // printf("i is %u\n", i ); 

  assert( m_cols(buffer) == 1 );

  bool full = false;
  if(*i == m_rows(buffer)) {
    full = true;
    *i = 0;
  }

  m_set_val( buffer, *i, 0, value );
  ++(*i);

  return full;
}




static void push_stats_buffer( app_t *app , double value )
{
  //////////////
  // report value....
  // printf("-----------------");
  char buf[100];
  printf("push_stats_buffer %sV ", format_float_with_commas(buf, 100, 7, value));
  // printf("\n");


  push_buffer1( app->stats_buffer, &app->stats_buffer_i, value);


  // m_foutput(stdout, app->stats_buffer );


  MAT *stddev = m_stddev( app->stats_buffer, 0, MNULL );
  assert( m_cols(stddev) == 1 && m_rows(stddev) == 1);
  double stddev_ = m_get_val( stddev, 0, 0);
  M_FREE(stddev);


  usart_printf("stddev(%u) %.2fuV, ", m_rows(app->stats_buffer), stddev_  * 1000000 );   // multiply by 10^6. for uV

  printf("\n");
}





// value is completely wrong.... 0.7????

//


#if 0
static bool push_buffer( app_t *app , double value )
{
  // leaf function
  // push_value onto buffer and rerturn if a new buffer we are ready to output



  bool full = push_buffer1( app->buffer, &app->buffer_i, value);

  // printf("push_buffer i %u of %u %lf\n", app->buffer_i , m_rows(app->buffer), value );


  /*
    all of this code needs to be extracted top level.

  */

  // all of this co
  if( full ) {
  // if( (app->buffer_i % m_rows( app->buffer))  ==  0 ) {
    // we have enought to push
    // m_foutput(stdout, app->buffer );

    // take the mean of the buffer.
    MAT *mean = m_mean( app->buffer, MNULL );
    assert(m_rows(mean) == 1 && m_cols(mean) == 1);
    double mean_ = m_get_val(mean, 0, 0);
    M_FREE(mean);

    /*
    // this calling function is problematic - as it couples the behavor.
      needs to be hoisted up to the top level. (the code that does the modulation).

    */
    push_stats_buffer( app, mean_ );

    /*printf("mean \n");
    char buf[100];
    printf("mean %sV ", format_float_with_commas(buf, 100, 7, mean_ ));
      */

    return true;
  }

  return false;
}

#endif








static void process( app_t *app, double predict )
{
  // deep nested functions are kind of normal in stats.

  /* The only difference between this and an auto zero mode.
    is how we calculate the value. using two obs or four.
    OK. so perhaps do the calcuation at higher level
  */

  printf("process \n");

#if 1
  // push onto buffer
  bool full = push_buffer1( app->buffer, &app->buffer_i, predict );

  if( full ) {
    // take the mean of the buffer.
    MAT *mean = m_mean( app->buffer, MNULL );
    assert(m_rows(mean) == 1 && m_cols(mean) == 1);
    double mean_ = m_get_val(mean, 0, 0);
    M_FREE(mean);

    double value = mean_;

    // push onto stats buffer
    push_buffer1( app->stats_buffer, &app->stats_buffer_i, value);


    MAT *stddev = m_stddev( app->stats_buffer, 0, MNULL );
    assert( m_cols(stddev) == 1 && m_rows(stddev) == 1);
    double stddev_ = m_get_val( stddev, 0, 0);
    M_FREE(stddev);

    // report
    char buf[100];
    printf("value %sV ", format_float_with_commas(buf, 100, 7, value));
    printf("stddev(%u) %.2fuV, ", m_rows(app->stats_buffer), stddev_  * 1000000 );   // multiply by 10^6. for uV
    printf("\n");

  }
#endif

}



static void simple_yield( app_t * app )
{
  update_console_cmd(app);

  static uint32_t soft_500ms = 0;
  // 500ms soft timer. should handle wrap around
  if( (system_millis - soft_500ms) > 500) {
    soft_500ms += 500;
    led_toggle();
  }
}





#include <alloca.h>

static double calc_predicted_val(  MAT *b , Run *run, Param *param )
{
  // do xs.
  MAT *xs = run_to_matrix( param,  run, MNULL );
  assert(xs);
  assert( m_rows(xs) == 1 );

  // do aperture
  MAT *aperture = m_get(1,1);
  m_set_val( aperture, 0, 0, param->clk_count_aper_n);

  // predicted
  MAT *predicted = calc_predicted( b, xs, aperture);      // TODO - combine this function....
  assert(m_cols(predicted) == 1);
  // m_foutput(stdout, predicted );
  double value = m_get_val(predicted, 0, 0 );


  M_FREE(xs);
  M_FREE(aperture);
  M_FREE(predicted);

  return value;
}



void loop1 ( app_t *app )
{
  usart_printf("=========\n");
  usart_printf("loop1\n");

  ctrl_set_pattern( 0 ) ;     // no azero.

  Run   run;
  Param param;

  while(true) {

    // configure  integrator
    ctrl_reset_enable();
    ctrl_set_mux( HIMUX_SEL_REF_HI );
    app->data_ready = false;
    ctrl_reset_disable();

    // block/wait for data
    while(!app->data_ready ) {

      // usart_printf("."); usart_flush();
      // we have a value.
      if(run.count_up ) {

        if(app ->b) {

          // calculate value and push onto buffer
          double predict = calc_predicted_val( app->b, &run, &param );
    
          char buf[100];
          // char *buf  = alloca( 100 );
          printf("value %sV ", format_float_with_commas(buf, 100, 7, predict ));
          process( app, predict ); 
        }
        // clear to reset
        memset(&run, 0, sizeof(Run));
      }

      simple_yield( app );   // change name simple update
      if(app->continuation_f) {
        return;
      }
    }

    // read the ready data
    run_read(&run);
    param_read_last( &param);

  }
}





/////////////////////////


/* Passing a continuation.

  - to allow calculating mean/std.
  - and to allow aggregating multiple entries. eg. nplc 50 == 5 lots of nplc 10.

  - the problem is that we cannot partially apply the continuation .  at the top level.
        - we could peel off the continuations off an array.
        - but that has type safety issues.

  - OR. pass a structure - with the named continuations.
  struct A
  {
     void (*continuation_for_yeild)( app_t *app, double val ) ;
     void (*continuation_for_stats)( app_t *app,  );
  }

  - or perhaps . it isn't really necessary and the signal processing chain . if just trunk to leaf

*/
#if 0

static void yield2( app_t *app, Run *run_zero, Param *param_zero, Run *run_sig, Param *param_sig )
{

  // we have both obs available...
  if(run_zero->count_up && run_sig->count_up ) {

    if(app ->b) {
      double predict_zero   = calc_predicted_val( app-> b , run_zero, param_zero );
      double predict_sig    = calc_predicted_val( app-> b , run_sig,  param_sig );
      double predict        = predict_sig - predict_zero;
      push_buffer(app, predict );
    }

    // clear to reset
    memset(run_zero, 0, sizeof(Run));
    memset(run_sig, 0, sizeof(Run));
  }

}
#endif





void loop2 ( app_t *app /* void (*pyield)( appt_t * )*/  )
{
  // could pass the continuatino to use.
  // auto-zero
  // iMPORTANT do three variable azero . by shuffling  values about.

  usart_printf("=========\n");
  usart_printf("loop2\n");

  assert(app);

  ctrl_set_pattern( 0 ) ;     // no azero.

  /*
    have tmp.
    1) do ref-hi/sig .
    2) then lo. and convert using 3 values.
    3) then copy lo to temp.   and use for the next input.

  */

  Run   run_zero;
  Param param_zero;
  Run   run_sig;
  Param param_sig;

  memset(&run_zero, 0, sizeof(Run));
  memset(&run_sig, 0, sizeof(Run));


  while(true) {

    printf("---------------\n");

    // configure ref_lo
    ctrl_reset_enable();
    ctrl_set_mux( HIMUX_SEL_REF_LO );
    app->data_ready = false;
    ctrl_reset_disable();


    // block/wait for data
    while(!app->data_ready ) {

      // we have both obs available...
      if(run_zero.count_up && run_sig.count_up ) {

        if(app ->b) {
          double predict_zero   = calc_predicted_val( app->b , &run_zero, &param_zero );
          double predict_sig    = calc_predicted_val( app->b , &run_sig,  &param_sig );
          double predict        = predict_sig - predict_zero;

          process( app, predict ); 
        }

        // clear to reset
        memset(&run_zero, 0, sizeof(Run));
        memset(&run_sig, 0, sizeof(Run));
      }

      simple_yield( app );   // change name simple update
      if(app->continuation_f) {
        return;
      }
    }

    // read the ready data. is it doing another complete by the time we process it?
    run_read(&run_zero);
    param_read_last( &param_zero);
    assert(param_zero.himux_sel ==  HIMUX_SEL_REF_LO );
    // char buf[100];
    // printf("got value should be zero %sV\n", format_float_with_commas(buf, 100, 7, calc_predicted_val( app-> b , &run_zero, &param_zero )));



    // configure ref_hi
    ctrl_reset_enable();
    ctrl_set_mux( HIMUX_SEL_REF_HI );
    app->data_ready = false;
    ctrl_reset_disable();

    // block/wait for data
    while(!app->data_ready ) {

      simple_yield( app );
      if(app->continuation_f) {
        return;
      }
    }

    // read the ready data
    run_read(&run_sig);
    param_read_last( &param_sig);
    assert(param_sig.himux_sel == HIMUX_SEL_REF_HI );  // this is not correct....

    // printf("got value should be predict %sV\n", format_float_with_commas(buf, 100, 7, calc_predicted_val( app-> b , &run_sig , &param_sig )));

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



