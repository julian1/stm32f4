

#pragma once

/*
  the arrays should reference from app.
  although the cal values ....

  arrays for   b, a,  name, unit,    and perhaps to put the mode into position.

*/

typedef struct _mode_t _mode_t;
typedef struct cal_t  cal_t;



#define RANGE_MAGIC 34523418


typedef struct range_t
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
  const char  *unit;

  /*
    can always use a structure to pass arguments,
    if argument passing becomes to onerous
  */

  // set the range, by manipulating the mode
  void (*range_set_mode)( const range_t *range, _mode_t *,  bool range_10Meg);


  // TODO. renam the range_reading value to count_norm.

  // convert normalized count to a reading according to the calibration
  double (*range_reading_convert)( const range_t *range, const cal_t *, double count_sum_norm);


  // convert value to string
  void (*range_format_value)( const range_t *range, char *s, size_t sz, unsigned ndigits, double value);

  /* could have a range_cal_set()  function ...
      to hide all the detail of writing the cal co-efficientss
      when using the calibration routines.
      but suspect would obscure too much what is going on.
  */
  // void (*range_cal_set)( range_t *range, const cal_t *, double b, double a);


  // autoranging predicate test here also
  int32_t (*range_predicate)( range_t *range, /*status_reg, */ double v);

  //


} range_t;






int32_t range_get_idx( range_t *ranges, size_t sz, const char *name, const char *arg );


// forward declaration
// do not use global ... use app->range_values instead
// may be better manage this with a  reset() singleton.
extern range_t  range_init_values[];

extern const size_t range_init_sz ;






#if 0

// DCV_INITIAL
#define DCV_REF    0              // default. nominal. for 7.1V. reference
#define DCV_10        1           // with b + a - for thermal EMF.
#define DCV_1         2           // derived
#define DCV_01        3           // 100mV
#define DCV_001       4           // 10mV
#define DCV_100       5           // 100V
#define DCV_1000      6           // 1000
#define TEMP_          7           // single range

#endif


