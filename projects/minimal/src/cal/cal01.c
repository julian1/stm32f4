

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

  _mode_t *mode = app->mode;                // only for sa structure
  assert(mode && mode->magic == MODE_MAGIC) ;



  // set trigger delay for settle time
  sa_trig_delay_set( &mode->sa, period_to_aper_n(  1.f )); // 1 sec.


  printf("cal01\n");

  double values[ 10 ];
  memset(values, 0, sizeof(values));

  mode_reset( app->mode);

  // we already have the 10V. range.
  // so set LTS. 10. input range.
  app_switch_range1( app, "LTS", "10");


  // set the LTS source. to 1V.
  mode_lts_source_set( app->mode, 1.0 ); // dcv source is ok.

  data->show_counts   = true;
  data->show_reading  = true;

  // start sampling
  gpio_write( app->gpio_trigger, true);

  // obs loop
  for( size_t i = 0; i < ARRAY_SIZE( values);)
  {
    printf("i %u, ", i);

    // wait for adc data
    while( !app->adc_interrupt_valid )
      app_yield( app);

    app->adc_interrupt_valid = false;

    data_update( data);

    if( data->valid ) {

      // No.  measurement of 1V source using 10V. range is accurate - so use reading.

      // values[ i] = data->count_norm * 1.f /*cal->b */;
      values[ i] = data->reading ;
      ++i;
    }

    printf("\n");
  }

  // stop sampling
  gpio_write( app->gpio_trigger, false);





}

