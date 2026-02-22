


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
  mode_az_set(mode, "0" );

  // REVIWE should not need this....
  mode_gain_set(mode, 1);

  // hold input to adc at lo. to reduce leakage.
  mode_ch2_set_ref_lo( mode);


  // sigmux not active. for initial weight.
  mode->reg_cr.adc_p_active_sigmux = 0;


  /////////////////////////

  unsigned nplc = 10;

  // cal stuff
  double w = 0;
  // uint32_t w_clk_count_aperture = 0;

  {
    // need double for mean()
    double pos_values[ 10 ];
    double neg_values[ 10 ];

    _Static_assert(ARRAY_SIZE(pos_values) == ARRAY_SIZE(neg_values), "whoot");

    // stop sampling
    app_trigger( app, false);

    // nplc to use
    mode->adc.p_aperture = nplc_to_aperture( nplc , data->line_freq );
    app_transition_state( app);
    // sleep
    yield_with_msleep( 1 * 1000, &app->system_millis, (void (*)(void *))app_update_simple_led_blink, app);
    // start sampling
    app_trigger( app, true);


    /* we can run this loop more simply
        just sum up the values.
        no need for mean function etc...  although having the stddev is useful.

    */

    // take obs loop
    for(unsigned i = 0; i < ARRAY_SIZE(pos_values); ++i)
    {
      printf("i %u, ", i);

      // wait for adc data
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
      uint32_t clk_count_sigmux     = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_SIGMUX );

      // w_clk_count_aperture            = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_APERTURE);

      printf("counts pos %7lu neg %7lu, ", clk_count_refmux_pos, clk_count_refmux_neg);
      printf("sigmux %7lu, ", clk_count_sigmux);



      pos_values[i] = clk_count_refmux_pos;
      neg_values[i] = clk_count_refmux_neg;

      printf("\n");
    }
    // trig off
    app_trigger( app, false);


    double pos_mean   = mean(   pos_values, ARRAY_SIZE(pos_values));
    double neg_mean   = mean(   neg_values, ARRAY_SIZE(neg_values));
    double stddev_pos = stddev( pos_values, ARRAY_SIZE(pos_values));
    double stddev_neg = stddev( neg_values, ARRAY_SIZE(neg_values));

    printf( "pos mean   %.3f, ", pos_mean);
    printf( "stddev %.3f, ", stddev_pos);
    printf("\n");

    printf( "neg mean   %.3f, ", neg_mean);
    printf( "stddev %.3f, ", stddev_neg);
    printf("\n");

    // set w
    w =  pos_mean / neg_mean;

  }
  // sampling off
  app_trigger( app, false);


  // printf(" w %.8f, ", w );
  printf( "w %s\n", str_format_float_with_commas(buf, 100, 9, w));

  assert( w);





  ////////////////////////
  // may need larger int ....
  // for long
  // better name clk_count_refmux_net
  // int32_t w_clk_count_refmux = 0;     // MUST BE SIGNED.
  // uint32_t w_clk_count_aperture  = 0 ;

    ////////
  // only record the last lo.
  uint32_t clk_count_refmux_pos_lo = 0;     // this is adjusted...
  uint32_t clk_count_refmux_neg_lo = 0;

  // better name. count divisor/ factor.
  double values[ 10 ];
  memset(values, 0, sizeof(values));

  {
    // nplc
    mode->adc.p_aperture = nplc_to_aperture( nplc, data->line_freq );

    // calibrate against ref
    mode_ch2_set_ref( mode);

    mode_az_set(mode, "ch2" );

    // sigmux active
    mode->reg_cr.adc_p_active_sigmux = 1;

    app_transition_state( app);


    // sleep
    yield_with_msleep( 1 * 1000, &app->system_millis, (void (*)(void *))app_update_simple_led_blink, app);
    // start sampling
    app_trigger( app, true);


    // compute ref for diff
    for(unsigned i = 0; i < ARRAY_SIZE( values);)
    {
      printf("i %u, ", i);      // two readings per value...

      // wait for adc data
      // use express yield function here. not app->yield etc
      while( !app->adc_interrupt_valid )
        app_update_simple_led_blink( app);

      app->adc_interrupt_valid = false;

      uint32_t status_ = spi_ice40_reg_read32( spi, REG_STATUS );
      reg_sr_t  status;
       _Static_assert(sizeof(status) == sizeof(status_), "bad typedef size");
      memcpy( &status, &status_,  sizeof( status_));

      uint32_t clk_count_refmux_pos = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_POS);
      uint32_t clk_count_refmux_neg = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_NEG);
      // uint32_t clk_count_aperture   = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_APERTURE);
      uint32_t clk_count_sigmux     = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_SIGMUX );

      printf("first=%u  idx=%u seq_n=%u, ", status.first, status.sample_idx, status.sample_seq_n);
      printf("counts pos %7lu neg %7lu, ", clk_count_refmux_pos, clk_count_refmux_neg);
      printf("sigmux %7lu, ", clk_count_sigmux);

      if(status.sample_idx == 0) {
        // lo - record counts
        clk_count_refmux_pos_lo = clk_count_refmux_pos;
        clk_count_refmux_neg_lo = clk_count_refmux_neg;
      }
      else if (status.sample_idx == 1) {
        // hi
        double v = ((double) clk_count_refmux_pos - (w * clk_count_refmux_neg))
                - ( (double) clk_count_refmux_pos_lo  - (w * clk_count_refmux_neg_lo));

        printf("v %f, ", v );
        values[ i ] = v;
        // only increment on hi.
        ++i;
      }
      else
        assert(0);

      printf("\n");
    }

    // stop sampling
    app_trigger( app, false);

    // printf( "w_clk_count_refmux   %ld\n", w_clk_count_refmux);
    // printf( "w_clk_count_aperture %lu\n", w_clk_count_aperture);
  }


  double values_mean = mean(   values, ARRAY_SIZE(values));
  double values_stddev = stddev( values, ARRAY_SIZE(values));

  printf( "mean   %.3f, ", values_mean );
  printf( "stddev %.3f, ", values_stddev);
  printf("\n");




  ////////////////////////

  memset(values, 0, sizeof(values));

    // mode_ch2_set_ref( mode);
    // mode_ch2_set_ref_lo( mode);
    // mode_az_set(mode, "ch2" );

    // set 10V.
    // mode_lts_set( mode, 10 );
    // mode_ch2_set_lts( mode);

    app_transition_state( app);

    // sleep
    yield_with_msleep( 1 * 1000, &app->system_millis, (void (*)(void *))app_update_simple_led_blink, app);

    // start sampling
    app_trigger( app, true);

    // take obs loop
    for(unsigned i = 0; i < ARRAY_SIZE( values);)
    {
      printf("i %u, ", i);

      // wait for adc data
      // use express yield function here. not app->yield etc
      while( !app->adc_interrupt_valid )
        app_update_simple_led_blink( app);

      app->adc_interrupt_valid = false;

      uint32_t status_ = spi_ice40_reg_read32( spi, REG_STATUS );
      reg_sr_t  status;
       _Static_assert(sizeof(status) == sizeof(status_), "bad typedef size");
      memcpy( &status, &status_,  sizeof( status_));

      uint32_t clk_count_refmux_pos = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_POS);
      uint32_t clk_count_refmux_neg = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_NEG);
      // uint32_t clk_count_aperture   = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_APERTURE);
      uint32_t clk_count_sigmux     = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_SIGMUX );

      printf("first=%u  idx=%u seq_n=%u, ", status.first, status.sample_idx, status.sample_seq_n);
      printf("counts pos %7lu neg %7lu, ", clk_count_refmux_pos, clk_count_refmux_neg);
      printf("sigmux %7lu, ", clk_count_sigmux);

      if(status.sample_idx == 0) {
        // lo - record counts
        clk_count_refmux_pos_lo = clk_count_refmux_pos;
        clk_count_refmux_neg_lo = clk_count_refmux_neg;
      }
      else if (status.sample_idx == 1) {
        // hi
        double v = ((double) clk_count_refmux_pos - (w * clk_count_refmux_neg))
                - ( (double) clk_count_refmux_pos_lo  - (w * clk_count_refmux_neg_lo));

        printf("v %f, ", v );

        double v2 = v / values_mean * 7.1 ;  //  need to adjust for the cal voltage

        printf( "v2 %s, ", str_format_float_with_commas(buf, 100, 9, v2));
        // printf("v2 %f, ", v2 );

        values[i] = v2;
        ++i;
      }
      else
        assert(0);


      printf("\n");
    }
    // trig off
    app_trigger( app, false);


    // rename values_mean
    values_mean = mean(   values, ARRAY_SIZE(values));
    values_stddev = stddev( values, ARRAY_SIZE(values));

    printf( "mean   %s, ", str_format_float_with_commas(buf, 100, 9, values_mean));
    printf( "stddev %s, ", str_format_float_with_commas(buf, 100, 9, values_stddev));
    printf("\n");


    // switch back to direct mode operation
    mode_reg_cr_set( mode, MODE_DIRECT);

    app_transition_state( app);

    printf("\n");
}



