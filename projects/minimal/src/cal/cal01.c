

#include <stdio.h>
#include <string.h>
#include <assert.h>



#include <peripheral/gpio.h>        // trigger manipulation

#include <app.h>
#include <mode.h>
#include <lib2/util.h>    // ARRAY_SIZE

#include <data/data.h>




void app_cal_01( app_t *app)
{

  data_t *data = app->data;
  assert( data && data->magic == DATA_MAGIC);



  printf("cal01\n");

  double values[ 10 ];
  memset(values, 0, sizeof(values));

  mode_reset( app->mode);

  // we already have the 10V. range.
  // so set LTS. 10. input range.
  app_switch_range1( app, "LTS", "10");


  // set the LTS source. to 1V.
  mode_lts_source_set( app->mode, 1.0 );

  data->show_counts   = true;
  data->show_reading  = true;

  // start sampling
  gpio_write( app->gpio_trigger, true);


  // take obs loop
  for( unsigned i = 0; i < ARRAY_SIZE( values);)
  {
    printf("i %u, ", i);

    // wait for adc data
    while( !app->adc_interrupt_valid )
      app_yield( app);

    app->adc_interrupt_valid = false;

    data_update( data);

    if( data->valid ) {

      values[ i] = data->count_norm * 1.f /*cal->b */;
      ++i;
    }

    printf("\n");
  }




}

