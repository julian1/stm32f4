
/*
  Feb 2026.

  Can we create a calibration with just a ratio term.

  - no scale, offset.   so cannot compute noise
  - assume that parasitic/ leakage etc. will be constant. across az.

  by just setting the current ratio do not even need a value.

*/


/*
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
#include <string.h>
// #include <math.h>     // fabs



#include <lib3/util.h>    // ARRAY_SIZE
#include <lib3/stats.h>
#include <lib3/format.h>  // format_with_commas


#include <support.h> // nplc_to_aperture()
#include <mode.h>
#include <app.h>


#include <peripheral/gpio.h>        // trigger

#include <test/test.h>
#include <test/support.h>




static void test( app_t *app)
{

  _mode_t *mode = app->mode;
  assert(mode);
  assert(mode->magic == MODE_MAGIC) ;


  spi_t *spi = app->spi_fpga0;
  assert(spi);


  char buf[100 + 1];


  // we are not doing hi/lo herer
  double values[ 10];


  // sample off
  // app_trigger( app, false);
  gpio_write( app->gpio_trigger, false);


  mode_reset( mode);

  // normal sample acquisition/adc operation
  cr_mode_set( &mode->reg_cr, MODE_SA_ADC);


  // sample acquisition mode - for adc running standalone.  // REVIEW ME
  sa_set( &mode->sa, "0" );


  mode->reg_cr.adc_p_active_sigmux = 0;   // sigmux not active.



  // loop nplc
  for(unsigned k = 1; k <= 10; ++k) {

    // sampling off
    gpio_write( app->gpio_trigger, false);

    // set nplc
    adc_aperture_set( &mode->adc, nplc_to_aperture( k, *app->line_freq ));

    app_transition_state( app);

    // sleep
    app_msleep( app, 1000);

    // start sampling
    gpio_write( app->gpio_trigger, true);


    printf("nplc %u\n", k);

    // take obs loop
    for(unsigned i = 0; i < ARRAY_SIZE(values); ++i)
    {

      printf("i %u, ", i);


      // wait for adc data, on interrupt
      // use express yield function here. not app->yield etc
      while( !app->adc_interrupt_valid )
        app_yield( app);

      app->adc_interrupt_valid = false;


      data_t   data;
      memset( &data, 0, sizeof( data));
      spi_read_registers( spi, &data);
      print_data( &data);


      double w =  (double) (data.clk_count_refmux_pos  )  /   data.clk_count_refmux_neg  ;
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

    // stop sampling
    gpio_write( app->gpio_trigger, false);


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



bool app_test50(
  app_t *app,
  const char *cmd
) {
  assert(app);
  assert(app->magic == APP_MAGIC);
  assert(cmd);

  if( strcmp(cmd, "test50") == 0) {

    printf("test50()\n");
    test( app);
    return 1;
  }

  return 0;
}


#if 0


feb 2026.

empty

> test50
test50()
nplc 1
i 0,   first=1  idx=0 seq_n=2, w 0.975,649,566,
i 1,   first=0  idx=1 seq_n=2, w 0.975,649,574,
i 2,   first=0  idx=0 seq_n=2, w 0.975,649,446,
i 3,   first=0  idx=1 seq_n=2, w 0.975,649,454,
i 4,   first=0  idx=0 seq_n=2, w 0.975,649,454,
i 5,   first=0  idx=1 seq_n=2, w 0.975,649,574,
i 6,   first=0  idx=0 seq_n=2, w 0.975,649,454,
i 7,   first=0  idx=1 seq_n=2, w 0.975,649,566,
i 8,   first=0  idx=0 seq_n=2, w 0.975,649,574,
i 9,   first=0  idx=1 seq_n=2, w 0.975,649,566,
1 (n 10) mean   0.975,649,523, stddev 0.000,000,058,
nplc 2
i 0,   first=1  idx=0 seq_n=2, w 0.975,649,227,
i 1,   first=0  idx=1 seq_n=2, w 0.975,649,106,
i 2,   first=0  idx=0 seq_n=2, w 0.975,649,106,
i 3,   first=0  idx=1 seq_n=2, w 0.975,649,106,
i 4,   first=0  idx=0 seq_n=2, w 0.975,649,046,
i 5,   first=0  idx=1 seq_n=2, w 0.975,649,106,
i 6,   first=0  idx=0 seq_n=2, w 0.975,649,166,
i 7,   first=0  idx=1 seq_n=2, w 0.975,649,106,
i 8,   first=0  idx=0 seq_n=2, w 0.975,649,046,
i 9,   first=0  idx=1 seq_n=2, w 0.975,649,166,
2 (n 10) mean   0.975,649,118, stddev 0.000,000,052,
nplc 3
i 0,   first=1  idx=0 seq_n=2, w 0.975,648,884,
i 1,   first=0  idx=1 seq_n=2, w 0.975,648,884,
i 2,   first=0  idx=0 seq_n=2, w 0.975,648,844,
i 3,   first=0  idx=1 seq_n=2, w 0.975,648,924,
i 4,   first=0  idx=0 seq_n=2, w 0.975,648,804,
i 5,   first=0  idx=1 seq_n=2, w 0.975,648,844,
i 6,   first=0  idx=0 seq_n=2, w 0.975,648,844,
i 7,   first=0  idx=1 seq_n=2, w 0.975,648,844,
i 8,   first=0  idx=0 seq_n=2, w 0.975,648,924,
i 9,   first=0  idx=1 seq_n=2, w 0.975,648,844,
3 (n 10) mean   0.975,648,864, stddev 0.000,000,037,
nplc 4
i 0,   first=1  idx=0 seq_n=2, w 0.975,648,758,
i 1,   first=0  idx=1 seq_n=2, w 0.975,648,818,
i 2,   first=0  idx=0 seq_n=2, w 0.975,648,818,
i 3,   first=0  idx=1 seq_n=2, w 0.975,648,848,
i 4,   first=0  idx=0 seq_n=2, w 0.975,648,788,
i 5,   first=0  idx=1 seq_n=2, w 0.975,648,818,
i 6,   first=0  idx=0 seq_n=2, w 0.975,648,818,
i 7,   first=0  idx=1 seq_n=2, w 0.975,648,880,
i 8,   first=0  idx=0 seq_n=2, w 0.975,648,788,
i 9,   first=0  idx=1 seq_n=2, w 0.975,648,848,
4 (n 10) mean   0.975,648,818, stddev 0.000,000,033,
nplc 5
i 0,   first=1  idx=0 seq_n=2, w 0.975,648,697,
i 1,   first=0  idx=1 seq_n=2, w 0.975,648,673,
i 2,   first=0  idx=0 seq_n=2, w 0.975,648,721,
i 3,   first=0  idx=1 seq_n=2, w 0.975,648,745,
i 4,   first=0  idx=0 seq_n=2, w 0.975,648,745,
i 5,   first=0  idx=1 seq_n=2, w 0.975,648,721,
i 6,   first=0  idx=0 seq_n=2, w 0.975,648,769,
i 7,   first=0  idx=1 seq_n=2, w 0.975,648,793,
i 8,   first=0  idx=0 seq_n=2, w 0.975,648,721,
i 9,   first=0  idx=1 seq_n=2, w 0.975,648,721,
5 (n 10) mean   0.975,648,731, stddev 0.000,000,033,
nplc 6
i 0,   first=1  idx=0 seq_n=2, w 0.975,648,703,
i 1,   first=0  idx=1 seq_n=2, w 0.975,648,723,
i 2,   first=0  idx=0 seq_n=2, w 0.975,648,723,
i 3,   first=0  idx=1 seq_n=2, w 0.975,648,723,
i 4,   first=0  idx=0 seq_n=2, w 0.975,648,743,
i 5,   first=0  idx=1 seq_n=2, w 0.975,648,743,
i 6,   first=0  idx=0 seq_n=2, w 0.975,648,723,
i 7,   first=0  idx=1 seq_n=2, w 0.975,648,703,
i 8,   first=0  idx=0 seq_n=2, w 0.975,648,703,
i 9,   first=0  idx=1 seq_n=2, w 0.975,648,723,
6 (n 10) mean   0.975,648,721, stddev 0.000,000,014,
nplc 7
i 0,   first=1  idx=0 seq_n=2, w 0.975,648,686,
i 1,   first=0  idx=1 seq_n=2, w 0.975,648,703,
i 2,   first=0  idx=0 seq_n=2, w 0.975,648,703,
i 3,   first=0  idx=1 seq_n=2, w 0.975,648,686,
i 4,   first=0  idx=0 seq_n=2, w 0.975,648,686,
i 5,   first=0  idx=1 seq_n=2, w 0.975,648,720,
i 6,   first=0  idx=0 seq_n=2, w 0.975,648,669,
i 7,   first=0  idx=1 seq_n=2, w 0.975,648,686,
i 8,   first=0  idx=0 seq_n=2, w 0.975,648,669,
i 9,   first=0  idx=1 seq_n=2, w 0.975,648,651,
7 (n 10) mean   0.975,648,686, stddev 0.000,000,019,
nplc 8
i 0,   first=1  idx=0 seq_n=2, w 0.975,648,690,
i 1,   first=0  idx=1 seq_n=2, w 0.975,648,690,
i 2,   first=0  idx=0 seq_n=2, w 0.975,648,690,
i 3,   first=0  idx=1 seq_n=2, w 0.975,648,690,
i 4,   first=0  idx=0 seq_n=2, w 0.975,648,675,
i 5,   first=0  idx=1 seq_n=2, w 0.975,648,690,
i 6,   first=0  idx=0 seq_n=2, w 0.975,648,690,
i 7,   first=0  idx=1 seq_n=2, w 0.975,648,659,
i 8,   first=0  idx=0 seq_n=2, w 0.975,648,675,
i 9,   first=0  idx=1 seq_n=2, w 0.975,648,690,
8 (n 10) mean   0.975,648,684, stddev 0.000,000,010,
nplc 9
i 0,   first=1  idx=0 seq_n=2, w 0.975,648,638,
i 1,   first=0  idx=1 seq_n=2, w 0.975,648,665,
i 2,   first=0  idx=0 seq_n=2, w 0.975,648,652,
i 3,   first=0  idx=1 seq_n=2, w 0.975,648,678,
i 4,   first=0  idx=0 seq_n=2, w 0.975,648,638,
i 5,   first=0  idx=1 seq_n=2, w 0.975,648,665,
i 6,   first=0  idx=0 seq_n=2, w 0.975,648,638,
i 7,   first=0  idx=1 seq_n=2, w 0.975,648,638,
i 8,   first=0  idx=0 seq_n=2, w 0.975,648,665,
i 9,   first=0  idx=1 seq_n=2, w 0.975,648,652,
9 (n 10) mean   0.975,648,653, stddev 0.000,000,014,
nplc 10
i 0,   first=1  idx=0 seq_n=2, w 0.975,648,668,
i 1,   first=0  idx=1 seq_n=2, w 0.975,648,644,
i 2,   first=0  idx=0 seq_n=2, w 0.975,648,644,
i 3,   first=0  idx=1 seq_n=2, w 0.975,648,656,
i 4,   first=0  idx=0 seq_n=2, w 0.975,648,644,
i 5,   first=0  idx=1 seq_n=2, w 0.975,648,656,
i 6,   first=0  idx=0 seq_n=2, w 0.975,648,656,
i 7,   first=0  idx=1 seq_n=2, w 0.975,648,644,
i 8,   first=0  idx=0 seq_n=2, w 0.975,648,644,
i 9,   first=0  idx=1 seq_n=2, w 0.975,648,668,
10 (n 10) mean   0.975,648,652, stddev 0.000,000,009,


#endif

