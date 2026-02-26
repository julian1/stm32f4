

#pragma once


#define DCV_10_REF   0          // non derived
#define DCV_10       1          // with b + a - for thermal EMF.
#define DCV_1        2           // derived
#define DCV_01       3           // 100mV

#define MAX_RANGE   20

// the arrays should reference from app.
// although the cal values ....

// arrays for   b, a,  name, unit,    and perhaps to put the mode into position.


typedef struct _mode_t _mode_t;


typedef struct range_t
{

  // const char *repl_name;
  const char *name;         // for repl and display
  const char *unit;

  void (*f)( _mode_t *);

  // cal coeffs.  as experiment. instead of using a separate structure
  double b;
  double a;


} range_t;



// forward declaration
// do not use this directly... instead use app->range_values
extern range_t  init_range_values[];





