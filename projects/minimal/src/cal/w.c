
/*

*/



#include <stdio.h>
#include <assert.h>
// #include <math.h>     // NAN
#include <string.h>




#include <lib2/util.h>    // ARRAY_SIZE
#include <lib2/stats.h>
#include <lib2/format.h>  // format_with_commas



// #include <peripheral/spi-ice40.h>
#include <peripheral/gpio.h>        // trigger


#include <cal/transfer.h>
#include <app.h>
#include <mode.h>
#include <support.h> // nplc_to_aperture()

#include <data/cal.h>
#include <data/decode.h>






void app_cal_w( app_t *app)
{
  /*
    This is the cal. for the primary (not acal derived) DCV 10 range.
    It is irrelevant. that we use the local 7V ref as nominal value..

  */
  assert(app && app->magic == APP_MAGIC);

  _mode_t *mode = app->mode;
  assert(mode && mode->magic == MODE_MAGIC);

  decode_t *decode = app->decode;
  assert( decode && decode->magic == DECODE_MAGIC);

  cal_t *cal = app->cal;
  assert( cal && cal->magic == CAL_MAGIC);


  printf("--------\n");
  printf("cal_w\n");

  ///////////////////////////////
  // ensure sample off
  gpio_write( app->gpio_trigger, false);

  // reset mode
  mode_reset( mode);

  // set the trigger delay for settle time
  sa_trig_delay_set( &mode->sa, period_to_aperture(  1.f )); // 1 sec.

  // set normal sample acquisition/adc operation
  cr_mode_set( &mode->reg_cr, MODE_SA_ADC);

  // set nplc
  adc_aperture_set( &mode->adc, nplc_to_aperture( 10, *app->line_freq ));


  ///////////////////////////////


  // disable sigmux. required to calc relative pos/neg ref current weight.
  mode->reg_cr.adc_p_active_sigmux = 0;

  // special sample acquisition mode, just sampling lo
  sa_set( &mode->sa, "0" );

  // set ch2 input to LO, to reduce leakage on adc input mux.
  mode_ch2_set( mode, "ref-lo");

  app_transition_state( app);

  /////////////////////////


  // need double for mean()
  double pos_values[ 10 ];
  double neg_values[ 10 ];

  memset(pos_values, 0, sizeof(pos_values));
  memset(neg_values, 0, sizeof(neg_values));

  _Static_assert(ARRAY_SIZE(pos_values) == ARRAY_SIZE(neg_values), "array sizes do not match");


  decode->show_counts  = true;
  decode->show_reading = false;

  // fill decode
  app_fill_buffer1( app, pos_values, neg_values, ARRAY_SIZE( pos_values));

  double pos_mean   = mean(   pos_values, ARRAY_SIZE(pos_values));
  double neg_mean   = mean(   neg_values, ARRAY_SIZE(neg_values));
  double pos_stddev = stddev( pos_values, ARRAY_SIZE(pos_values));
  double neg_stddev = stddev( neg_values, ARRAY_SIZE(neg_values));

  char buf[100 + 1];

  printf( "pos mean   %.3f, stddev %.3f\n", pos_mean,  pos_stddev);
  printf( "neg mean   %.3f, stddev %.3f\n", neg_mean, neg_stddev);

  // ref current weight
  cal->w = pos_mean / neg_mean;

  printf( "cal->w %s\n", str_format_float_with_commas(buf, 100, 9, cal->w));


  // restore sigmux. even if mode_reset() will do it
  mode->reg_cr.adc_p_active_sigmux = 1;


}







#if 0



Feb 22. 2026.
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



#endif







