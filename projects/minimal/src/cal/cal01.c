

#include <stdio.h>
#include <string.h>
#include <assert.h>



#include <peripheral/gpio.h>        // trigger

#include <util.h>
#include <app.h>
#include <mode.h>
#include <lib2/util.h>    // ARRAY_SIZE

#include <data/data.h>


/*

  questions
    1. whether to work with cal_t structure directly.
        or communicate back values - through the range structure.
        - communicating back.  - can add/ range specific bounds checks. etc.
        - but this code is already range / cal specific.

        BUT we really only want to set amplifier gain once.

    2. whether to have the gain ranges - scale according to the 10V. range b.  or else directly from adc adjusted_sum.

*/


void app_cal_01( app_t *app)
{
  assert( app && app->magic == APP_MAGIC);

  data_t *data = app->data;
  assert( data && data->magic == DATA_MAGIC);

  _mode_t *mode = app->mode;                // only for sa for setting trig delay
  assert(mode && mode->magic == MODE_MAGIC) ;

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

  // set the input range to LTS.
  app_switch_range1( app, "LTS", "10");     // overriding some state?

  // must call transition state.
  app_transition_state( app);


  // start sampling
  gpio_write( app->gpio_trigger, true);

  // take obs loop
  for( unsigned i = 0; i < ARRAY_SIZE(values); ++i)
  {
    printf("i %u, ", i);

    // wait for adc data
    while( !app->adc_interrupt_valid )
      app_yield( app);

    app->adc_interrupt_valid = false;

    data_update( data);

    printf("\n");
  }

  // stop sampling
  gpio_write( app->gpio_trigger, false);


}

