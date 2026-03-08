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
  printf("cal_b10\n");

  assert( app->cal->b);

  // dont need to set az mode, because we are using the range
  // set dc source voltage
  mode_lts_source_set ( app->mode, 1.f );
  // set LTS input range
  app_switch_range1( app, "LTS", "10");
}


static void step2( app_t *app)
{
  app_switch_range1( app, "LTS", "1");
}


static void cal_set_value( cal_t *cal, double mean0, double mean1)
{
  // cal->b = 7.0 / mean( values, ARRAY_SIZE(values));
  cal->b10 = (cal->b * mean0) / mean1 ;
  printf("cal->b10 %f\n", cal->b10 );
}




void app_cal_b10( app_t *app)
{

  transfer_t x = {
    . step1 = step1,
    . step2 = step2,
    . cal_set_value = cal_set_value
  };

  app_transfer( app, &x );
}



