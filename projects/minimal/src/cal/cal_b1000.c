

/*
  REMEMBER
    - amplifier is picking up lots of smps noise. from the inductor.
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
  the this entire. thing can be done.
  with just the lts_source, and LTS range arguments  and b1000
  being set.

  So instead of factoring to simplify the set_up() function.
  could factor into three functions.

  step1_setup()
  step2_setup()
  set_value(  cal, mean0, mean1, )
*/

static void step1( app_t *app)
{
  assert( app->cal->b100);
  mode_lts_source_set ( app->mode, 0.01 );
  app_switch_range1( app, "LTS", "0.1");
}

static void step2( app_t *app)
{
  app_switch_range1( app, "LTS", "0.01");
}

static void cal_set_value( cal_t *cal, double mean0, double mean1)
{
  cal->b1000 = (cal->b100 * mean0) / mean1;
}


typedef struct transfer10
{
  void (*step1)( app_t *app);
  void (*step2)( app_t *app);
  void (*set_value)( cal_t *cal, double mean0, double mean1);

} transfer10;





void app_cal_b1000( app_t *app)
{
  data_t *data = app->data;
  _mode_t *mode = app->mode;
  cal_t *cal = app->cal;

  printf("--------\n");
  printf("cal_b1000\n");

  assert(cal->b100);

  app_cal_setup( app);

  // set the dc source voltage
  mode_lts_source_set ( mode, 0.01 );     // this fails

  // set the input range to LTS
  app_switch_range1( app, "LTS", "0.1");

  app_transition_state( app);

  double values[ 10 ];
  memset(values, 0, sizeof(values));

  data->show_reading = true;
  app_fill_buffer( app, values, ARRAY_SIZE(values));
  double mean0 = mean( values, ARRAY_SIZE(values));
  UNUSED( mean0);

  printf("mean0 %f\n", mean0);


  /////////////////////////////////

  // set the input range to LTS
  app_switch_range1( app, "LTS", "0.01");
  app_transition_state( app);

  //
  data->show_reading = false;
  app_fill_buffer( app, values, ARRAY_SIZE(values));
  double mean1 = mean( values, ARRAY_SIZE(values));
  UNUSED( mean1);
  printf("mean1 %f\n", mean1);

  // cal->b = 7.0 / mean( values, ARRAY_SIZE(values));
  cal->b1000 = (cal->b100 * mean0) / mean1 ;

  printf("cal->b100 %f\n", cal->b1000 );


  // show some values to confirm
  data->show_reading = true;
  app_fill_buffer( app, values, ARRAY_SIZE(values));

  app_cal_finish( app);
}


