

#include "assert.h"
#include "streams.h"  // printf
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



// TODO change name predict. to est. or estimator .


void app_loop3 ( app_t *app)
{
  printf("=========\n");
  printf("app_loop3\n");

  assert(app);

  // don't need static.
  float predict_ar[ 10 ] ;
  size_t n = 5;   // can change this.
  int i = 0;

  memset( predict_ar, 0, sizeof( predict_ar));


  /*
    - OK. rather than passing around app_t in these functions so we can poll data_ready.
    it might be easier/cleaner to set interupt handler

    - still need a few things from app. like spi, calibration. etc.

    - but lets try to get an auto-zero working.
      Ahhhh.

    --------------
    there's an issue - that the second obs. is quite different from the first.
    could be DA settling.
  */


  // read the hires mux select. to figure out what we should be sampling.


  // figure out what we should be integrating.
  uint32_t signal = ctrl_get_mux();




  // TODO move to app_t structure?.
  static uint32_t led_tick_count = 0;

  while(true) {


    if(app->data_ready) {

      // TODO - this to use the collect_obs() func - to get multiple observations. then average for desired period.
      // then stddev()
      // clear
      app->data_ready = false;

      // in priority
      Run run;

      /////////////////////////////////////
      ctrl_reset_enable();
      ctrl_run_read(&run );

      // should be hold everything in reset???
      uint32_t mux = ctrl_get_mux();
      if(mux == signal ) {

        // set up for next time.
        ctrl_set_mux( HIMUX_SEL_REF_LO );
      }  else if (mux == HIMUX_SEL_REF_LO) {

        ctrl_set_mux( signal );
      } else assert(0);
      /////////////////////////////////////
      ctrl_reset_disable();

      // need a better name. run_print.
      run_show(&run, 0);


      if(app ->b) {

          MAT *x = param_run_to_matrix( /*&app->params,*/ &run, MNULL );
          assert(x );

          MAT *predict = m_mlt(x, app->b, MNULL );

          // result is 1x1 matrix
          assert(predict->m == 1 && predict->n == 1);
          double value = m_get_val( predict, 0, 0 );

          // TODO use the matrix operations. same as app_loop2.
          value /=  run.clk_count_aper_n;

          // TODO predict, rename. estimator?
          char buf[100];
          printf("predict %sV ", format_float_with_commas(buf, 100, 7, value));


          predict_ar[ i++ % n ] = value;
          printf("stddev(%u) %.2fuV, ", n, stddev(predict_ar, n) * 1000000 );   // multiply by 10^6. for uV ?

          // usart1_flush();

          M_FREE(x);
          M_FREE(predict);
      }

      printf("\n");
    }



    // pump the main processing stuff
    // most of this could be surrendered.
    // do processing.
    update_console_cmd(app);

    // 500ms soft timer. should handle wrap around
    if( (system_millis - led_tick_count) > 500) {
      led_tick_count += 500;
      led_toggle();
    }

    // if there is another continuation to run, then bail
    if(app->continuation_f) {
      return;
    }


  }
}
