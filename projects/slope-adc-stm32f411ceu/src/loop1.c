



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
  MAT *xs = m_get(1,1); // TODO change MNULL
  // MAT *y = m_get(1,1);
  MAT *aperture = m_get(1,1);


  /////////////////
  // TODO fix - don't allocate, then resize
  // eg. 10x 10NPLC
  unsigned  max_rows =  10 ;
  m_resize( xs ,        max_rows, X_COLS );
  // m_resize( y ,         max_rows, 1 );
  m_resize( aperture,   max_rows, 1 );


  // params are relatively unchanging...
  // Do once.
  Param param;
  param_read( &param);

  while(true) {

    // discard == 0, gather 1.

    unsigned row = 0;
    printf("\n");


    unsigned row_start = row;

    // void collect_obs( app_t *app, Param *param, unsigned discard_n, unsigned gather_n, unsigned *row, MAT *xs);
    collect_obs( app, &param, 2 , 5, &row, xs );

    for(unsigned i = row_start; i < row; ++i ) {

      // fill aperture, from param.

      assert(row < m_rows(aperture));
      m_set_val( aperture, row, 0, param.clk_count_aper_n );
    } 


    // there's no easy halt.... or halt will leak memory...
    if(app ->b) {

        MAT *predicted = calc_predicted( app->b, xs, aperture);

        double value = m_get_val(predicted, 0, 0 );


        // TODO predict, rename. estimator?
        char buf[100];
        printf("predict %sV ", format_float_with_commas(buf, 100, 7, value));


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


}






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



