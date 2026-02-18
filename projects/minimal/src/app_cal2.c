
/*
  Feb 2026.

  Can we create a calibration with just a ratio term.

  - no scale, offset.   so cannot compute noise
  - assume that parasitic/ leakage etc. will be constant. across az.

  by just setting the current ratio do not even need a value.

*/


/*
  EXTR.

    EXTR.
        - Our problem with the integration - when using 50,50,50k current resistors.
          having very high ratios at +-11V.  can be fixed by simply using asymetrical VAR timing.
          just need separate parameters for plus var, and min-var. +ve

    EXTR.
      it is simple to inject varying/dithering current by changing the var values.
      don't have to do it at the end, with different cycles.

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

  char buf[100 + 1];


  assert(data);
  assert(data->magic == DATA_MAGIC) ;
  assert(mode);
  assert(mode->magic == MODE_MAGIC) ;


  spi_t *spi = app->spi_fpga0;
  assert(spi);


  printf("whoot app_cal2() \n");


  // we are not doing hi/lo herer
  double values[ 10];


 // let things settle from spi emi burst, and board DA settle, amp to come out of lockup.
  // equivalent to discarding values

  // asmplifing off
  app_trigger( app, false);


  mode_reset( mode);

  // normal sample acquisition/adc operation
  mode_reg_cr_set( mode, MODE_SA_ADC);


  // sample acquisition mode - for adc running standalone.  // REVIEW ME
  mode_sa_set(mode, "0" );


  mode->reg_cr.adc_p_active_sigmux = 0;   // sigmux not active.



  for(unsigned k = 1; k <= 10; ++k) {

    // sampling off
    app_trigger( app, false);

    // set nplc
    mode->adc.p_aperture = nplc_to_aperture( k, data->line_freq );				// fix jul 2024.

    // printf("spi_mode_transition_state()\n");
    app_transition_state( app);

    // printf("sleep\n");
    yield_with_msleep( 1 * 1000, &app->system_millis, (void (*)(void *))app_update_simple_led_blink, app);

    // start sampling
    app_trigger( app, true);


    printf("nplc %u\n", k);

    // take obs loop
    for(unsigned i = 0; i < ARRAY_SIZE(values); ++i)
    {

      printf("i %u, ", i);


      // wait for adc data, on interrupt
      // use express yield function here. not app->yield etc
      while( !app->adc_interrupt_valid )
        app_update_simple_led_blink( app);

      app->adc_interrupt_valid = false;


      // embed a 8 bit. counter ini the reg_status and use it for the measure.

      uint32_t status_ = spi_ice40_reg_read32( spi, REG_STATUS );
       // error: dereferencing type-punned pointer will break strict-aliasing rules [-Werror=strict-aliasing]
      // reg_sr_t  status = * (reg_sr_t*)((void *) &status_);  // gives error

      reg_sr_t  status;
       _Static_assert(sizeof(status) == sizeof(status_), "bad typedef size");

      memcpy( &status, &status_,  sizeof( status_));

      printf("  first=%u  idx=%u seq_n=%u, ",
        status.first,
        status.sample_idx,
        status.sample_seq_n
      );

      // uint32_t clk_count_rstmux       = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_RSTMUX);    // useful check.
      uint32_t clk_count_refmux_neg   = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_NEG);
      uint32_t clk_count_refmux_pos   = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_POS);
      // uint32_t clk_count_refmux_both  = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_BOTH);   // check.
      // uint32_t clk_count_sigmux       = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_SIGMUX);
      // uint32_t clk_count_aperture     = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_APERTURE);     // check.

      double w =  (double) (clk_count_refmux_pos  )  /   clk_count_refmux_neg  ;
      values[ i ] = w;


      // printf(" w %.8f, ", w );
      printf( "w %s ", str_format_float_with_commas(buf, 100, 9, w));

      /*
          printf("  counts %6lu %lu %lu %lu %lu %6lu",
            clk_count_rstmux,
            clk_count_aperture,
            clk_count_sigmux,
            clk_count_refmux_neg,
            clk_count_refmux_pos,
            clk_count_refmux_both
          );
          printf("\n");
      */

      printf("\n");
    }

    // trig off
    app_trigger( app, false);


    double mean_   = mean(   values, ARRAY_SIZE(values));     // should prefix functions stats_mean ?
    double stddev_ = stddev( values, ARRAY_SIZE(values));
    // printf( "mean   %.8f\n", mean_ );
    // printf( "stddev %.10f\n", stddev_);


    printf("%u ", k );
    printf("(n %u) ", ARRAY_SIZE(values));
    printf( "mean   %s ", str_format_float_with_commas(buf, 100, 9, mean_));
    printf( "stddev %s", str_format_float_with_commas(buf, 100, 9, stddev_));

    printf("\n");

  }


}


/*
    --------------------
    EXTR. should this  - do it once.  in data.
    ie. localize in one place.

    eg. because we call cal with app context.n
    we can call.

    app_update()
      data->update();     to do the spi reading.
      buffers->update()   if we want to.
      etc.

    and relying  on the 'first' flag to clear the buffers. if needed.

*/



/*
  w is independent of aperture/nplc.
  and independent of a constant leakage.  actually no.   will be cancelled.  when do proper AZ.
  a constant current leakage is same as as having an input signal.
*/

  // A 4-byte float (IEEE 754 single-precision) is accurate to approximately 7 decimal digits
  // so should really use double prec array.
  // perhaps move stats. from lib2. to change.


