/*
  range.h sets mode.  which is a high-level unrelated to data handling.
  consider move out of src/data dir
  but everything else - formatting, range lookup, etc. are data orientated
*/

#pragma once



#include <stdbool.h>

/*
  the arrays should reference from app.
  although the cal values ....

  arrays for   b, a,  name, unit,    and perhaps to put the mode into position.

*/

typedef struct _mode_t _mode_t;
typedef struct cal_t  cal_t;



#define RANGE_MAGIC 34523418



/*
  using struct eases syntax and we use it in enough places
  justified
  handle all parts of the formatted value separately.
  since the way we use depends on the output device, layout, font width etc,
  tft, vfd, repl, buffer statistic,
  ----
  should be format_reading
*/


typedef struct format_val_t
{
  // this could be computed once, and stored in data_t

  // unsigned ndigits;
  unsigned leading;

  // formatted value
  char s[ 100];
  // unit multiplier
  char m;
  // unit
  char u[ 10];

  // convenience, all fields formatted
  char all[ 120];

} format_val_t;



typedef struct range_t range_t;

struct range_t
{
  // unsigned  id;     // same as index in range_t [] array
  // const char *repl_name;

  uint32_t    magic;

  /*
    public name and arg - both used for search critiera
    should probably move the sentinal indicators to start.
  */
  const char  *name;

  const char  *arg;

  // sentinel encodes range limits - either top or bottom.
  bool bot_sentinal;
  bool top_sentinal;

  // bool ignore;


  ///////////////////

  // consider make a function. it doefor
  // const char  *unit;

  /*
    can always use a structure to pass arguments,
    if argument passing becomes to onerous
  */

  // set the range, by manipulating the mode
  void (*range_set_mode)( const range_t *range, _mode_t *,  bool range_10Meg);


  // TODO. renam the range_reading value to count_norm.

  // convert normalized count to a reading according to calibration
  double (*range_reading_convert)( const range_t *range, const cal_t *, double adc_count_sum_norm);


  // convert value to string

  void (*range_reading_format)( const range_t *range, format_val_t *fval, unsigned ndigits, double value);

  // void (*range_reading_format)( const range_t *range, char *s, size_t sz, char *m, char *u, unsigned ndigits, double value);
  // void (*range_reading_format)( const range_t *range, char *s, size_t sz, unsigned ndigits, double value);

  /* could have a range_cal_set()  function ...
      to hide all the detail of writing the cal co-efficientss
      when using the calibration routines.
      but suspect would obscure too much what is going on.
  */
  // void (*range_cal_set)( range_t *range, const cal_t *, double b, double a);


  // autoranging predicate test here also
  int32_t (*range_ar_predicate)( range_t *range, /*status_reg, */ double v);

  //
};





size_t ranges_init( range_t *ranges, size_t sz);

