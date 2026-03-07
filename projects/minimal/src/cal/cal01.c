

#include <stdio.h>
#include <string.h>
#include <assert.h>



#include <peripheral/gpio.h>        // trigger

#include <util.h>
#include <app.h>
#include <mode.h>
#include <lib2/util.h>    // ARRAY_SIZE
#include <lib2/stats.h>

#include <data/data.h>
#include <data/cal.h>





/*
  - using data->count_norm is the most flexible.  independent of range.
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


    data_update( data);
    if( data->valid) {

      // IMPORTANT - relies on havng the cal function.
      // may want t
      // values[ i] = data->reading;
      values[ i] = data->count_norm;
      ++i;
    }

    printf("\n");
  }

  // stop sampling
  gpio_write( app->gpio_trigger, false);
}




/*

  questions
    1. whether to work with cal_t structure directly.
        or communicate back values - through the range structure.
        - communicating back.  - can add/ range specific bounds checks. etc.
        - but this code is already range / cal specific.

        BUT we do not want to repeat cal constants.
          eg. many values for amp. gain  are shared. for lts, daq. and dcv.
          in cal structure.  so i think we dont use the range.
          may be on ohms.

    2. whether to have the gain ranges - scale according to the 10V. range b.  or else directly from adc adjusted_sum.

*/


void app_cal_01( app_t *app)
{
  assert( app && app->magic == APP_MAGIC);

  data_t *data = app->data;
  assert( data && data->magic == DATA_MAGIC);

  _mode_t *mode = app->mode;                // need for sa for setting trig delay
  assert(mode && mode->magic == MODE_MAGIC) ;

  cal_t *cal = app->cal;
  assert( cal && cal->magic == CAL_MAGIC);


  printf("cal01\n");

  double values[ 10 ];
  memset(values, 0, sizeof(values));


  // sample off
  gpio_write( app->gpio_trigger, false);

  mode_reset( mode);

  // trigger delay
  sa_trig_delay_set( &mode->sa, period_to_aper_n(  1.f )); // 1 sec.

  // set the dc source voltage
  mode_lts_source_set ( mode, 1.f );

  /////////////////////////////////

  // set the input range to LTS
  app_switch_range1( app, "LTS", "10");

  // must call transition state
  app_transition_state( app);

  app_fill_buffer( app, values, ARRAY_SIZE(values));

  double mean0 = mean( values, ARRAY_SIZE(values));
  UNUSED( mean0);

  printf("mean0 %f\n", mean0);


  /////////////////////////////////

  // set the input range to LTS
  app_switch_range1( app, "LTS", "1");

  // must call transition state
  app_transition_state( app);

  //
  app_fill_buffer( app, values, ARRAY_SIZE(values));

  double mean1 = mean( values, ARRAY_SIZE(values));
  UNUSED( mean1);
  printf("mean1 %f\n", mean1);

  // cal->b = 7.0 / mean( values, ARRAY_SIZE(values));
  cal->b2 = (cal->b * mean0) / mean1 ;

  printf("cal->b2 %f\n", cal->b2 );


  // print some values using the new cal - will use the range_reading convert function
  app_fill_buffer( app, values, ARRAY_SIZE(values));

}

