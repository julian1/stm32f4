/*
  REMEMBER
    - amplifier is picking up lots of smps noise. from the inductor.
    especially higher ranges.

  // could use an array of all these functions.
*/

#include <stdio.h>
#include <assert.h>

#include <lib2/util.h>    // UNUSED

#include <cal/transfer.h>
#include <app.h>
#include <mode.h>
#include <data/cal.h>




static void step1( app_t *app)
{
  printf("--------\n");
  printf("cal_b\n");

  assert(app->cal->w);

/*
  should use the range function.
  that way. it is clear what reading value is displayed
  // Could probably use the range here to set to the REF.
*/

  // reference range
  sa_set( &app->mode->sa, "ch2" );

  // alternate calibrate against 10V.
  // mode_lts_source_set( mode, 10 );
  // mode_ch2_set_lts( mode);
  // use the ref as source
  mode_ch2_set( app->mode, "ref");

}


static void step2( app_t *app)
{
  UNUSED( app);
  // ignore decode.here....
  // because target is comes from external source or ref

  printf("*data ignored*\n");
}


static void cal_set_value( cal_t *cal, double mean0, double mean1)
{
  // values are count_norm
  UNUSED(mean1);
  // could also reset the mode for the print data

  cal->b = 7.0 / mean0;
  printf("cal->b %f\n", cal->b);
}




void app_cal_b( app_t *app)
{

  transfer_t x = {
    . step1 = step1,
    . step2 = step2,
    . cal_set_value = cal_set_value
  };

  app_transfer( app, &x );
}







#if 0


void app_cal_b( app_t *app)
{

  _mode_t *mode = app->mode;
  decode_t *data = app->data;
  cal_t *cal = app->cal;

  printf("--------\n");
  printf("cal_b\n");

  assert(cal->w);

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
    above code - should be able to just call decode_update_data()  directly
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

#endif



/*

  questions
    1. whether to use cal_t structure directly.
        or communicate values value - through the range_t structure.
        - if use range_t then can add/ range specific bounds checks. etc.
        - but this code is already range / cal specific.

        BUT we do not want to repeat cal constants.
          eg. many values for amp. gain  are shared. for lts, daq. and dcv.
          in cal structure.  so i think we dont use the range.
          may be on ohms.

    2. whether to have the gain ranges - scale according to the 10V. range b.  or else directly from adc adjusted_sum.

*/







#if 0





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







