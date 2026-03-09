/*
  REMEMBER
    - amplifier is picking up lots of smps noise. from the inductor.
    especially higher ranges.

  // could use an array of all these functions.
*/

#include <stdio.h>
#include <assert.h>

#include <app.h>
#include <mode.h>
#include <data/cal.h>






static void step1( app_t *app)
{
  printf("\n\n--------\n");
  printf("cal_b100\n");

  assert( app->cal->b10);

  // dont need to set az mode, because we are using the range
  // set dc source voltage
  mode_lts_source_set ( app->mode, 0.1 );
  // reference range
  app_switch_range1( app, "LTS", "1");
}


static void step2( app_t *app)
{
  // target range
  app_switch_range1( app, "LTS", "0.1");
}


static void cal_set_value( cal_t *cal, double mean0, double mean1)
{
  // cal->b = 7.0 / mean( values, ARRAY_SIZE(values));
  cal->b100 = (cal->b10 * mean0) / mean1 ;
  printf("cal->b100 %f\n", cal->b100 );
}




void app_cal_b100( app_t *app)
{

  transfer_t x = {
    . step1 = step1,
    . step2 = step2,
    . cal_set_value = cal_set_value
  };

  app_transfer( app, &x );
}




