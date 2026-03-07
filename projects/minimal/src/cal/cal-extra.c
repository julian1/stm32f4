



#include <stdio.h>
#include <assert.h>


#include <lib2/util.h>    // UNUSED

#include <peripheral/gpio.h>        // trigger

#include <app.h>
#include <util.h> // nplc_to_aperture()
#include <mode.h>

#include <data/cal.h>
#include <data/data.h>





void app_cal_all( app_t *app)
{
  app_cal_w( app);
  app_cal_b( app);
  app_cal_b10( app);
  app_cal_b100( app);
  // app_cal_b1000( app);

  cal_show( app->cal);
}


// factor these functions into separate file
// along with cal_fill_buffer() etc.

void app_cal_setup( app_t *app)
{
  // would be better to pass this function as a dependency of the
  // specific cal function. eg. function pointer in a context

  assert(app && app->magic == APP_MAGIC);

  _mode_t *mode = app->mode;
  assert(mode && mode->magic == MODE_MAGIC);


  // sample off
  gpio_write( app->gpio_trigger, false);

  // reset mode
  mode_reset( mode);
  assert( mode->reg_cr.adc_p_active_sigmux == 1 );  // make no unwanted state left behind

  // set the trigger delay for settle time
  sa_trig_delay_set( &mode->sa, period_to_aper_n(  1.f )); // 1 sec.

  // normal sample acquisition/adc operation
  reg_cr_mode_set( &mode->reg_cr, MODE_SA_ADC);

  // set nplc
  adc_aperture_set( &mode->adc, nplc_to_aperture( 10, app->line_freq ));

}


void app_cal_finish( app_t *app)
{
  UNUSED( app);
}




/*
  - using data->count_norm is the most flexible.  independent of range.
  - could pass in the transfer function to use.
  - OR. can just apply the transform on the result.

  - eg. add function to stats.c  to scale the buffer.

*/


void app_fill_buffer( app_t *app, double *values, size_t n)
{
  data_t *data = app->data;
  assert( data && data->magic == DATA_MAGIC);


  // start sampling
  gpio_write( app->gpio_trigger, true);

  // obs loop
  for( unsigned i = 0; i < n; )
  {
    printf("i %u, ", i);

    // wait for adc data
    while( !app->adc_interrupt_valid )
      app_yield( app);

    app->adc_interrupt_valid = false;


    data_update( data);
    if( data->valid) {

      values[ i] = data->count_norm;
      ++i;
    }

    printf("\n");
  }

  // stop sampling
  gpio_write( app->gpio_trigger, false);
}




void app_fill_buffer1( app_t *app, double *pos_values, double *neg_values, size_t n)
{
  data_t *data = app->data;
  assert( data && data->magic == DATA_MAGIC);


  // start sampling
  gpio_write( app->gpio_trigger, true);

  // take obs loop
  for( size_t i = 0; i < n; )
  {
    printf("i %u, ", i);

    // wait for adc data
    while( !app->adc_interrupt_valid )
      app_yield( app);

    app->adc_interrupt_valid = false;

    data_update( data);

    // take both hi and lo readings since they are the same.
    // ignore data->valid

    pos_values[i] = data->clk_count_refmux_pos;
    neg_values[i] = data->clk_count_refmux_neg;

    ++i;

    printf("\n");
  }

  // sampling off
  gpio_write( app->gpio_trigger, false);

}


