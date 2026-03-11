

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

  const char  *name;

  const char  *arg;

  const char  *unit;

  ///////////////////

  /*
    we can pass the arguments
    using a structure,
    if arg handling becomes too annoying

  */

  void (*range_set_mode)( const range_t *range, _mode_t * /*,  bool range_10Meg*/ );


  //  the same way the mode function  sets the mode.   the cal functino can scale/adjust the value to a reading.

  // convert value to a reading according to calibration
  // TODO consider rename to range_cal_reading.  eg. turning data into a reading
  // range_cal_convert() // range_cal_reading()
  // and range_cal_set( coeff)
  double (*range_reading)( const range_t *range, const cal_t *, double value);


  /* could have a range_cal_set  function here...
      to hide all the underlying variable writing to the cal structure
      from the calibration routines.
      but would obscure what is going on.
  */

  // void (*range_cal_set)( range_t *range, const cal_t *, double b, double a);

  /* put autoranging predicate test here also
      return value 1,0,-1  for up,stay,no change.
  */
  int32_t (*range_predicate)( range_t *range, /*status_reg, */ double v);


  // sentinel to encode range limit - either top or bottom.
  bool bot_sentinal;
  bool top_sentinal;



} range_t;






int32_t range_get_idx( range_t *ranges, size_t sz, const char *name, const char *arg );


// forward declaration
// do not use directly... use app->range_values instead
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


