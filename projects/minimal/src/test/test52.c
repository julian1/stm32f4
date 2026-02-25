
/*
  TODO

  - add count ratio.  in output
      so can adjust var pos to var neg.  for the bounds.

  - better variable prefix would use cal_w and cal_7v1_b, cal_aper  and cal_ref_v


  // why is there both aperture clk count, and sigmux clk count.  because runup does not turn on sigmux, for weight calculation
*/



#include <stdio.h>
#include <assert.h>
// #include <math.h>     // NAN
#include <string.h>




#include <lib2/util.h>    // ARRAY_SIZE
#include <lib2/stats.h>
#include <lib2/format.h>  // format_with_commas


#include <data/cal.h>
#include <mode.h>
#include <util.h> // nplc_to_aperture()
#include <app.h>


#include <peripheral/spi-ice40.h>
#include <peripheral/gpio.h>        // trigger manipulation





static void test2( app_t *app, double cal_w, double cal_7v1_b)
{
  /*
    should be able to use data_t and data_update() to handle all this

  */

  _mode_t *mode = app->mode;
  assert(mode);
  assert(mode->magic == MODE_MAGIC) ;


  spi_t *spi = app->spi_fpga0;
  assert(spi);


  char buf[100 + 1];


  ////////////////////


  // mode_ch2_set_ref( mode);

  mode_ch2_set_ref_lo( mode);
  mode_az_set(mode, "ch2" );

  // set 10V.
  // mode_lts_set( mode, 10 );
  // mode_ch2_set_lts( mode);

  // nplc to use
  mode_aperture_set( mode, nplc_to_aperture( 10, app->line_freq ));


  ////////////////////

  // record previous lo
  uint32_t clk_count_refmux_pos_lo = 0;
  uint32_t clk_count_refmux_neg_lo = 0;   // no adjustment

  // TODO consider better name - readings
  double values[ 10 ];
  memset(values, 0, sizeof(values));


  ////////////////////
  app_transition_state( app);


  // sleep
  app_msleep( app, 1000);

  // start sampling
  gpio_write( app->gpio_trigger, true);


  // take obs loop
  for(unsigned i = 0; i < ARRAY_SIZE( values);)
  {
    printf("i %u, ", i);

    // wait for adc data
    while( !app->adc_interrupt_valid )
      app_yield( app);

    app->adc_interrupt_valid = false;

    uint32_t status_ = spi_ice40_reg_read32( spi, REG_STATUS );
    reg_sr_t  status;
     _Static_assert(sizeof(status) == sizeof(status_), "bad typedef size");
    memcpy( &status, &status_,  sizeof( status_));

    uint32_t clk_count_refmux_pos = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_POS);
    uint32_t clk_count_refmux_neg = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_NEG);
    uint32_t clk_count_sigmux     = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_SIGMUX );

    printf("first=%u  idx=%u seq_n=%u, ", status.first, status.sample_idx, status.sample_seq_n);
    printf("counts pos %7lu neg %7lu, sigmux %7lu, ", clk_count_refmux_pos, clk_count_refmux_neg, clk_count_sigmux);

    if(status.sample_idx == 0) {
      // lo - record counts
      clk_count_refmux_pos_lo = clk_count_refmux_pos;
      clk_count_refmux_neg_lo = clk_count_refmux_neg;
    }
    else if (status.sample_idx == 1) {
      // hi
      double v = ((double) clk_count_refmux_pos    - (cal_w * clk_count_refmux_neg))
              - ( (double) clk_count_refmux_pos_lo - (cal_w * clk_count_refmux_neg_lo));

      printf("v %f, ", v );

      double v2 = v / clk_count_sigmux / cal_7v1_b * 7.1 ;  //  need to adjust for the cal voltage

      printf( "v2 %s, ", str_format_float_with_commas(buf, 100, 9, v2));
      // printf("v2 %f, ", v2 );

      values[i] = v2;
      ++i;
    }
    else
      assert(0);


    printf("\n");
  }

  // stop sampling
  gpio_write( app->gpio_trigger, false);




  // better names - readings_mean ?
  double values_mean   = mean(   values, ARRAY_SIZE(values));
  double values_stddev = stddev( values, ARRAY_SIZE(values));

  printf( "mean   %s, ", str_format_float_with_commas(buf, 100, 9, values_mean));
  printf( "stddev %s, ", str_format_float_with_commas(buf, 100, 9, values_stddev));
  printf("\n");



  printf("\n");

}








