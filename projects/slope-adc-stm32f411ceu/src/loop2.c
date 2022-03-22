

// #include <stdbool.h>

#include "assert.h"
#include "format.h" // format_bits
#include "usart.h"   // usart_flus()
#include "util.h"   // system_millis


#include "regression.h"
// #include <matrix.h>
#include "app.h"


#include "voltage-source.h"








void app_loop22( app_t *app )
{
  printf("=========\n");
  printf("app_loop2 - cal loop using permutation of var_pos_n, with fixed aperture\n");

  ctrl_set_pattern( app->spi, 0 ) ;     // no azero.
  // default.
  assert( ctrl_get_var_pos_n( app->spi) == 5500 );
  // nplc 10.
  ctrl_set_aperture( app->spi, nplc_to_aper_n( 10  ));


#define X_COLS 4

  // may want a row pointer as well.
  unsigned  max_rows =  10 * 9 * 2;
  MAT *xs       = m_get(max_rows, X_COLS); // TODO change MNULL
  MAT *y        = m_get(max_rows, 1);
  MAT *aperture = m_get(max_rows, 1); // required for predicted


  // changing this - does change the rundown at the end 
  // but not always.
  // 5500 is default.
  uint32_t var_pos_n[] = { 5500, 5450, 5400, 5350, 5300, 5250, 5200, 5150, 5100  };


  unsigned row = 0;

    // loop var_pos_n
  for(unsigned h = 0; h < ARRAY_SIZE(var_pos_n); ++h)
  {
    uint32_t var_pos_n_ = var_pos_n[ h ]; 
    printf("var_pos_n_ %lu\n", var_pos_n_);


    // loop mux
    for(unsigned j = 0; j < 2; ++j)
    {
      uint32_t mux = j == 0 ? HIMUX_SEL_REF_LO : HIMUX_SEL_REF_HI;
      double   y_  = j == 0 ? 0                : 7.1;

      printf("mux %s\n", himux_sel_format( mux));


      ctrl_reset_enable(app->spi);
      ctrl_set_var_pos_n( app->spi, var_pos_n_);
      ctrl_set_mux( app->spi, mux );
      ctrl_reset_disable(app->spi);


      for(unsigned i = 0; i < 7; ++i) {


        ctrl_reset_enable(app->spi);  // TODO consider without this
        app->data_ready = false;
        ctrl_reset_disable(app->spi);

        // block/wait for data
        while(!app->data_ready ) {
          app_update( app );
          if(app->continuation_f) {
            return;
          }
        }

        // read the data
        Run   run;
        Param param;

        ctrl_run_read(        app->spi, &run);
        ctrl_param_read_last( app->spi, &param);

        run_report(&run);


        if(i < 2) {
          printf("discard");
        }
        else {

          // record xs
          assert(row < m_rows(xs));
          MAT *whoot = param_run_to_matrix( &param, &run, X_COLS, MNULL );
          assert(whoot);
          assert( m_cols(whoot) == m_cols(xs) );
          assert( m_rows(whoot) == 1  );

          // printf("\n");
          // m_foutput(stdout, whoot );
          m_row_set( xs, row, whoot );
          M_FREE(whoot);

          // record aperture
          assert(row < m_rows(aperture));
          m_set_val( aperture, row, 0, param.clk_count_aper_n );

          // record y, as target * aperture
          assert(row < m_rows(y));
          m_set_val( y       , row , 0, y_  *  param.clk_count_aper_n);

          // increment row.
          ++row;
        }

        printf("\n");



      } // i
    } // j
  } // h



  // restore.
  ctrl_reset_enable(app->spi);
  ctrl_set_var_pos_n( app->spi, 5500 );
  ctrl_reset_disable(app->spi);


  printf("row %u\n", row);
  usart1_flush();

  // shrink matrixes for the data collected
  m_resize( xs, row, m_cols( xs) );
  m_resize( y,  row, m_cols( y) );
  m_resize( aperture, row, m_cols( aperture) ); // we don't use aperture


  #if 0
  m_foutput(stdout, xs );
  usart1_flush();

  m_foutput(stdout, y );
  usart1_flush();
  #endif


  MAT *b = m_regression( xs, y, MNULL );
  assert( m_rows(b) == m_cols( xs) ); // calibration coeff is horizontal matrix.
  printf("b\n");
  m_foutput(stdout, b);
  usart1_flush();


  // no we need the aperture. to calc predicted

  MAT *predicted = m_calc_predicted( b, xs, aperture);
  printf("predicted\n");
  m_foutput(stdout, predicted);


  app->b = b;

}





