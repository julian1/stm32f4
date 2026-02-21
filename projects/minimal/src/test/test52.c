


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

  // hold input to adc at lo. to reduce leakage.
  mode_ch2_set_ref_lo( mode);


  mode->reg_cr.adc_p_active_sigmux = 0;   // sigmux not active.


  /////////////////////////

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
    mode->adc.p_aperture = nplc_to_aperture( 1 , data->line_freq );
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

      // w_clk_count_aperture            = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_APERTURE);

      printf("  counts pos %lu neg %lu", clk_count_refmux_pos, clk_count_refmux_neg);

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


    // set nplc
    mode->adc.p_aperture = nplc_to_aperture( 1, data->line_freq );

    // reset ; set lts 10; set gain 1;  set ch2 lts; set az ch2; set mode 6; trig;
    // need to set the ref.


    // use the internal reference
    mode_ch2_set_ref( mode);

    mode_az_set(mode, "ch2" );



    app_transition_state( app);
    // sleep
    yield_with_msleep( 1 * 1000, &app->system_millis, (void (*)(void *))app_update_simple_led_blink, app);
    // start sampling
    app_trigger( app, true);


    // may need larger int ....
    // for long
    // better name clk_count_refmux_net
    uint32_t w_clk_count_refmux = 0;     // this is weight/adjusted.
    uint32_t w_clk_count_aperture  = 0 ;

    // compute ref for diff
    // take obs loop
    for(unsigned i = 0; i < 10; ++i)
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

      // we care about hi v lo. yes because we want the diff.
      if(status.sample_idx == 0) {
        // lo
        w_clk_count_refmux   += spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_POS);
        w_clk_count_refmux   -= w * spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_NEG) ;
      } else {
        // hi
        w_clk_count_refmux   -= spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_POS);
        w_clk_count_refmux   -= w * spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_NEG);
      }

      w_clk_count_aperture  += spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_APERTURE);
      printf("\n");
    }

    // stop sampling
    app_trigger( app, false);






  ////////////////////////

    // sleep
    yield_with_msleep( 1 * 1000, &app->system_millis, (void (*)(void *))app_update_simple_led_blink, app);
    // start sampling
    app_trigger( app, true);

    ////////
    // only record the last lo.
    uint32_t clk_count_refmux_pos_lo = 0;     // this is adjusted...
    uint32_t clk_count_refmux_neg_lo = 0;

    // take obs loop
    for(unsigned i = 0; i < 10; ++i)
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

      uint32_t clk_count_refmux_pos = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_POS);
      uint32_t clk_count_refmux_neg = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_NEG);
      uint32_t clk_count_aperture   = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_APERTURE);

      // we care about hi v lo. yes because we want the diff.
      if(status.sample_idx == 0) {
        // lo

        // just record to use for later...
        // we can produce a value here as well.
        clk_count_refmux_pos_lo = clk_count_refmux_pos;
        clk_count_refmux_neg_lo = clk_count_refmux_neg;
      } else {
        // hi


        double v = ((double) +clk_count_refmux_pos - (w * clk_count_refmux_neg))
                - ( clk_count_refmux_pos_lo  - (w * clk_count_refmux_neg_lo));


        double v2 = v / clk_count_aperture  * w_clk_count_aperture  * 7.1 ;  //  need to adjust for the cal voltage


        printf("v2 %f\n", v2 );

      }


      // ahhh no. we have to repeat the hi lo. thing going.
      // double v = clk_count_refmux_pos -   w_clk_count_refmux


      printf("\n");
    }
    // trig off
    app_trigger( app, false);





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


