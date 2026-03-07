
/*
  REMEMBER
    - amplifier is picking up lots of noise. from the inductor.
    especially higher ranges.

*/

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

// perhaps change name cal_b10

void app_cal_b10( app_t *app)
{
  assert( app && app->magic == APP_MAGIC);

  data_t *data = app->data;
  assert( data && data->magic == DATA_MAGIC);

  _mode_t *mode = app->mode;                // need for sa for setting trig delay
  assert(mode && mode->magic == MODE_MAGIC) ;

  cal_t *cal = app->cal;
  assert( cal && cal->magic == CAL_MAGIC);

  assert(cal->b);

  printf("--------\n");
  printf("cal_b10\n");

  double values[ 10 ];
  memset(values, 0, sizeof(values));


  app_cal_setup( app);

  // set the dc source voltage
  mode_lts_source_set ( mode, 1.f );

  // set the input range to LTS
  app_switch_range1( app, "LTS", "10");
  app_transition_state( app);

  data->show_reading = true;
  app_fill_buffer( app, values, ARRAY_SIZE(values));
  double mean0 = mean( values, ARRAY_SIZE(values));
  UNUSED( mean0);

  printf("mean0 %f\n", mean0);


  /////////////////////////////////

  // set the input range to LTS
  app_switch_range1( app, "LTS", "1");
  app_transition_state( app);

  //
  data->show_reading = false;
  app_fill_buffer( app, values, ARRAY_SIZE(values));
  double mean1 = mean( values, ARRAY_SIZE(values));
  UNUSED( mean1);
  printf("mean1 %f\n", mean1);

  // cal->b = 7.0 / mean( values, ARRAY_SIZE(values));
  cal->b10 = (cal->b * mean0) / mean1 ;

  printf("cal->b10 %f\n", cal->b10 );


  // print some values to confirm
  data->show_reading = true;
  app_fill_buffer( app, values, ARRAY_SIZE(values));

  app_cal_finish( app);
}

