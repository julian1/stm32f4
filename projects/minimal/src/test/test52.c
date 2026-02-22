


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


    double mean_pos   = mean(   pos_values, ARRAY_SIZE(pos_values));
    double mean_neg   = mean(   neg_values, ARRAY_SIZE(neg_values));
    double stddev_pos = stddev( pos_values, ARRAY_SIZE(pos_values));
    double stddev_neg = stddev( neg_values, ARRAY_SIZE(neg_values));

    printf( "pos mean   %s\n",  str_format_float_with_commas(buf, 100, 9, mean_pos));
    printf( "pos stddev %s\n",  str_format_float_with_commas(buf, 100, 9, stddev_pos));
    printf( "neg mean   %s\n",  str_format_float_with_commas(buf, 100, 9, mean_neg));
    printf( "neg stddev %s\n",  str_format_float_with_commas(buf, 100, 9, stddev_neg));

    // printf( "aperture   %lu\n",  w_clk_count_aperture);


    // set w
    w =  mean_pos / mean_neg;

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

  double mean_values = mean(   values, ARRAY_SIZE(values));
  // printf( "mean_values %s\n", str_format_float_with_commas(buf, 100, 9, mean_values));
  printf( "mean_values %.3f\n",  mean_values);



  ////////////////////////


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
    for(unsigned i = 0; i < 5 ;)
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

        double v2 = v / mean_values * 7.1 ;  //  need to adjust for the cal voltage

        printf( "v2 %s, ", str_format_float_with_commas(buf, 100, 9, v2));
        // printf("v2 %f, ", v2 );

        ++i;

      }
      else
        assert(0);


      // ahhh no. we have to repeat the hi lo. thing going.
      // double v = clk_count_refmux_pos -   w_clk_count_refmux


      printf("\n");
    }
    // trig off
    app_trigger( app, false);

    // normal sample acquisition/adc operation
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
i 0,   first=1  idx=0 seq_n=2, counts pos 1975922 neg 2025233, sigmux       0,
i 1,   first=0  idx=1 seq_n=2, counts pos 1975926 neg 2025237, sigmux       0,
i 2,   first=0  idx=0 seq_n=2, counts pos 1975924 neg 2025235, sigmux       0,
i 3,   first=0  idx=1 seq_n=2, counts pos 1975926 neg 2025237, sigmux       0,
i 4,   first=0  idx=0 seq_n=2, counts pos 1975921 neg 2025232, sigmux       0,
i 5,   first=0  idx=1 seq_n=2, counts pos 1975925 neg 2025236, sigmux       0,
i 6,   first=0  idx=0 seq_n=2, counts pos 1975924 neg 2025235, sigmux       0,
i 7,   first=0  idx=1 seq_n=2, counts pos 1975924 neg 2025235, sigmux       0,
i 8,   first=0  idx=0 seq_n=2, counts pos 1975922 neg 2025233, sigmux       0,
i 9,   first=0  idx=1 seq_n=2, counts pos 1975924 neg 2025235, sigmux       0,
pos mean   1975923.800,000,000,
pos stddev 1.600,000,000,
neg mean   2025234.800,000,000,
neg stddev 1.600,000,000,
w 0.975,651,712,
i 0, first=1  idx=0 seq_n=2, counts pos 1976363 neg 2025633, sigmux 4000001,
i 0, first=0  idx=1 seq_n=2, counts pos  988741 neg 3013361, sigmux 4000001, v -1951300.514277,
i 1, first=0  idx=0 seq_n=2, counts pos 1976362 neg 2025632, sigmux 4000001,
i 1, first=0  idx=1 seq_n=2, counts pos  988741 neg 3013361, sigmux 4000001, v -1951300.489929,
i 2, first=0  idx=0 seq_n=2, counts pos 1976361 neg 2025631, sigmux 4000001,
i 2, first=0  idx=1 seq_n=2, counts pos  988741 neg 3013361, sigmux 4000001, v -1951300.465581,
i 3, first=0  idx=0 seq_n=2, counts pos 1976357 neg 2025627, sigmux 4000001,
i 3, first=0  idx=1 seq_n=2, counts pos  988740 neg 3013360, sigmux 4000001, v -1951300.392536,
i 4, first=0  idx=0 seq_n=2, counts pos 1976357 neg 2025627, sigmux 4000001,
i 4, first=0  idx=1 seq_n=2, counts pos  988736 neg 3013356, sigmux 4000001, v -1951300.489929,
i 5, first=0  idx=0 seq_n=2, counts pos 1976357 neg 2025627, sigmux 4000001,
i 5, first=0  idx=1 seq_n=2, counts pos  988736 neg 3013356, sigmux 4000001, v -1951300.489929,
i 6, first=0  idx=0 seq_n=2, counts pos 1976359 neg 2025629, sigmux 4000001,
i 6, first=0  idx=1 seq_n=2, counts pos  988738 neg 3013358, sigmux 4000001, v -1951300.489929,
i 7, first=0  idx=0 seq_n=2, counts pos 1976362 neg 2025632, sigmux 4000001,
i 7, first=0  idx=1 seq_n=2, counts pos  988742 neg 3013362, sigmux 4000001, v -1951300.465581,
i 8, first=0  idx=0 seq_n=2, counts pos 1976365 neg 2025635, sigmux 4000001,
i 8, first=0  idx=1 seq_n=2, counts pos  988744 neg 3013364, sigmux 4000001, v -1951300.489929,
i 9, first=0  idx=0 seq_n=2, counts pos 1976367 neg 2025637, sigmux 4000001,
i 9, first=0  idx=1 seq_n=2, counts pos  988744 neg 3013364, sigmux 4000001, v -1951300.538625,
mean_values -1951300.483
i 0, first=1  idx=0 seq_n=2, counts pos 1976371 neg 2025641, sigmux 4000001,
i 0, first=0  idx=1 seq_n=2, counts pos  988750 neg 3013370, sigmux 4000001, v -1951300.489929, v2 7.100,000,027,,
i 1, first=0  idx=0 seq_n=2, counts pos 1976365 neg 2025635, sigmux 4000001,
i 1, first=0  idx=1 seq_n=2, counts pos  988744 neg 3013364, sigmux 4000001, v -1951300.489929, v2 7.100,000,027,,
i 2, first=0  idx=0 seq_n=2, counts pos 1976363 neg 2025633, sigmux 4000001,
i 2, first=0  idx=1 seq_n=2, counts pos  988745 neg 3013365, sigmux 4000001, v -1951300.416884, v2 7.099,999,761,,
i 3, first=0  idx=0 seq_n=2, counts pos 1976366 neg 2025636, sigmux 4000001,
i 3, first=0  idx=1 seq_n=2, counts pos  988746 neg 3013366, sigmux 4000001, v -1951300.465581, v2 7.099,999,938,,
i 4, first=0  idx=0 seq_n=2, counts pos 1976370 neg 2025640, sigmux 4000001,
i 4, first=0  idx=1 seq_n=2, counts pos  988747 neg 3013367, sigmux 4000001, v -1951300.538625, v2 7.100,000,204,,

#endif







