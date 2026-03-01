

#pragma once


// DCV_INITIAL
#define DCV_REF    0              // default. nominal. for 7.1V. reference
#define DCV_10        1           // with b + a - for thermal EMF.
#define DCV_1         2           // derived
#define DCV_01        3           // 100mV
#define DCV_001       4           // 10mV
#define DCV_100       5           // 100V
#define DCV_1000      6           // 1000
#define TEMP_          7           // single range



/*
  the arrays should reference from app.
  although the cal values ....

  arrays for   b, a,  name, unit,    and perhaps to put the mode into position.

*/

typedef struct _mode_t _mode_t;


typedef struct range_t
{
  unsigned  id;     // same as index in range_t [] array

  // const char *repl_name;
  const char *name;         // for repl and display
  const char *unit;

  ///////////////////

  /*
      should we pass the 10meg. impedance state here...
      i think it would be better.
      mode represents spi writable state.
  */
  void (*f)( _mode_t *);

  ///////////////////

  // cal coeffs.  as experiment. instead of using a separate structure
  // the amplifier  gain. should be included in b.
  double b;

  // offset/intercept. thermal EMF.
  double a;

} range_t;



// forward declaration
// do not use this directly... instead use app->range_values
extern range_t  init_range_values[];

extern const size_t init_ranges_sz ;