bool app_test52(
  app_t *app,
  const char *cmd
) {
  assert(app);
  assert(app->magic == APP_MAGIC);
  assert(cmd);

  if( strcmp(cmd, "test52") == 0) {

    printf("test52()\n");
    test( app);
    return 1;
  }

  return 0;
}

#if 0

> test52
test52()
set amp gain
i 0,   first=1  idx=0 seq_n=2, counts pos 1975925 neg 2025236, sigmux       0,
i 1,   first=0  idx=1 seq_n=2, counts pos 1975927 neg 2025238, sigmux       0,
i 2,   first=0  idx=0 seq_n=2, counts pos 1975928 neg 2025239, sigmux       0,
i 3,   first=0  idx=1 seq_n=2, counts pos 1975929 neg 2025240, sigmux       0,
i 4,   first=0  idx=0 seq_n=2, counts pos 1975928 neg 2025239, sigmux       0,
i 5,   first=0  idx=1 seq_n=2, counts pos 1975931 neg 2025242, sigmux       0,
i 6,   first=0  idx=0 seq_n=2, counts pos 1975925 neg 2025236, sigmux       0,
i 7,   first=0  idx=1 seq_n=2, counts pos 1975928 neg 2025239, sigmux       0,
i 8,   first=0  idx=0 seq_n=2, counts pos 1975926 neg 2025237, sigmux       0,
i 9,   first=0  idx=1 seq_n=2, counts pos 1975928 neg 2025239, sigmux       0,
pos mean   1975927.500, stddev 1.746,
neg mean   2025238.500, stddev 1.746,
w 0.975,651,757,
i 0, first=1  idx=0 seq_n=2, counts pos 1976369 neg 2025640, sigmux 4000001,
i 0, first=0  idx=1 seq_n=2, counts pos  988745 neg 3013366, sigmux 4000001, v -1951300.606911,
i 1, first=0  idx=0 seq_n=2, counts pos 1976334 neg 2025604, sigmux 4000001,
i 1, first=0  idx=1 seq_n=2, counts pos  988745 neg 3013366, sigmux 4000001, v -1951300.730374,
i 2, first=0  idx=0 seq_n=2, counts pos 1976337 neg 2025607, sigmux 4000001,
i 2, first=0  idx=1 seq_n=2, counts pos  988712 neg 3013332, sigmux 4000001, v -1951300.631259,
i 3, first=0  idx=0 seq_n=2, counts pos 1976336 neg 2025606, sigmux 4000001,
i 3, first=0  idx=1 seq_n=2, counts pos  988750 neg 3013371, sigmux 4000001, v -1951300.657329,
i 4, first=0  idx=0 seq_n=2, counts pos 1976338 neg 2025608, sigmux 4000001,
i 4, first=0  idx=1 seq_n=2, counts pos  988712 neg 3013332, sigmux 4000001, v -1951300.655607,
i 5, first=0  idx=0 seq_n=2, counts pos 1976336 neg 2025606, sigmux 4000001,
i 5, first=0  idx=1 seq_n=2, counts pos  988713 neg 3013333, sigmux 4000001, v -1951300.582562,
i 6, first=0  idx=0 seq_n=2, counts pos 1976342 neg 2025612, sigmux 4000001,
i 6, first=0  idx=1 seq_n=2, counts pos  988719 neg 3013339, sigmux 4000001, v -1951300.582562,
i 7, first=0  idx=0 seq_n=2, counts pos 1976347 neg 2025617, sigmux 4000001,
i 7, first=0  idx=1 seq_n=2, counts pos  988715 neg 3013335, sigmux 4000001, v -1951300.801697,
i 8, first=0  idx=0 seq_n=2, counts pos 1976342 neg 2025612, sigmux 4000001,
i 8, first=0  idx=1 seq_n=2, counts pos  988712 neg 3013332, sigmux 4000001, v -1951300.753000,
i 9, first=0  idx=0 seq_n=2, counts pos 1976338 neg 2025608, sigmux 4000001,
i 9, first=0  idx=1 seq_n=2, counts pos  988746 neg 3013367, sigmux 4000001, v -1951300.803419,
mean   -1951300.680, stddev 0.081,
i 0, first=1  idx=0 seq_n=2, counts pos 1976370 neg 2025641, sigmux 4000001,
i 0, first=0  idx=1 seq_n=2, counts pos  988744 neg 3013365, sigmux 4000001, v -1951300.655607, v2 7.099,999,910,,
i 1, first=0  idx=0 seq_n=2, counts pos 1976370 neg 2025641, sigmux 4000001,
i 1, first=0  idx=1 seq_n=2, counts pos  988745 neg 3013366, sigmux 4000001, v -1951300.631259, v2 7.099,999,821,,
i 2, first=0  idx=0 seq_n=2, counts pos 1976339 neg 2025609, sigmux 4000001,
i 2, first=0  idx=1 seq_n=2, counts pos  988748 neg 3013369, sigmux 4000001, v -1951300.779070, v2 7.100,000,359,,
i 3, first=0  idx=0 seq_n=2, counts pos 1976337 neg 2025607, sigmux 4000001,
i 3, first=0  idx=1 seq_n=2, counts pos  988743 neg 3013364, sigmux 4000001, v -1951300.852115, v2 7.100,000,625,,
i 4, first=0  idx=0 seq_n=2, counts pos 1976368 neg 2025639, sigmux 4000001,
i 4, first=0  idx=1 seq_n=2, counts pos  988739 neg 3013360, sigmux 4000001, v -1951300.728652, v2 7.100,000,175,,
i 5, first=0  idx=0 seq_n=2, counts pos 1976333 neg 2025603, sigmux 4000001,
i 5, first=0  idx=1 seq_n=2, counts pos  988744 neg 3013365, sigmux 4000001, v -1951300.730374, v2 7.100,000,182,,
i 6, first=0  idx=0 seq_n=2, counts pos 1976335 neg 2025605, sigmux 4000001,
i 6, first=0  idx=1 seq_n=2, counts pos  988747 neg 3013368, sigmux 4000001, v -1951300.706026, v2 7.100,000,093,,
i 7, first=0  idx=0 seq_n=2, counts pos 1976333 neg 2025603, sigmux 4000001,
i 7, first=0  idx=1 seq_n=2, counts pos  988748 neg 3013369, sigmux 4000001, v -1951300.632981, v2 7.099,999,827,,
i 8, first=0  idx=0 seq_n=2, counts pos 1976336 neg 2025606, sigmux 4000001,
i 8, first=0  idx=1 seq_n=2, counts pos  988750 neg 3013371, sigmux 4000001, v -1951300.657329, v2 7.099,999,916,,
i 9, first=0  idx=0 seq_n=2, counts pos 1976341 neg 2025611, sigmux 4000001,
i 9, first=0  idx=1 seq_n=2, counts pos  988749 neg 3013370, sigmux 4000001, v -1951300.803419, v2 7.100,000,447,,
mean   7.100,000,135,, stddev 0.000,000,262,,

#endif







