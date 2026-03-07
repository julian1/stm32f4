
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

#include <app.h>
#include <util.h> // nplc_to_aperture()
#include <mode.h>

#include <data/cal.h>
#include <data/data.h>





void app_cal_b( app_t *app)
{
  /*
    This is the cal. for the primary (not acal derived) DCV 10 range.
    It is irrelevant. that we use the local 7V ref as nominal value..

  */
  assert(app && app->magic == APP_MAGIC);

  _mode_t *mode = app->mode;
  assert(mode && mode->magic == MODE_MAGIC);

  data_t *data = app->data;
  assert( data && data->magic == DATA_MAGIC);

  cal_t *cal = app->cal;
  assert( cal && cal->magic == CAL_MAGIC);

  assert(cal->w);

  printf("--------\n");
  printf("cal_b\n");


  app_cal_setup( app);


  // use the ref as source
  mode_ch2_set( mode, "ref");

  // alternate calibrate against 10V.
  // mode_lts_source_set( mode, 10 );
  // mode_ch2_set_lts( mode);
  mode_az_set(mode, "ch2" );

  app_transition_state( app);

  // TODO consider better name
  // double count_norm[];
  double values[ 10 ];
  memset( values, 0, sizeof(values));

  data->show_reading = false;
  app_fill_buffer( app, values, ARRAY_SIZE( values));


  char buf[100 + 1];
  printf("norm ");
  printf( "mean   %.9f, ", mean( values, ARRAY_SIZE(values)) );
  printf( "stddev %.9f, ", stddev( values, ARRAY_SIZE(values)) );
  printf("\n");

  /*
    above code - should be able to just call data_update()  directly
      but keep separate. for independence when facoring etc.
  */

  cal->b = 7.0 / mean( values, ARRAY_SIZE(values));
  printf("cal->b %f\n", cal->b);

  printf("\n");


  /////////////////////////////////////

  // now show some values..  to confirm

  data->show_reading = true;
  app_fill_buffer( app, values, ARRAY_SIZE( values));

  // better names - readings_mean ?
  double values_mean   = mean(   values, ARRAY_SIZE(values))  ;
  double values_stddev = stddev( values, ARRAY_SIZE(values));

  printf( "mean   %s, ", str_format_float_with_commas(buf, 100, 9, values_mean));
  printf( "stddev %s, ", str_format_float_with_commas(buf, 100, 9, values_stddev));
  printf("\n");


  app_cal_finish( app);

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







