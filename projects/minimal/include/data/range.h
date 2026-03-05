

#pragma once

/*
  the arrays should reference from app.
  although the cal values ....

  arrays for   b, a,  name, unit,    and perhaps to put the mode into position.

*/

typedef struct _mode_t _mode_t;
typedef struct cal_t  cal_t;





typedef struct range_t
{
  // unsigned  id;     // same as index in range_t [] array
  // const char *repl_name;

  const char *name;

  const char  *arg;

  const char *unit;

  ///////////////////

  /*
    important.
    instead of passing the arg

    It may make sense to pass the range as the first argument...


  */

  void (*mode_f)( _mode_t *, const char *arg );


  //  the same way the mode function  sets the mode.   the cal functino can scale/adjust the value to a reading.

  // convert value to a reading according to calibration
  // TODO consider rename to cal_reading.  eg. turning data into a reading
  // cal coefficients belong on cal.
  double (*cal_f)( const cal_t *, const char *arg, double value);


  // sentinel need to encode whether it is top or bottom.
  // otherwise we do not know which direction to move from...
  // so needs to be enum/int.
  // or have  two fields a top/ bottom or up/down  limmit
  bool sentinal;
  // bool down_sentinal;

  ///////////////////


} range_t;






int32_t range_get_idx( range_t *ranges, size_t sz, const char *name, const char *arg );


// forward declaration
// do not use directly... use app->range_values instead
// may be better manage this with a  reset() singleton.
extern range_t  init_range_values[];

extern const size_t init_ranges_sz ;






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


