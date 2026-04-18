/*
  REMEMBER
    - amplifier is picking up lots of smps noise. from the inductor.
    especially higher ranges.

  // could use an array of all these functions.
*/

#include <stdio.h>
#include <assert.h>

#include <lib2/util.h>    // UNUSED

#include <cal/transfer.h>
#include <app.h>
#include <ranging.h>
#include <mode.h>
#include <data/cal.h>




static void mode_override_range( _mode_t *mode)
{
  // isolate external input using open input relay
  mode->serial.K402 = SR_RESET;

  // isolate HV divider open/ no 10Meg. impedance
  mode->serial.K403 = SR_RESET;

  // close relay for LTS into dcv and hv divider
  mode->serial.K404 = SR_SET;
}


static void step1( app_t *app)
{
  // setup the reference/target
  printf("\n\n--------\n");
  printf("cal_div100\n");

  assert( app->cal->b);   // for reference dcv10
  assert( app->cal->b10); // for target dcv100

  // reference range
  ranging_range_switch1( app->ranging, "DCV", "10");
  mode_override_range( app->mode);

  // set lts source voltage
  mode_lts_source_set ( app->mode, 10);
}


static void step2( app_t *app)
{
  // target range
  ranging_range_switch1( app->ranging, "DCV", "100");
  mode_override_range( app->mode);
}


static void cal_set_value( cal_t *cal, double mean0, double mean1)
{
  // note. b (not b10) used for the reference
  cal->div100 = (cal->b * mean0)  /  mean1;    // adjustment needed?
  printf("cal->div100 %f\n", cal->div100 );
}


void app_cal_div100( app_t *app)
{

  transfer_t x = {
    // .name = "div100",
    . step1 = step1,
    . step2 = step2,
    . cal_set_value = cal_set_value
  };

  app_transfer( app, &x );
}







