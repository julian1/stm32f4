


#include <stdio.h>
#include <assert.h>
#include <math.h>     // NAN


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







static void test( app_t *app)
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



  // sample off
  app_trigger( app, false);

  mode_reset( mode);

  // normal sample acquisition/adc operation
  mode_reg_cr_set( mode, MODE_SA_ADC);


  // sample acquisition mode - for adc running standalone.  // REVIEW ME
  mode_sa_set(mode, "0" );


  mode->reg_cr.adc_p_active_sigmux = 0;   // sigmux not active.





  /////////////////////////

  double w = 0;
  uint32_t w_clk_count_aperture = 0;


  {
    // need double for mean()
    double pos_values[ 10 ];
    double neg_values[ 10 ];

    _Static_assert(ARRAY_SIZE(pos_values) == ARRAY_SIZE(neg_values), "whoot");

    // stop sampling
    app_trigger( app, false);

    // nplc to use
    mode->adc.p_aperture = nplc_to_aperture( 1 , data->line_freq );
    app_transition_state( app);
    // sleep
    yield_with_msleep( 1 * 1000, &app->system_millis, (void (*)(void *))app_update_simple_led_blink, app);
    // start sampling
    app_trigger( app, true);


    // take obs loop
    for(unsigned i = 0; i < ARRAY_SIZE(pos_values); ++i)
    {
      printf("i %u, ", i);

      // wait for adc data, on interrupt
      // use express yield function here. not app->yield etc
      while( !app->adc_interrupt_valid )
        app_update_simple_led_blink( app);

      app->adc_interrupt_valid = false;

      uint32_t status_ = spi_ice40_reg_read32( spi, REG_STATUS );
      reg_sr_t  status;
       _Static_assert(sizeof(status) == sizeof(status_), "bad typedef size");
      memcpy( &status, &status_,  sizeof( status_));

      printf("  first=%u  idx=%u seq_n=%u, ", status.first, status.sample_idx, status.sample_seq_n);

      uint32_t clk_count_refmux_pos   = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_POS);
      uint32_t clk_count_refmux_neg   = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_NEG);

      w_clk_count_aperture            = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_APERTURE);     // check.

      printf("  counts pos %lu neg %lu", clk_count_refmux_pos, clk_count_refmux_neg);

      pos_values[i] = clk_count_refmux_pos;
      neg_values[i] = clk_count_refmux_neg;

      printf("\n");
    }
    // trig off
    app_trigger( app, false);


    double mean_pos   = mean( pos_values, ARRAY_SIZE(pos_values));
    double mean_neg   = mean( neg_values, ARRAY_SIZE(neg_values));
    double stddev_pos = stddev( pos_values, ARRAY_SIZE(pos_values));
    double stddev_neg = stddev( neg_values, ARRAY_SIZE(neg_values));

    printf( "pos mean   %s\n",  str_format_float_with_commas(buf, 100, 9, mean_pos));
    printf( "pos stddev %s\n",  str_format_float_with_commas(buf, 100, 9, stddev_pos));
    printf( "neg mean   %s\n",  str_format_float_with_commas(buf, 100, 9, mean_neg));
    printf( "neg stddev %s\n",  str_format_float_with_commas(buf, 100, 9, stddev_neg));

    printf( "aperture   %lu\n",  w_clk_count_aperture);


    // set w
    w =  mean_pos / mean_neg;

  }

  // printf(" w %.8f, ", w );
  printf( "w %s\n", str_format_float_with_commas(buf, 100, 9, w));


  assert( w);



    ////////////////////////




  // unsigned nplc_[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10  };

  // we are not doing hi/lo herer
  double values[ 10];

  // loop nplc
  // for(unsigned k = 0; k < ARRAY_SIZE(nplc_); ++k) {
  for(unsigned nplc = 1; nplc < 11; ++nplc) {

    printf("nplc %u\n", nplc);

    // sampling off
    app_trigger( app, false);

    // set nplc
    mode->adc.p_aperture = nplc_to_aperture( nplc, data->line_freq );				// fix jul 2024.
    app_transition_state( app);
    // sleep
    yield_with_msleep( 1 * 1000, &app->system_millis, (void (*)(void *))app_update_simple_led_blink, app);
    // start sampling
    app_trigger( app, true);

    uint32_t clk_count_aperture  ;

    // take obs loop
    for(unsigned i = 0; i < ARRAY_SIZE(values); ++i)
    {
      printf("i %u, ", i);


      // wait for adc data, on interrupt
      // use express yield function here. not app->yield etc
      while( !app->adc_interrupt_valid )
        app_update_simple_led_blink( app);

      app->adc_interrupt_valid = false;

      uint32_t status_ = spi_ice40_reg_read32( spi, REG_STATUS );
      reg_sr_t  status;
       _Static_assert(sizeof(status) == sizeof(status_), "bad typedef size");
      memcpy( &status, &status_,  sizeof( status_));

      printf("  first=%u  idx=%u seq_n=%u, ", status.first, status.sample_idx, status.sample_seq_n);

      // uint32_t clk_count_rstmux       = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_RSTMUX);    // useful check.
      uint32_t clk_count_refmux_pos   = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_POS);
      uint32_t clk_count_refmux_neg   = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_NEG);
      // uint32_t clk_count_refmux_both  = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_BOTH);   // check.
      // uint32_t clk_count_sigmux       = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_SIGMUX);
      clk_count_aperture              = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_APERTURE);     // check.


      printf(" pos %lu neg %lu, ", clk_count_refmux_pos, clk_count_refmux_neg);


      /*
          the difference value increases - BUT remember we will divide by the aperture.
      */

      // difference in weighted.

      double neg_w = clk_count_refmux_neg  * w;
      double v =  (double)clk_count_refmux_pos  - neg_w;
      values[ i ] = v;
      printf( "neg_w %.3f,  v %.3f ", neg_w , v );


      printf("\n");
    }

    // trig off
    app_trigger( app, false);


    double mean_   = mean(   values, ARRAY_SIZE(values));     // should prefix functions stats_mean ?
    double stddev_ = stddev( values, ARRAY_SIZE(values));

    printf("(n %u) ", ARRAY_SIZE(values));
    printf( "mean   %.3f ",  mean_);
    printf( "stddev %.3f ",  stddev_);


    printf( " mean ap. adj %.3f ",  mean_  / clk_count_aperture * w_clk_count_aperture);

    printf("\n");

  }


}



bool app_test51(
  app_t *app,
  const char *cmd
) {
  assert(app);
  assert(app->magic == APP_MAGIC);
  assert(cmd);

  if( strcmp(cmd, "test51") == 0) {

    printf("test51()\n");
    test( app);
    return 1;
  }

  return 0;
}



