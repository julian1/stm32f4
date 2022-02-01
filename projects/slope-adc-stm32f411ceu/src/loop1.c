



#include "assert.h"
#include "streams.h"  // usart_printf
#include "format.h" // format_bits
#include "usart2.h"   // usart_flus()
#include "util.h"   // system_millis




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

void loop1 ( app_t *app)
{
  usart_printf("=========\n");
  usart_printf("loop1\n");
  usart_printf("> ");

  assert(app);

  /*
    app() subsumes update()
  */

  assert( HIMUX_SEL_REF_LO ==  0b1011  );


 // params_set( 5 * 20000000, 1, HIMUX_SEL_REF_LO );

#if 0
  Params  params;
  params_read( &params );

  usart_printf("overwriting params\n");
  // overwrite
  params.clk_count_int_n  = 1 * 20000000;
  params.use_slow_rundown = 1;
  // params.himux_sel = HIMUX_SEL_REF_LO;
  params.himux_sel = HIMUX_SEL_REF_HI;
  // params.himux_sel = HIMUX_SEL_SIG_HI;
  params_write(&params);

  params_report( &params);
#endif

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
      run_report(&run);

      if(app ->b) {

          MAT *x = run_to_matrix( &app->params, &run, MNULL );
          assert(x );

          MAT *predicted = m_mlt(x, app->b, MNULL );
          #if 0
                printf("predicted \n");
                m_foutput(stdout, predicted );
          #endif

          #if 1
          // result is 1x1 matrix
          assert(predicted->m == 1 && predicted->n == 1);
          double value = m_get_val( predicted, 0, 0 );
          // TODO predicted, rename. estimator?
          char buf[100];
          printf("predicted %s", format_float_with_commas(buf, 100, 7, value));
          #endif
          usart_flush();

          M_FREE(x);
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
}
