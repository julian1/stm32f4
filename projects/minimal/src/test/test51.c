


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

      w_clk_count_aperture            = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_APERTURE);

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

      // difference in weighted.

      double neg_w = clk_count_refmux_neg  * w;
      double v =  (double)clk_count_refmux_pos  - neg_w;
      values[ i ] = v;
      printf( "neg_w %.3f,  v %.3f ", neg_w , v );


      printf("\n");
    }

    // stop sampling
    app_trigger( app, false);


    double mean_   = mean(   values, ARRAY_SIZE(values));     // should prefix functions stats_mean ?
    double stddev_ = stddev( values, ARRAY_SIZE(values));

    printf("(n %u) ", ARRAY_SIZE(values));
    printf( "mean   %.3f ",  mean_);
    printf( "stddev %.3f ",  stddev_);
    printf( "mean ap. adj %.3f ",  mean_  / clk_count_aperture * w_clk_count_aperture);
    printf( "aperture %lu ",  clk_count_aperture);

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



#if 0

feb 2026.

> test51
test51()
i 0,   first=1  idx=0 seq_n=2,   counts pos 198412 neg 203364
i 1,   first=0  idx=1 seq_n=2,   counts pos 198411 neg 203363
i 2,   first=0  idx=0 seq_n=2,   counts pos 198412 neg 203364
i 3,   first=0  idx=1 seq_n=2,   counts pos 198411 neg 203363
i 4,   first=0  idx=0 seq_n=2,   counts pos 198451 neg 203404
i 5,   first=0  idx=1 seq_n=2,   counts pos 198411 neg 203363
i 6,   first=0  idx=0 seq_n=2,   counts pos 198412 neg 203364
i 7,   first=0  idx=1 seq_n=2,   counts pos 198411 neg 203363
i 8,   first=0  idx=0 seq_n=2,   counts pos 198410 neg 203362
i 9,   first=0  idx=1 seq_n=2,   counts pos 198451 neg 203404
pos mean   198419.200,000,000,
pos stddev 15.911,002,483,
neg mean   203371.400,000,000,
neg stddev 16.310,732,663,
aperture   400001
w 0.975,649,477,
nplc 1
i 0,   first=1  idx=0 seq_n=2,  pos 198412 neg 203364, neg_w 198411.980,  v 0.020
i 1,   first=0  idx=1 seq_n=2,  pos 198411 neg 203363, neg_w 198411.005,  v -0.005
i 2,   first=0  idx=0 seq_n=2,  pos 198411 neg 203363, neg_w 198411.005,  v -0.005
i 3,   first=0  idx=1 seq_n=2,  pos 198412 neg 203364, neg_w 198411.980,  v 0.020
i 4,   first=0  idx=0 seq_n=2,  pos 198412 neg 203364, neg_w 198411.980,  v 0.020
i 5,   first=0  idx=1 seq_n=2,  pos 198412 neg 203364, neg_w 198411.980,  v 0.020
i 6,   first=0  idx=0 seq_n=2,  pos 198411 neg 203363, neg_w 198411.005,  v -0.005
i 7,   first=0  idx=1 seq_n=2,  pos 198451 neg 203404, neg_w 198451.006,  v -0.006
i 8,   first=0  idx=0 seq_n=2,  pos 198411 neg 203363, neg_w 198411.005,  v -0.005
i 9,   first=0  idx=1 seq_n=2,  pos 198451 neg 203404, neg_w 198451.006,  v -0.006
(n 10) mean   0.005 stddev 0.012  mean ap. adj 0.005 aperture 400001
nplc 2
i 0,   first=1  idx=0 seq_n=2,  pos 395493 neg 405364, neg_w 395493.175,  v -0.175
i 1,   first=0  idx=1 seq_n=2,  pos 395494 neg 405365, neg_w 395494.150,  v -0.150
i 2,   first=0  idx=0 seq_n=2,  pos 395492 neg 405363, neg_w 395492.199,  v -0.199
i 3,   first=0  idx=1 seq_n=2,  pos 395493 neg 405364, neg_w 395493.175,  v -0.175
i 4,   first=0  idx=0 seq_n=2,  pos 395494 neg 405365, neg_w 395494.150,  v -0.150
i 5,   first=0  idx=1 seq_n=2,  pos 395493 neg 405364, neg_w 395493.175,  v -0.175
i 6,   first=0  idx=0 seq_n=2,  pos 395492 neg 405363, neg_w 395492.199,  v -0.199
i 7,   first=0  idx=1 seq_n=2,  pos 395495 neg 405366, neg_w 395495.126,  v -0.126
i 8,   first=0  idx=0 seq_n=2,  pos 395493 neg 405364, neg_w 395493.175,  v -0.175
i 9,   first=0  idx=1 seq_n=2,  pos 395493 neg 405364, neg_w 395493.175,  v -0.175
(n 10) mean   -0.170 stddev 0.021  mean ap. adj -0.085 aperture 800001
nplc 3
i 0,   first=1  idx=0 seq_n=2,  pos 593575 neg 608390, neg_w 593575.385,  v -0.385
i 1,   first=0  idx=1 seq_n=2,  pos 593575 neg 608390, neg_w 593575.385,  v -0.385
i 2,   first=0  idx=0 seq_n=2,  pos 593575 neg 608390, neg_w 593575.385,  v -0.385
i 3,   first=0  idx=1 seq_n=2,  pos 593576 neg 608391, neg_w 593576.361,  v -0.361
i 4,   first=0  idx=0 seq_n=2,  pos 593573 neg 608388, neg_w 593573.434,  v -0.434
i 5,   first=0  idx=1 seq_n=2,  pos 593576 neg 608391, neg_w 593576.361,  v -0.361
i 6,   first=0  idx=0 seq_n=2,  pos 593574 neg 608389, neg_w 593574.410,  v -0.410
i 7,   first=0  idx=1 seq_n=2,  pos 593574 neg 608389, neg_w 593574.410,  v -0.410
i 8,   first=0  idx=0 seq_n=2,  pos 593576 neg 608391, neg_w 593576.361,  v -0.361
i 9,   first=0  idx=1 seq_n=2,  pos 593574 neg 608389, neg_w 593574.410,  v -0.410
(n 10) mean   -0.390 stddev 0.024  mean ap. adj -0.130 aperture 1200001
nplc 4
...
i 3,   first=0  idx=1 seq_n=2,  pos 790737 neg 810473, neg_w 790737.558,  v -0.558
i 4,   first=0  idx=0 seq_n=2,  pos 790737 neg 810473, neg_w 790737.558,  v -0.558
i 5,   first=0  idx=1 seq_n=2,  pos 790737 neg 810473, neg_w 790737.558,  v -0.558
i 6,   first=0  idx=0 seq_n=2,  pos 790739 neg 810475, neg_w 790739.510,  v -0.510
i 7,   first=0  idx=1 seq_n=2,  pos 790738 neg 810474, neg_w 790738.534,  v -0.534
i 8,   first=0  idx=0 seq_n=2,  pos 790736 neg 810472, neg_w 790736.583,  v -0.583
i 9,   first=0  idx=1 seq_n=2,  pos 790738 neg 810474, neg_w 790738.534,  v -0.534
(n 10) mean   -0.544 stddev 0.022  mean ap. adj -0.136 aperture 1600001
nplc 5
i 0,   first=1  idx=0 seq_n=2,  pos 988738 neg 1013416, neg_w 988738.790,  v -0.790
i 1,   first=0  idx=1 seq_n=2,  pos 988738 neg 1013416, neg_w 988738.790,  v -0.790
i 2,   first=0  idx=0 seq_n=2,  pos 988739 neg 1013417, neg_w 988739.766,  v -0.766
i 3,   first=0  idx=1 seq_n=2,  pos 988741 neg 1013419, neg_w 988741.717,  v -0.717
i 4,   first=0  idx=0 seq_n=2,  pos 988738 neg 1013416, neg_w 988738.790,  v -0.790
i 5,   first=0  idx=1 seq_n=2,  pos 988739 neg 1013417, neg_w 988739.766,  v -0.766
i 6,   first=0  idx=0 seq_n=2,  pos 988740 neg 1013418, neg_w 988740.741,  v -0.741
i 7,   first=0  idx=1 seq_n=2,  pos 988740 neg 1013418, neg_w 988740.741,  v -0.741
i 8,   first=0  idx=0 seq_n=2,  pos 988740 neg 1013418, neg_w 988740.741,  v -0.741
i 9,   first=0  idx=1 seq_n=2,  pos 988739 neg 1013417, neg_w 988739.766,  v -0.766
(n 10) mean   -0.761 stddev 0.024  mean ap. adj -0.152 aperture 2000001
nplc 6
i 0,   first=1  idx=0 seq_n=2,  pos 1185940 neg 1215540, neg_w 1185940.965,  v -0.965
i 1,   first=0  idx=1 seq_n=2,  pos 1185943 neg 1215543, neg_w 1185943.892,  v -0.892
i 2,   first=0  idx=0 seq_n=2,  pos 1185944 neg 1215544, neg_w 1185944.868,  v -0.868
i 3,   first=0  idx=1 seq_n=2,  pos 1185941 neg 1215541, neg_w 1185941.941,  v -0.941
i 4,   first=0  idx=0 seq_n=2,  pos 1185942 neg 1215542, neg_w 1185942.916,  v -0.916
i 5,   first=0  idx=1 seq_n=2,  pos 1185939 neg 1215539, neg_w 1185939.989,  v -0.989
i 6,   first=0  idx=0 seq_n=2,  pos 1185942 neg 1215542, neg_w 1185942.916,  v -0.916
i 7,   first=0  idx=1 seq_n=2,  pos 1185941 neg 1215541, neg_w 1185941.941,  v -0.941
i 8,   first=0  idx=0 seq_n=2,  pos 1185940 neg 1215540, neg_w 1185940.965,  v -0.965
i 9,   first=0  idx=1 seq_n=2,  pos 1185943 neg 1215543, neg_w 1185943.892,  v -0.892
(n 10) mean   -0.928 stddev 0.037  mean ap. adj -0.155 aperture 2400001
nplc 7
i 0,   first=1  idx=0 seq_n=2,  pos 1383904 neg 1418445, neg_w 1383905.122,  v -1.122
i 1,   first=0  idx=1 seq_n=2,  pos 1383905 neg 1418446, neg_w 1383906.098,  v -1.098
i 2,   first=0  idx=0 seq_n=2,  pos 1383903 neg 1418444, neg_w 1383904.146,  v -1.146
i 3,   first=0  idx=1 seq_n=2,  pos 1383906 neg 1418447, neg_w 1383907.073,  v -1.073
i 4,   first=0  idx=0 seq_n=2,  pos 1383904 neg 1418445, neg_w 1383905.122,  v -1.122
i 5,   first=0  idx=1 seq_n=2,  pos 1383904 neg 1418445, neg_w 1383905.122,  v -1.122
i 6,   first=0  idx=0 seq_n=2,  pos 1383904 neg 1418445, neg_w 1383905.122,  v -1.122
i 7,   first=0  idx=1 seq_n=2,  pos 1383907 neg 1418448, neg_w 1383908.049,  v -1.049
i 8,   first=0  idx=0 seq_n=2,  pos 1383903 neg 1418444, neg_w 1383904.146,  v -1.146
i 9,   first=0  idx=1 seq_n=2,  pos 1383904 neg 1418445, neg_w 1383905.122,  v -1.122
(n 10) mean   -1.112 stddev 0.029  mean ap. adj -0.159 aperture 2800001
nplc 8
i 0,   first=1  idx=0 seq_n=2,  pos 1581187 neg 1620652, neg_w 1581188.276,  v -1.276
i 1,   first=0  idx=1 seq_n=2,  pos 1581185 neg 1620650, neg_w 1581186.325,  v -1.325
i 2,   first=0  idx=0 seq_n=2,  pos 1581186 neg 1620651, neg_w 1581187.300,  v -1.300
i 3,   first=0  idx=1 seq_n=2,  pos 1581186 neg 1620651, neg_w 1581187.300,  v -1.300
i 4,   first=0  idx=0 seq_n=2,  pos 1581187 neg 1620652, neg_w 1581188.276,  v -1.276
i 5,   first=0  idx=1 seq_n=2,  pos 1581189 neg 1620654, neg_w 1581190.227,  v -1.227
i 6,   first=0  idx=0 seq_n=2,  pos 1581186 neg 1620651, neg_w 1581187.300,  v -1.300
i 7,   first=0  idx=1 seq_n=2,  pos 1581187 neg 1620652, neg_w 1581188.276,  v -1.276
i 8,   first=0  idx=0 seq_n=2,  pos 1581188 neg 1620653, neg_w 1581189.251,  v -1.251
i 9,   first=0  idx=1 seq_n=2,  pos 1581185 neg 1620650, neg_w 1581186.325,  v -1.325
(n 10) mean   -1.286 stddev 0.029  mean ap. adj -0.161 aperture 3200001
nplc 9
i 0,   first=1  idx=0 seq_n=2,  pos 1778505 neg 1822895, neg_w 1778506.553,  v -1.553
i 1,   first=0  idx=1 seq_n=2,  pos 1778507 neg 1822897, neg_w 1778508.504,  v -1.504
i 2,   first=0  idx=0 seq_n=2,  pos 1778507 neg 1822897, neg_w 1778508.504,  v -1.504
i 3,   first=0  idx=1 seq_n=2,  pos 1778508 neg 1822898, neg_w 1778509.480,  v -1.480
i 4,   first=0  idx=0 seq_n=2,  pos 1778509 neg 1822899, neg_w 1778510.456,  v -1.456
i 5,   first=0  idx=1 seq_n=2,  pos 1778506 neg 1822896, neg_w 1778507.529,  v -1.529
i 6,   first=0  idx=0 seq_n=2,  pos 1778509 neg 1822899, neg_w 1778510.456,  v -1.456
i 7,   first=0  idx=1 seq_n=2,  pos 1778509 neg 1822899, neg_w 1778510.456,  v -1.456
i 8,   first=0  idx=0 seq_n=2,  pos 1778509 neg 1822899, neg_w 1778510.456,  v -1.456
i 9,   first=0  idx=1 seq_n=2,  pos 1778507 neg 1822897, neg_w 1778508.504,  v -1.504
(n 10) mean   -1.490 stddev 0.033  mean ap. adj -0.166 aperture 3600001
nplc 10
i 0,   first=1  idx=0 seq_n=2,  pos 1975908 neg 2025225, neg_w 1975909.712,  v -1.712
i 1,   first=0  idx=1 seq_n=2,  pos 1975908 neg 2025225, neg_w 1975909.712,  v -1.712
i 2,   first=0  idx=0 seq_n=2,  pos 1975911 neg 2025228, neg_w 1975912.639,  v -1.639
i 3,   first=0  idx=1 seq_n=2,  pos 1975910 neg 2025227, neg_w 1975911.663,  v -1.663
i 4,   first=0  idx=0 seq_n=2,  pos 1975909 neg 2025226, neg_w 1975910.687,  v -1.687
i 5,   first=0  idx=1 seq_n=2,  pos 1975910 neg 2025227, neg_w 1975911.663,  v -1.663
i 6,   first=0  idx=0 seq_n=2,  pos 1975909 neg 2025226, neg_w 1975910.687,  v -1.687
i 7,   first=0  idx=1 seq_n=2,  pos 1975910 neg 2025227, neg_w 1975911.663,  v -1.663
i 8,   first=0  idx=0 seq_n=2,  pos 1975910 neg 2025227, neg_w 1975911.663,  v -1.663
i 9,   first=0  idx=1 seq_n=2,  pos 1975910 neg 2025227, neg_w 1975911.663,  v -1.663
(n 10) mean   -1.675 stddev 0.022  mean ap. adj -0.168 aperture 4000001

#endif
