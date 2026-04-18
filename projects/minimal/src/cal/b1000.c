/*
  REMEMBER
    - amplifier is picking up lots of smps noise. from the inductor.
    especially higher ranges.

  // could use an array of all these functions.
*/

#include <stdio.h>
#include <assert.h>

#include <cal/transfer.h>
#include <app.h>
#include <ranging.h>
#include <mode.h>
#include <data/cal.h>






static void step1( app_t *app)
{
  printf("\n\n--------\n");
  printf("cal_b1000\n");

  assert( app->cal->b100);

  // dont need to set az mode, because we are using the range
  // set dc source voltage
  mode_lts_source_set ( app->mode, 0.01 );
  // set LTS input range
  ranging_range_switch1( app->ranging, "LTS", "0.1");
}


static void step2( app_t *app)
{
  ranging_range_switch1( app->ranging, "LTS", "0.01");
}


static void cal_set_value( cal_t *cal, double mean0, double mean1)
{
  // values are count_norm
  // cal->b = 7.0 / mean( values, ARRAY_SIZE(values));
  cal->b1000 = (cal->b100 * mean0) / mean1 ;
  printf("cal->b1000 %f\n", cal->b1000 );
}




void app_cal_b1000( app_t *app)
{

  transfer_t x = {
    . step1 = step1,
    . step2 = step2,
    . cal_set_value = cal_set_value
  };

  app_transfer( app, &x );
}





