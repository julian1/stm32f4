

// #include <stdbool.h>

#include "assert.h"
#include "format.h" // format_bits
#include "usart.h"   // usart_flus()
#include "util.h"   // system_millis


#include "regression.h"
// #include <matrix.h>
#include "app.h"
#include "cal.h"
#include "temp.h"




/*
  could manipulate optocoupplers/ relays to generate
*/



static void app_update_console_cmd2(app_t *app)
{

  int ch;
  clearerr(stdin);

  while( (ch = fgetc( stdin)) != EOF ) { // && -1 /EOF for error

    assert(ch >= 0);
    if(! ( ch == ';' || ch == '\r')) {

      if( app->cmd_buf_i < CMD_BUF_SZ - 1 )
        app->cmd_buf[ app->cmd_buf_i++ ] = ch;

      // echo to output. required for minicom.
      putchar( ch);

    }  else {

      // code should be CString. but this kind of works well enough...
      // we got a command
      app->cmd_buf[ app->cmd_buf_i ]  = 0;
      putchar('\n');
      // printf("got command '%s'\n", app->cmd_buf );

      if(strcmp(app->cmd_buf , "ok") == 0) {

        // we could set a flag here an then loop....
        app->block = false;
        printf("whoot!\n");
      }
      // unknown command
      else {
        printf( "unknown command '%s'\n", app->cmd_buf );
      }

      // reset buffer
      app->cmd_buf_i = 0;
      app->cmd_buf[ app->cmd_buf_i ]  = 0;

      // issue new command prompt
      printf("> ");
    }
  }
}




static void app_update2( app_t * app )
{
  // just have a separate
  app_update_console_cmd2(app);
  app_update_led(app);
}

static void prompt_for_user_ok( app_t *app)
{

  // pause for user input
  printf("type 'ok' when ready\n");
  printf("> ");

  app->block = true;
  while(app->block)
    app_update2(app);

}



static void app_loop22_( app_t *app )
{
  printf("=========\n");
  printf("app_loop2 test\ n");

  prompt_for_user_ok( app);
}







void app_loop22( app_t *app )
{
  printf("=========\n");
  printf("app_loop2 - cal loop using permutation of nplc/aperture\n");

  printf("Using cal slot %u . for model and var_n,fix_n \n",  app->cal_slot_idx );

  /*
    alsways work with slot 0?
  */

  Cal *cal = app->cal[  app->cal_slot_idx ];
  assert(cal);
  // TODO initially, if no cal. then should create a default.


  printf("model %u\n", cal->model);
  param_show( & cal->param );
  printf("\n");

  /*
    Note. no manipulation of var_n fix_n but should always be the same
  */
  {
    // should always hold.
    Param param;
    ctrl_param_read( app->spi, &param);
    assert( param.clk_count_var_n == cal->param.clk_count_var_n);
    assert( param.clk_count_fix_n == cal->param.clk_count_fix_n);
  }


  // clear last for mem
  if(app->last) {
    M_FREE(app->last);
  }



  // unsigned nplc_[] = { 9, 10, 11, 12 };
  // unsigned nplc[] = { 8, 9, 10, 11, 12, 13  };
  unsigned nplc[] = { 8, 9, 10, 11, 12, 13, 14, 15, 16  };
  // double y  = 0;

  unsigned obs_n = 7; // 7

  // may want a row pointer as well.
  unsigned  max_rows =  obs_n * ARRAY_SIZE(nplc) * 3;   // 3 == pos, neg, short,

  unsigned cols = 0;
  switch ( cal->model) {
    case 2: cols = 2; break;
    case 3: cols = 3; break;
    case 4: cols = 4; break;  // + intercept
    case 5: cols = 4; break;  // + flip_count
    default: assert(0);
  };

  MAT *xs       = m_get(max_rows, cols );
  MAT *y        = m_get(max_rows, 1);
  MAT *aperture = m_get(max_rows, 1); // required for predicted




  // printf("mux %s\n", himux_sel_format( mux));
  ctrl_set_mux( app->spi, HIMUX_SEL_SIG_HI );

  unsigned row = 0;



  // input voltage is  the outter loop

  double   vals[]  = { 7.146 , -7.146 } ;

  // loop hi/reverse polarity/short.
  for(unsigned j = 0; j < ARRAY_SIZE(vals); ++j)
  {
    double   y_  =  vals[ j ] ;

    printf("\n");
    printf("set input to %fV\n", y_ );

    // pause for user input
    prompt_for_user_ok( app);


    // loop apreture/ nplc
    for(unsigned h = 0; h < ARRAY_SIZE(nplc); ++h)
    {
      int nplc_ = nplc[h];
      uint32_t aperture_ = nplc_to_aper_n( nplc_ );  // move this up a loop.
      printf("nplc   %u\n", nplc_    );


      ctrl_reset_enable(app->spi);
      ctrl_set_aperture( app->spi, aperture_);
      ctrl_reset_disable(app->spi);


      for(unsigned i = 0; i < obs_n; ++i) {

        ctrl_reset_enable(app->spi);
        app->data_ready = false;
        ctrl_reset_disable(app->spi);

        // block/wait for data
        while(!app->data_ready ) {
          app_update( app );
          if(app->halt_func) {
            return;
          }
        }

        // read the data
        Run   run;
        Param param;

        ctrl_run_read(   app->spi, &run);
        ctrl_param_read( app->spi, &param); // doesn't really ???

        run_show(&run, app->verbose );
        /*
        // existing for calibration we won't be using b
        if(app ->b) {
          double predict = m_calc_predicted_val( app->cal, &run, &param );
          printf("val(current cal) %lf", predict );
        } */

        if(i < 2) {
          printf("discard");
        }
        else {

          // record xs
          assert(row < m_rows(xs));
          MAT *whoot = param_run_to_matrix( &param, &run, cols , MNULL );
          assert(whoot);
          assert( m_cols(whoot) == m_cols(xs) );
          assert( m_rows(whoot) == 1  );

          // printf("\n");
          // m_foutput(stdout, whoot );
          m_row_set( xs, row, whoot );
          M_FREE(whoot);

          // record aperture
          assert(row < m_rows(aperture));
          assert( param.clk_count_aper_n  == aperture_ );
          m_set_val( aperture, row, 0, param.clk_count_aper_n );


          // record y, as target * aperture
          assert(row < m_rows(y));
          m_set_val( y       , row , 0, y_  *  param.clk_count_aper_n );

          // increment row.
          ++row;
        }
        printf("\n");

      } // i
    } // j
  } // h



  // restore default
  ctrl_reset_enable(app->spi);
  ctrl_set_aperture( app->spi, nplc_to_aper_n( 10 ));
  ctrl_reset_disable(app->spi);



  printf("row %u\n", row);
  usart1_flush();

  // shrink matrixes to size collected data
  m_resize( xs, row, m_cols( xs) );
  m_resize( y,  row, m_cols( y) );
  m_resize( aperture, row, m_cols( aperture) ); // we don't use aperture


  calc_cal( app, y, xs, aperture );

}



