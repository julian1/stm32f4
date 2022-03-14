

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



// TODO change name predict. to est. or estimator .


void loop3 ( app_t *app)
{
  usart_printf("=========\n");
  usart_printf("loop3\n");

  assert(app);

  // don't need static.
  float predict_ar[ 10 ] ;
  size_t n = 5;   // can change this.
  int i = 0;

  memset( predict_ar, 0, sizeof( predict_ar));


  /*
    - OK. rather than passing around app_t in these functions so we can poll data_ready.
    it would be cleaner interupt locally would be easier.

    - but lets try to get an auto-zero working.
      Ahhhh.
  */


  // read the hires mux select. to figure out what we should be sampling.

  // #define REG_HIMUX_SEL           18
  


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

          MAT *x = run_to_matrix( /*&app->params,*/ &run, MNULL );
          assert(x );

          MAT *predict = m_mlt(x, app->b, MNULL );

          // result is 1x1 matrix
          assert(predict->m == 1 && predict->n == 1);
          double value = m_get_val( predict, 0, 0 );

          // TODO use the matrix operations. same as loop2.
          value /=  run.clk_count_aper_n;

          // TODO predict, rename. estimator?
          char buf[100];
          printf("predict %sV ", format_float_with_commas(buf, 100, 7, value));


          predict_ar[ i++ % n ] = value;
          usart_printf("stddev(%u) %.2fuV, ", n, stddev(predict_ar, n) * 1000000 );   // multiply by 10^6. for uV ?

          // usart_flush();

          M_FREE(x);
          M_FREE(predict);
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
}
