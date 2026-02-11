
/*
  Feb 2026.

  Can we create a calibration with just a ratio term.

  - no scale, offset.   so cannot compute noise
  - assume that parasitic/ leakage etc. will be constant. across az.

  by just setting the current ratio do not even need a value.

*/

#include <stdio.h>
#include <assert.h>
// #include <math.h>     // fabs




#include <peripheral/gpio.h>
#include <peripheral/spi-ice40.h>
#include <peripheral/interrupt.h>



#include <lib2/util.h>    // yield_with_msleep
#include <lib2/stats.h>
#include <lib2/format.h>  // format_with_commas


#include <mode.h>

#include <util.h> // nplc_to_aperture()

#include <app.h>
#include <data/data.h>



// #include <data/matrix.h> // m_set_row()
// #include <data/regression.h>






void app_cal2(

  app_t *app
)
{


  data_t    *data = app->data;
  _mode_t *mode = app->mode;

  assert(data);
  assert(data->magic == DATA_MAGIC) ;
  assert(mode);
  assert(mode->magic == MODE_MAGIC) ;


  spi_t *spi_fpga0  = app->spi_fpga0;
  assert(spi_fpga0);


  printf("whoot app_cal2() \n");




  // we are not doing hi/lo herer
  double values[ 10 ];


 // let things settle from spi emi burst, and board DA settle, amp to come out of lockup.
  // equivalent to discarding values

  // trig off
  gpio_write( app->gpio_trigger_internal, 0 );


  mode_reset( mode);

  mode_reg_cr_set( mode, MODE_SA_ADC);    // set fpga reg_mode.

  // setup adc nplc
  mode->adc.p_aperture = nplc_to_aperture( 10, data->line_freq );				// fix jul 2024.

  mode_sa_set(mode, "0" );      // special sample acquisition.  for adc running standalone.


  mode->reg_cr.adc_p_active_sigmux = 0;   // dont turn on sigmux


  // write board state
  printf("spi_mode_transition_state()\n");
  // spi_mode_transition_state( devices, mode, system_millis);
  app_transition_state( app);


  printf("sleep\n");
  // yield_with_msleep( 1 * 1000, system_millis, yield, yield_ctx);
  yield_with_msleep( 1 * 1000, &app->system_millis, (void (*)(void *))app_update_simple_led_blink, app);


  // check magic.
  assert( app->fpga0_interrupt->magic == 789);

  // trig on - should add accessor ...
  gpio_write( app->gpio_trigger_internal, 1 );


  // take obs loop
  for(unsigned i = 0; i < ARRAY_SIZE(values); ++i)
  {

    printf("i %u\n", i);


    // wait for adc data, on interrupt
    // use express yield function here. not app->yield etc
    while( !app->adc_interrupt_valid )
      app_update_simple_led_blink( app);

    app->adc_interrupt_valid = false;


    // embed a 8 bit. counter ini the reg_status and use it for the measure.

    uint32_t status_ = spi_ice40_reg_read32( spi_fpga0, REG_STATUS );
     // error: dereferencing type-punned pointer will break strict-aliasing rules [-Werror=strict-aliasing]
    // reg_sr_t  status = * (reg_sr_t*)((void *) &status_);  // gives error

    reg_sr_t  status;
     _Static_assert(sizeof(status) == sizeof(status_), "bad typedef size");

    memcpy( &status, &status_,  sizeof( status_));

    printf("  first=%u  idx=%u seq_n=%u\n",
      status.first,
      status.sample_idx,
      status.sample_seq_n
    );


    uint32_t clk_count_rstmux       = spi_ice40_reg_read32( spi_fpga0, REG_ADC_CLK_COUNT_RSTMUX);    // useful check.
    uint32_t clk_count_refmux_neg   = spi_ice40_reg_read32( spi_fpga0, REG_ADC_CLK_COUNT_REFMUX_NEG);
    uint32_t clk_count_refmux_pos   = spi_ice40_reg_read32( spi_fpga0, REG_ADC_CLK_COUNT_REFMUX_POS);
    uint32_t clk_count_refmux_both  = spi_ice40_reg_read32( spi_fpga0, REG_ADC_CLK_COUNT_REFMUX_BOTH);   // check.
    uint32_t clk_count_sigmux       = spi_ice40_reg_read32( spi_fpga0, REG_ADC_CLK_COUNT_SIGMUX);
    uint32_t clk_count_aperture     = spi_ice40_reg_read32( spi_fpga0, REG_ADC_CLK_COUNT_APERTURE);     // check.


    printf("  counts %6lu %lu %lu %lu %lu %6lu",
      clk_count_rstmux,
      clk_count_aperture,
      clk_count_sigmux,
      clk_count_refmux_neg,
      clk_count_refmux_pos,
      clk_count_refmux_both
    );
    printf("\n");


    double w =  (double) (clk_count_refmux_pos  )  /   clk_count_refmux_neg  ;

    printf("  w %.8f", w );
    values[ i ] = w;


    printf("\n");

    /*
      w is independent of aperture/nplc.
      and independent of a constant leakage.  actually no.   will be cancelled.  when do proper AZ.

      a constant current leakage is same as as having an input signal.
    */

  }

  // trig off
  gpio_write( app->gpio_trigger_internal, 0 );


  double mean_   = mean(   values, ARRAY_SIZE(values));     // should prefix functions stats_mean ?
  double stddev_ = stddev( values, ARRAY_SIZE(values));
  // printf( "mean   %.8f\n", mean_ );
  // printf( "stddev %.10f\n", stddev_);


  char buf[100 + 1];
  printf( "mean   %s\n", str_format_float_with_commas(buf, 100, 9, mean_));
  printf( "stddev %s\n", str_format_float_with_commas(buf, 100, 9, stddev_));
;


  // A 4-byte float (IEEE 754 single-precision) is accurate to approximately 7 decimal digits
  // so should really use double prec array.
  // perhaps move stats. from lib2. to change.



}