static void test( app_t *app)
{

  _mode_t *mode = app->mode;
  assert(mode);
  assert(mode->magic == MODE_MAGIC) ;

/*
  // TODO review/remove - only needed for line_freq... which indicates issue
  data_t    *data = app->data;
  assert(data);
  assert(data->magic == DATA_MAGIC) ;
*/

  spi_t *spi = app->spi_fpga0;
  assert(spi);

  char buf[100 + 1];


  // sample off
  gpio_write( app->gpio_trigger, false);

  mode_reset( mode);

  // normal sample acquisition/adc operation
  mode_reg_cr_set( mode, MODE_SA_ADC);

  // sample acquisition mode - for adc running standalone.  // REVIEW ME
  mode_az_set(mode, "0" );

  // REVIWE should not need this....
  mode_gain_set(mode, 1);

  // hold input to adc at lo. to reduce leakage.
  mode_ch2_set_ref_lo( mode);


  // set sigmux not active. needed to calculate relative pos/neg ref current weight.
  mode->reg_cr.adc_p_active_sigmux = 0;


  /////////////////////////

  unsigned nplc = 10;


  // need double for mean()
  double pos_values[ 10 ];
  double neg_values[ 10 ];

  memset(pos_values, 0, sizeof(pos_values));
  memset(neg_values, 0, sizeof(neg_values));


  _Static_assert(ARRAY_SIZE(pos_values) == ARRAY_SIZE(neg_values), "array sizes do not match");

  // stop sampling
  // app_trigger( app, false);
  gpio_write( app->gpio_trigger, false);

  // set nplc
  mode_aperture_set( mode, nplc_to_aperture( 10, app->line_freq ));


  app_transition_state( app);
  // sleep
  app_msleep( app, 1000);

  // start sampling
  gpio_write( app->gpio_trigger, true);


  /* we can run this loop more simply
      just sum up the values.
      no need for mean function etc...  although having the stddev is useful.

  */

  // take obs loop
  for(unsigned i = 0; i < ARRAY_SIZE(pos_values); ++i)
  {
    printf("i %u, ", i);

    // wait for adc data
    while( !app->adc_interrupt_valid )
      app_yield( app);

    app->adc_interrupt_valid = false;

    uint32_t status_ = spi_ice40_reg_read32( spi, REG_STATUS );
    reg_sr_t  status;
     _Static_assert(sizeof(status) == sizeof(status_), "bad typedef size");
    memcpy( &status, &status_,  sizeof( status_));


    uint32_t clk_count_refmux_pos   = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_POS);
    uint32_t clk_count_refmux_neg   = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_NEG);
    uint32_t clk_count_sigmux     = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_SIGMUX );

    printf("first=%u  idx=%u seq_n=%u, ", status.first, status.sample_idx, status.sample_seq_n);
    printf("counts pos %7lu neg %7lu, sig %7lu, ", clk_count_refmux_pos, clk_count_refmux_neg, clk_count_sigmux);

    pos_values[i] = clk_count_refmux_pos;
    neg_values[i] = clk_count_refmux_neg;

    printf("\n");
  }

  // sampling off
  gpio_write( app->gpio_trigger, false);


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

  // cal_w
  double cal_w = pos_mean / neg_mean;
  assert( cal_w);

  // printf(" w %.8f, ", w );
  printf( "cal_w %s\n", str_format_float_with_commas(buf, 100, 9, cal_w));






  ////////////////////////

  // previous lo
  uint32_t clk_count_refmux_pos_lo = 0;
  uint32_t clk_count_refmux_neg_lo = 0;   // no adjustment

  // TODO better name here. count cal_7v1_b/ factor.
  double values[ 10 ];
  memset(values, 0, sizeof(values));

  {
    // nplc
    mode_aperture_set( mode, nplc_to_aperture( nplc, app->line_freq ));

    /*
      expressing diff as a ratio of ref current - which is derived from main-ref is a decent approach.
      mean   -0.488   for 7.1V.
      mean   -0.689,  for 10V.

      then different ranges dcv/shunts can use a different multiplier.
    */

    // calibrate using ref-current sources, derived from main ref
    mode_ch2_set_ref( mode);

    // calibrate against 10V.
    // mode_lts_set( mode, 10 );
    // mode_ch2_set_lts( mode);

    mode_az_set(mode, "ch2" );

    // sigmux active
    mode->reg_cr.adc_p_active_sigmux = 1;

    app_transition_state( app);


    // sleep
    app_msleep( app, 1000);

    // start sampling
  gpio_write( app->gpio_trigger, true);


    // compute ref for diff
    for(unsigned i = 0; i < ARRAY_SIZE( values);)
    {
      printf("i %u, ", i);      // two readings per value...

      // wait for adc data
      while( !app->adc_interrupt_valid )
        app_yield( app);

      app->adc_interrupt_valid = false;

      uint32_t status_ = spi_ice40_reg_read32( spi, REG_STATUS );
      reg_sr_t  status;
       _Static_assert(sizeof(status) == sizeof(status_), "bad typedef size");
      memcpy( &status, &status_,  sizeof( status_));

      uint32_t clk_count_refmux_pos = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_POS);
      uint32_t clk_count_refmux_neg = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_NEG);
      uint32_t clk_count_sigmux     = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_SIGMUX );

      printf("first=%u  idx=%u seq_n=%u, ", status.first, status.sample_idx, status.sample_seq_n);
      printf("counts pos %7lu neg %7lu, sig %7lu, ", clk_count_refmux_pos, clk_count_refmux_neg, clk_count_sigmux);

      if(status.sample_idx == 0) {
        // lo - record counts
        clk_count_refmux_pos_lo = clk_count_refmux_pos;
        clk_count_refmux_neg_lo = clk_count_refmux_neg;
      }
      else if (status.sample_idx == 1) {
        // hi
        double v = ((double) clk_count_refmux_pos    - (cal_w * clk_count_refmux_neg))
                - ( (double) clk_count_refmux_pos_lo - (cal_w * clk_count_refmux_neg_lo));

        printf("v %f, ", v );

        // OK. we could incorporate the voltage target, here as well, if we wanted.

        // eg. values[ i ] = 7.1 / ( v / clk_count_sigmux );

        values[ i ] = v / clk_count_sigmux;
        // only increment on hi.
        ++i;
      }
      else
        assert(0);

      printf("\n");
    }

    // stop sampling
    gpio_write( app->gpio_trigger, false);

  }

  /*
    above code - should be able to just call data_update()  directly
      but keep separate. for independence when facoring etc.

  */

  // range[ range_10V ]  = 7.1 /   mean;

  // eg. the cal source (7.1 or external 10V) expressed  as ratio of the pos-ref-current
  // stddev.  doesn't mean much.

  double cal_7v1_b        = 7.1 / mean(   values, ARRAY_SIZE(values));
  // double cal_7v1_b_stddev = stddev( values, ARRAY_SIZE(values));

  printf( "cal_7v1_b %.3f, ", cal_7v1_b );
  // printf( "stddev %.9f, ", cal_7v1_b_stddev);
  printf("\n");


  assert(app->cal);
  assert(app->cal->magic == CAL_MAGIC);

  app->cal->w        = cal_w;
  app->cal->cal_7v1_b  = cal_7v1_b;


  test2( app, cal_w, cal_7v1_b);



  ////////////////////////

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
nplc 10 for everything
set amp gain
i 0, first=1  idx=0 seq_n=2, counts pos 1975923 neg 2025236, sig       0,
i 1, first=0  idx=1 seq_n=2, counts pos 1975927 neg 2025240, sig       0,
i 2, first=0  idx=0 seq_n=2, counts pos 1975925 neg 2025238, sig       0,
i 3, first=0  idx=1 seq_n=2, counts pos 1975924 neg 2025237, sig       0,
i 4, first=0  idx=0 seq_n=2, counts pos 1975923 neg 2025236, sig       0,
i 5, first=0  idx=1 seq_n=2, counts pos 1975926 neg 2025239, sig       0,
i 6, first=0  idx=0 seq_n=2, counts pos 1975926 neg 2025239, sig       0,
i 7, first=0  idx=1 seq_n=2, counts pos 1975928 neg 2025241, sig       0,
i 8, first=0  idx=0 seq_n=2, counts pos 1975924 neg 2025237, sig       0,
i 9, first=0  idx=1 seq_n=2, counts pos 1975926 neg 2025239, sig       0,
pos mean   1975925.200, stddev 1.600,
neg mean   2025238.200, stddev 1.600,
cal_w 0.975,650,765,
i 0, first=1  idx=0 seq_n=2, counts pos 1976353 neg 2025623, sig 4000001,
i 0, first=0  idx=1 seq_n=2, counts pos  988735 neg 3013356, sig 4000001, v -1951300.457487,
i 1, first=0  idx=0 seq_n=2, counts pos 1976360 neg 2025630, sig 4000001,
i 1, first=0  idx=1 seq_n=2, counts pos  988738 neg 3013359, sig 4000001, v -1951300.554883,
i 2, first=0  idx=0 seq_n=2, counts pos 1976358 neg 2025628, sig 4000001,
i 2, first=0  idx=1 seq_n=2, counts pos  988735 neg 3013356, sig 4000001, v -1951300.579233,
i 3, first=0  idx=0 seq_n=2, counts pos 1976359 neg 2025629, sig 4000001,
i 3, first=0  idx=1 seq_n=2, counts pos  988740 neg 3013361, sig 4000001, v -1951300.481836,
i 4, first=0  idx=0 seq_n=2, counts pos 1976360 neg 2025630, sig 4000001,
i 4, first=0  idx=1 seq_n=2, counts pos  988739 neg 3013360, sig 4000001, v -1951300.530534,
i 5, first=0  idx=0 seq_n=2, counts pos 1976365 neg 2025635, sig 4000001,
i 5, first=0  idx=1 seq_n=2, counts pos  988738 neg 3013359, sig 4000001, v -1951300.676630,
i 6, first=0  idx=0 seq_n=2, counts pos 1976361 neg 2025631, sig 4000001,
i 6, first=0  idx=1 seq_n=2, counts pos  988737 neg 3013358, sig 4000001, v -1951300.603582,
i 7, first=0  idx=0 seq_n=2, counts pos 1976361 neg 2025631, sig 4000001,
i 7, first=0  idx=1 seq_n=2, counts pos  988738 neg 3013359, sig 4000001, v -1951300.579233,
i 8, first=0  idx=0 seq_n=2, counts pos 1976361 neg 2025631, sig 4000001,
i 8, first=0  idx=1 seq_n=2, counts pos  988737 neg 3013358, sig 4000001, v -1951300.603582,
i 9, first=0  idx=0 seq_n=2, counts pos 1976362 neg 2025632, sig 4000001,
i 9, first=0  idx=1 seq_n=2, counts pos  988741 neg 3013362, sig 4000001, v -1951300.530534,
mean   -0.488, stddev 0.000000015,
i 0, first=1  idx=0 seq_n=2, counts pos 1976354 neg 2025624, sigmux 4000001,
i 0, first=0  idx=1 seq_n=2, counts pos 1976360 neg 2025630, sigmux 4000001, v 0.146095, v2 -0.000,000,532,,
i 1, first=0  idx=0 seq_n=2, counts pos 1976359 neg 2025629, sigmux 4000001,
i 1, first=0  idx=1 seq_n=2, counts pos 1976362 neg 2025632, sigmux 4000001, v 0.073048, v2 -0.000,000,266,,
i 2, first=0  idx=0 seq_n=2, counts pos 1976357 neg 2025627, sigmux 4000001,
i 2, first=0  idx=1 seq_n=2, counts pos 1976363 neg 2025633, sigmux 4000001, v 0.146095, v2 -0.000,000,532,,
i 3, first=0  idx=0 seq_n=2, counts pos 1976361 neg 2025631, sigmux 4000001,
i 3, first=0  idx=1 seq_n=2, counts pos 1976363 neg 2025633, sigmux 4000001, v 0.048698, v2 -0.000,000,177,,
i 4, first=0  idx=0 seq_n=2, counts pos 1976360 neg 2025630, sigmux 4000001,
i 4, first=0  idx=1 seq_n=2, counts pos 1976365 neg 2025635, sigmux 4000001, v 0.121746, v2 -0.000,000,443,,
i 5, first=0  idx=0 seq_n=2, counts pos 1976359 neg 2025629, sigmux 4000001,
i 5, first=0  idx=1 seq_n=2, counts pos 1976361 neg 2025631, sigmux 4000001, v 0.048698, v2 -0.000,000,177,,
i 6, first=0  idx=0 seq_n=2, counts pos 1976359 neg 2025629, sigmux 4000001,
i 6, first=0  idx=1 seq_n=2, counts pos 1976362 neg 2025632, sigmux 4000001, v 0.073048, v2 -0.000,000,266,,
i 7, first=0  idx=0 seq_n=2, counts pos 1976356 neg 2025626, sigmux 4000001,
i 7, first=0  idx=1 seq_n=2, counts pos 1976359 neg 2025629, sigmux 4000001, v 0.073048, v2 -0.000,000,266,,
i 8, first=0  idx=0 seq_n=2, counts pos 1976356 neg 2025626, sigmux 4000001,
i 8, first=0  idx=1 seq_n=2, counts pos 1976361 neg 2025631, sigmux 4000001, v 0.121746, v2 -0.000,000,443,,
i 9, first=0  idx=0 seq_n=2, counts pos 1976356 neg 2025626, sigmux 4000001,
i 9, first=0  idx=1 seq_n=2, counts pos 1976363 neg 2025633, sigmux 4000001, v 0.170445, v2 -0.000,000,620,,
mean   -0.000,000,372,, stddev 0.000,000,152,,




Feb 22. 2026.
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







