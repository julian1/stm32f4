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




static void app_override_range( app_t *app)
{
	// consider - change name app_override_range()
  _mode_t *mode = app->mode;
  assert( mode && mode->magic == MODE_MAGIC);


  // isolate external input with relay open
  mode->serial.K402 = SR_RESET;

  // isolate HV divider open/ no 10Meg. impedance
  mode->serial.K403 = SR_RESET;

  // close relay for LTS into dcv and hv divider
  mode->serial.K404 = SR_SET;
}

// rename reference_step()

static void step1( app_t *app)
{
  // setup the reference/target
  printf("\n\n--------\n");
  printf("cal_div1000\n");

  assert( app->cal->b);

  _mode_t *mode = app->mode;
  assert( mode && mode->magic == MODE_MAGIC);

  /* we use the range here. so that data_reading() will print something nominally ok
    but note that calculation will use mean0,mean1 which are normalized counts
    --
    do we need to deal with an EMF offset. when we set a ref voltage. but then transfer
    perhaps it cancels?
    ----
    perhaps we can tell if correct, by checking the offset using an input short, after the div1000 cal
  */

  // reference range
  app_switch_range1( app, "DCV", "10");
  // override
  app_override_range( app);

  // set the source voltage
  mode_lts_source_set ( mode, 10 );

}


static void step2( app_t *app)
{
  _mode_t *mode = app->mode;
  assert( mode && mode->magic == MODE_MAGIC);

  // target range
  app_switch_range1( app, "DCV", "1000");
  // override
  app_override_range( app);
}


static void cal_set_value( cal_t *cal, double mean0, double mean1)
{
  // cal->b = 7.0 / mean( values, ARRAY_SIZE(values));
  // cal->b10 = (cal->b * mean0) / mean1 ;

  cal->div1000 = (cal->b * mean0)  /  mean1;    // the adjustment needed
  printf("cal->div1000 %f\n", cal->div1000 );
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







