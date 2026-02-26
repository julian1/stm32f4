
#include <stddef.h>

#include <data/ranges.h>
#include <mode.h>

// arrays for   b, a,  name, unit,    and perhaps to put the mode into position.



// are we going to use a static initializer
// and how do we pass to app.
// the internal state


// in mode_t we may want to separate normal state...

/*
  const char *name;
  const char *unit;

  void (*f)( _mode_t *);
*/


/*
  we may want to remove the initial state...
  or move it here
  -------

  extra statec

  whether 10Meg. impedance persists
    through range change

    can add an extra variable in the mode...

*/


static void dcv_10_ref( _mode_t *mode )
{
  /*
    i think we should rename dcv_10_ref
    -----
    the same as dcv10. except need to distingusih for cal.
    because will have no offset.
  */

  // normal sample acquisition/adc operation
  mode_reg_cr_set( mode, MODE_SA_ADC);
  mode_az_set(mode, "ch2" );

  // just measure internal ref...
  mode_ch2_set_ref( mode);
}


static void dcv_10( _mode_t *mode )
{
  mode_reg_cr_set( mode, MODE_SA_ADC);
  mode_az_set(mode, "ch1" );
  mode->second.K402 = SR_SET;
}


static void dcv_1( _mode_t *mode )
{
  dcv_10( mode);
  mode_gain_set(mode, 10);
}


static void dcv_01( _mode_t *mode )
{
  dcv_10( mode);
  mode_gain_set(mode, 100);
}


/*
  we may need a calibration range...
  for front/ v rear terminals.
  ---------

  - consider -  placing the cal structure here. rather than just sharing the index.
  - on cal load. we write the cal coefficients.

  - we pass extra data .  butthere is no reason. into this static structure.
  - it could simplify.
  ----------------

  eg. the loading from flash would be interspersed....
    and the cal_w would be a separate variable..

  ---------------------------

  OK.

  Now the big thing....

  Is we pass the ranges structure the data_t ...
  to be able to do formatting.

  With an index...

  // the active range
  range_idx.

  this would be a lot simpler...

  Same as how Microsoft Office structure worked.  with the active

*/

range_t init_range_values[ MAX_RANGE] = {

  {
    "DCV 10 REF",   // DCV_10_REF
    "V",
    dcv_10_ref,
    0, 0
  },
  {
    "DCV 10",         // DCV_10
    "V",
    dcv_10,
    0, 0
  },
  {
    "DCV 1",         // DCV_1
    "V",
    dcv_1,
    0, 0
  },
  {
    "DCV 0.1",         // DCV_01
    "V",
    dcv_01,
    0, 0
  }




};



