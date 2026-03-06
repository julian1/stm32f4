

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


  void (*range_set_mode)( range_t *range, _mode_t * );


  //  the same way the mode function  sets the mode.   the cal functino can scale/adjust the value to a reading.

  // convert value to a reading according to calibration
  // TODO consider rename to cal_reading.  eg. turning data into a reading
  double (*range_reading)( range_t *range, const cal_t *, double value);

  // perhaps autoranging test could be added here
  // bool (*range_outofrange)( range_t *range, status_reg sr);


  // sentinel to encode range limit - either top or bottom.
  // else needs to be enum/int.
  bool bot_sentinal;
  bool top_sentinal;

  // EXTR.  if top and bottom. then only one iterable range availableset.

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


