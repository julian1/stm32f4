

#pragma once

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


  void (*mode_f)( _mode_t *, const char *arg );


  // convert value to a reading according to calibration
  // TODO consider rename to reading.  eg. turning data into a reading
  double (*cal_f)( const cal_t *, const char *arg, double value);

  ///////////////////

#if 0
  // cal coeffs.  as experiment. instead of using a separate structure
  double b;
  double a;
#endif

} range_t;






int32_t range_get_idx( range_t *ranges, size_t sz, const char *name, const char *arg );


// forward declaration
// do not use this directly... instead use app->range_values
extern range_t  init_range_values[];

extern const size_t init_ranges_sz ;



