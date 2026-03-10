/*
  rename cal.c perhaps
  or app_cal.c
*/



#include <stdio.h>
#include <assert.h>
#include <string.h>


#include <lib2/util.h>    // UNUSED

#include <peripheral/gpio.h>        // trigger

#include <app.h>
#include <cal/transfer.h>
#include <util.h> // nplc_to_aperture()
#include <lib2/stats.h>

#include <mode.h>

#include <data/cal.h>
#include <data/data.h>





bool app_transfer_repl_statement( app_t *app, const char *cmd)
{
  assert(app && app->magic == APP_MAGIC);


  // there is a bit of a name clash with the 'cal' specific funcs and repl.

  // better name? transfer w;  transfer b;   etc.
  // TODO. improve string/arg handling  by decoding argument
  if( false );
  else if(strcmp(cmd, "cal w") == 0)
    app_cal_w( app);
  else if(strcmp(cmd, "cal b") == 0)
    app_cal_b( app);
  else if(strcmp(cmd, "cal b10") == 0)
    app_cal_b10( app);
  else if(strcmp(cmd, "cal b100") == 0)
    app_cal_b100( app);
  else if(strcmp(cmd, "cal b1000") == 0)
    app_cal_b1000( app);

  else if(strcmp(cmd, "cal div100") == 0)
    app_cal_div100( app);
  else if(strcmp(cmd, "cal div1000") == 0)
    app_cal_div1000( app);

  else if(strcmp(cmd, "cal all") == 0)
    app_cal_all( app);

  else return 0;

  return 1;
}


void app_cal_all( app_t *app)
{
  assert(app && app->magic == APP_MAGIC);

  app_cal_w( app);
  app_cal_b( app);
  app_cal_b10( app);
  app_cal_b100( app);
  app_cal_b1000( app);

  app_cal_div100( app);
  app_cal_div1000( app);


  cal_show( app->cal);
}




void app_transfer( app_t *app, transfer_t *transfer)
{
  assert(transfer);

  assert(app && app->magic == APP_MAGIC);

  _mode_t *mode = app->mode;
  assert(mode && mode->magic == MODE_MAGIC);

  data_t *data = app->data;
  assert( data && data->magic == DATA_MAGIC);

  cal_t *cal = app->cal;
  assert( cal && cal->magic == CAL_MAGIC);

  /////////////////////////

  // sample off
  gpio_write( app->gpio_trigger, false);

  // ensure 10Meg not set, persists range change, and mode_reset()
  // REALLY NOT SURE ABOUT THIS... just put it in mode?
  // normal user operation (eg. changing adc, ranges etc) will not reset the mode.
  // so i
  // app->range_10Meg = false;

  // reset mode
  mode_reset( mode);

  // set the trigger delay for settle time
  sa_trig_delay_set( &mode->sa, period_to_aper_n(  1.f )); // 1 sec.

  // set normal sample acquisition/adc operation
  cr_mode_set( &mode->reg_cr, MODE_SA_ADC);

  // set nplc
  adc_aperture_set( &mode->adc, nplc_to_aperture( 10, app->line_freq ));


  /////////////////////////


  // step 1 - take reference
  transfer->step1( app);

  app_transition_state( app);

  double values[ 2 ];
  memset(values, 0, sizeof(values));

  data->show_reading = true;
  app_fill_buffer( app, values, ARRAY_SIZE(values));
  double mean0    = mean( values, ARRAY_SIZE(values));
  double stddev0  = stddev( values, ARRAY_SIZE(values));
  printf("mean0 %f,  stddev0 %.9f\n", mean0, stddev0);


  /////////////////////////////////

  // step 2 - take target
  transfer->step2( app);

  app_transition_state( app);

  //
  data->show_reading = false;
  app_fill_buffer( app, values, ARRAY_SIZE(values));
  double mean1    = mean( values, ARRAY_SIZE(values));
  double stddev1  = stddev( values, ARRAY_SIZE(values));
  printf("mean1 %f,  stddev1 %.9f\n", mean1, stddev1);


  // step 3. - compute cal for the target
  transfer->cal_set_value( cal, mean0, mean1);



  // print some values using cal to confirm
  data->show_reading = true;
  app_fill_buffer( app, values, ARRAY_SIZE(values));

  // app_cal_finish( app);
}




/*
  - using data->count_norm is flexible.  and independent of range.
  - could pass in the transfer function to use.
  - OR. can just apply the transform on the result.

  - eg. add function to stats.c  to scale the buffer.

*/


void app_fill_buffer( app_t *app, double *values, size_t n)
{
  data_t *data = app->data;
  assert( data && data->magic == DATA_MAGIC);


  // start sampling
  gpio_write( app->gpio_trigger, true);

  // obs loop
  for( unsigned i = 0; i < n; )
  {
    printf("i %u, ", i);

    // wait for adc data
    while( !app->adc_interrupt_valid )
      app_yield( app);

    app->adc_interrupt_valid = false;

    // get and compute counts
    data_update( data);
    if( data->valid) {

      values[ i] = data->count_norm;
      ++i;
    }

    printf("\n");
  }

  // stop sampling
  gpio_write( app->gpio_trigger, false);
}




void app_fill_buffer1( app_t *app, double *pos_values, double *neg_values, size_t n)
{
  data_t *data = app->data;
  assert( data && data->magic == DATA_MAGIC);


  // start sampling
  gpio_write( app->gpio_trigger, true);

  // take obs loop
  for( size_t i = 0; i < n; )
  {
    printf("i %u, ", i);

    // wait for adc data
    while( !app->adc_interrupt_valid )
      app_yield( app);

    app->adc_interrupt_valid = false;

    // get and compute counts
    data_update( data);

    // we take both hi and lo readings, since they have the same
    // ignore data->valid

    pos_values[i] = data->clk_count_refmux_pos;
    neg_values[i] = data->clk_count_refmux_neg;

    ++i;

    printf("\n");
  }

  // sampling off
  gpio_write( app->gpio_trigger, false);

}



#if 0

id      0
w       0.975650
b       -14.349407
b10     -1.434664
b100    -0.143489
b1000   -0.014346
div100  -143.470197
div1000 -1434.977747

#endif