#if 0
void app_loop22( app_t *app )
{
  printf("=========\n");
  printf("app_loop22 - cal loop using permutation of var_n, with fixed aperture\n");

  // ctrl_set_pattern( app->spi, 0 ) ;     // no azero.
  // default.
  assert( ctrl_get_var_n( app->spi) == 5500 );
  // nplc 10.
  ctrl_set_aperture( app->spi, nplc_to_aper_n( 10  ));


  const unsigned x_cols  = 3;

  // may want a row pointer as well.
  unsigned  max_rows =  10 * 9 * 2;
  MAT *xs       = m_get(max_rows, x_cols); // TODO change MNULL
  MAT *y        = m_get(max_rows, 1);
  MAT *aperture = m_get(max_rows, 1); // required for predicted


  // changing this - does change the rundown at the end
  // but not always.
  // 5500 is default.
  uint32_t var_n[] = { 5500, 5450, 5400, 5350, 5300, 5250, 5200, 5150, 5100  };


  unsigned row = 0;

    // loop var_n
  for(unsigned h = 0; h < ARRAY_SIZE(var_n); ++h)
  {
    uint32_t var_n_ = var_n[ h ];
    printf("var_n_ %lu\n", var_n_);


    // loop mux
    for(unsigned j = 0; j < 2; ++j)
    {
      uint32_t mux = j == 0 ? HIMUX_SEL_REF_LO : HIMUX_SEL_REF_HI;
      double   y_  = j == 0 ? 0                : 7.1;

      printf("mux %s\n", himux_sel_format( mux));


      ctrl_reset_enable(app->spi);
      ctrl_set_var_n( app->spi, var_n_);
      ctrl_set_mux( app->spi, mux );
      ctrl_reset_disable(app->spi);


      for(unsigned i = 0; i < 7; ++i) {


        ctrl_reset_enable(app->spi);  // TODO consider without this
        app->data_ready = false;
        ctrl_reset_disable(app->spi);

        // block/wait for data
        while(!app->data_ready ) {
          app_update( app );
          if(app->halt_func) {
            return;
          }
        }

        // read the data
        Run   run;
        Param param;

        ctrl_run_read(   app->spi, &run);
        ctrl_param_read( app->spi, &param);

        run_show(&run, app->verbose);


        if(i < 2) {
          printf("discard");
        }
        else {

          // record xs
          assert(row < m_rows(xs));
          MAT *whoot = param_run_to_matrix( &param, &run, x_cols, MNULL );
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
    } // h
  } // j



  // restore.
  ctrl_reset_enable(app->spi);
  ctrl_set_var_n( app->spi, 5500 );
  ctrl_reset_disable(app->spi);


  printf("row %u\n", row);
  usart1_flush();

  // shrink matrixes to size collected data
  m_resize( xs, row, m_cols( xs) );
  m_resize( y,  row, m_cols( y) );
  m_resize( aperture, row, m_cols( aperture) ); // we don't use aperture



  R regression;
  memset( & regression, 0, sizeof(regression));

  m_regression( xs, y, &regression );

  // predicted must be adjusted by aperture
  MAT *predicted =  m_calc_predicted( regression.b, xs, aperture );
  printf("\npredicted\n");
  m_foutput(stdout, predicted);
  usart1_flush();



  regression_show( &regression, stdout);

  // copy, for new memory
  // app->b = m_copy( regression.b, MNULL );

  // should switch and save new cal in slot 0. by default?

  printf("\nswitching to cal slot 0\n");

  assert(0);  // not implemented

#if 0
  app-> b_current_idx = 0;
  app->b[ app-> b_current_idx ] =  m_copy( regression.b, MNULL );
#endif

  r_free( &regression );

}
#endif




