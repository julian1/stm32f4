/*
  REMEMBER
    - amplifier is picking up lots of smps noise. from the inductor.
    especially higher ranges.

  // could use an array of all these functions.
*/

#include <stdio.h>
#include <assert.h>

#include <lib2/util.h>    // UNUSED

#include <app.h>
#include <mode.h>
#include <data/cal.h>






static void step1( app_t *app)
{
  // setup the reference/target
  printf("\n\n--------\n");
  printf("cal_div1000\n");

  assert( app->cal->b);

  _mode_t *mode = app->mode;
  assert( mode && mode->magic == MODE_MAGIC);

  /* set the reference - measure lts 10V. on ch1. input
    easier to do this manually rather than with ranges.
    this is all com-lc referenced.
    do not call mode_reset() here... because adc/sa parameters have been set.
  */

  mode_lts_source_set ( mode, 10 );

  // close the relay to send lts into dcv ch1.  and the hv divider
  mode->serial.K404 = SR_SET;

  mode_az_set( mode, "ch1" );

  // is anything else needed dcv ?

  // set the ch2. input up, ready for step 2.
  mode_ch2_set( app->mode, "div");

}


static void step2( app_t *app)
{
  // switch to ch2.
  mode_az_set( app->mode, "ch2" );

}


static void cal_set_value( cal_t *cal, double mean0, double mean1)
{
  UNUSED(mean0);
  UNUSED(mean1);
  // cal->b = 7.0 / mean( values, ARRAY_SIZE(values));
  // cal->b10 = (cal->b * mean0) / mean1 ;

  printf("target %f\n" ,   cal->b * mean0 );   // should equal 9.8V. etc

  cal->div1000 = (cal->b * mean0)  /  mean1;

  printf("cal->div1000 %f\n", cal->div1000 );


  /* need to set the range here in order for data_update()
    to have the div1000 available to set correct value...

    but cannot because do not have reference to app_t.
    AND because.... we do not want the input relay closed.

    could set up a dummy range.  dcv1000 - internal...
  */

  app_switch_range1( app, "dcv", "1000");

}


// actually i think we are doing div10000.  eg. when there is no gain.

// if we have not got a range set, then the printing to use cal-.


void app_cal_div1000( app_t *app)
{

  transfer_t x = {
    . step1 = step1,
    . step2 = step2,
    . cal_set_value = cal_set_value
  };

  app_transfer( app, &x );
}







