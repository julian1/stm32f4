
DEPRECATED


#include "assert.h"
#include "streams.h"  // usart_printf
#include "ice40.h"  // spi_reg_read
#include "format.h" // format_bits
#include "usart.h"   // usart_flus()
#include "util.h"   // system_millis




#include "regression.h"


#include <stdbool.h>

#include <matrix.h>

#include <libopencm3/stm32/spi.h>   // SPI1 .. TODO remove. pass spi by argument

#include "app.h"







/*
  - think we need an association between calibration constants.
  and the params that were used.

  
  - so that can do a test on one - and then use the permuted ones.
    reasonably easily.  to observe the difference

*/

static void perm_collect_obs(app_t *app, MAT *x, MAT *y )
{
  /*
    ok. permuting the integration time - has about 1 /10k affect.
    what about permute teh fix/var frequency.
  */

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

  Params  params;
  params_set_main( &params,  1 * 20000000, 1, HIMUX_SEL_REF_LO);

  // permutate
  params_set_extra( &params,  10000, 650, 5500, 5500);
  // params_set_extra( &params,  10000, 750, 5500);
  params_write(&params);




  for(unsigned i = 0; /*i < 10*/; ++i )
  {
    double target;
    // switch integration configuration
    switch(i) {
      case 0:
        params_set_main( &params,  1 * 20000000, 1, HIMUX_SEL_REF_LO);
        target = 0.0;
        params_report(&params);
        params_write(&params);
        break;

      case 1:
        // same except mux lo.
        // IMPORTANT. it might make sense to record y in here...
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

    row = collect_obs( app, &params, row, 2 , 5 , x );
  } // state for
}






void permute(app_t *app, MAT *b)
{
  /*
    we want stderr of prediction.
  */

  assert(app);
  assert(b);

  MAT *x = m_get(1,1); // TODO change MNULL
  MAT *y = m_get(1,1);


  // We have to create rather than use MNULL, else there
  // is no way to return pointers to the resized structure from the subroutine
  perm_collect_obs(app, x , y );

  MAT *predicted = m_mlt(x, b, MNULL );
  printf("permuted predicted \n");
  m_foutput(stdout, predicted );
  usart_flush();

  // now calculate error of the predictors ..

  // for 2 second integration.
  // row 6:     14.2035525

  // 14.2035525  / 2
  // = 7.10177625
  // 1mV on +-10V range. 1/10k.
  // seems to be consistent for an integration period.
  // what about permuting fix/var times.



}






