


#include <stddef.h>
#include <assert.h>

#include <mode.h>
#include <data/range.h>

#include <lib2/util.h>   // UNUSED()


/*
  the amplifier  gain. should be included in b.

  -----
  state that is not updated with range  change

    - LTS.
    - STS
    - adc/sa. parameters not.  eg. line-freq.
    - 10Meg. impedance not.


      done have to do all the state.  for temp.  but measuring
      - ACAL can just propagate the internal ref. calibration.

    - temp - only one value.  unit is degC.  would likely use  100x.  gain, anyway.
    - ref  - only has 10V/1x gain range.
    - daq  - would have 10,1,0.1,   no HV. inputs.

  - so it is only really the daq. that needs ranges.
*/



/*
  the initial state, may want to move here

  10Meg. impedance setting needs to persists through range change.
    This is a high-level concept. should be applied at range change.

      we could add the variable in the mode...

*/

/*
  recognize that we dont seek to cover all states.
  all states, for self-diagnostics, acal etc

  //////////
  // actually we dont even care about not using default here..
  // lts,sts adc,

*/


/*
  instead of have this extra state

*/

static void partial_reset( _mode_t *mode)
{
  assert(mode);


  _mode_t tmp = *mode;

  // reset to known  good state
  mode_reset( mode);

  // keep adc parameters eg. nplc
  mode->adc = tmp.adc;


  // keep the LTS setting , AG
  // mode->second.U1003 = tmp.second.U1003;
  // mode->second.U1012 = tmp.second.U1012;

  // daq sts.
}




static void dcv_ref( _mode_t *mode, bool _10meg_impedance)
{
  /*
    internal ref.  not a particularly useful function
    only 1x gain applies
  */

  UNUSED( _10meg_impedance );
  partial_reset( mode);

  mode_reg_cr_set( mode, MODE_SA_ADC);
  mode_az_set(mode, "ch2" );
  mode_ch2_set_ref( mode);
}



static void dcv_10( _mode_t *mode, bool _10meg_impedance)
{

  partial_reset( mode);

  mode_reg_cr_set( mode, MODE_SA_ADC);
  mode_az_set(mode, "ch1");
  mode->first.K402 = SR_SET;

  // apply impedance
  mode->first.K403 = _10meg_impedance ? SR_SET : SR_RESET;
}

static void dcv_1( _mode_t *mode, bool _10meg_impedance)
{
  dcv_10( mode, _10meg_impedance);
  mode_gain_set(mode, 10);
}

static void dcv_01( _mode_t *mode, bool _10meg_impedance)
{
  dcv_10( mode, _10meg_impedance);
  mode_gain_set(mode, 100);
}



static void dcv_100( _mode_t *mode, bool _10meg_impedance)
{
  UNUSED(_10meg_impedance);

  partial_reset( mode);

  mode_reg_cr_set( mode, MODE_SA_ADC);
  mode_az_set(mode, "ch2");
  mode_ch2_set_dcv_div( mode);
  mode_gain_set( mode, 10);

  // main divider on
  mode->first.K403 = SR_SET;
}


static void dcv_1000( _mode_t *mode, bool _10meg_impedance)
{
  dcv_100( mode, _10meg_impedance);
  mode_gain_set( mode, 1);
}


static void temp( _mode_t *mode, bool _10meg_impedance)
{
  UNUSED(_10meg_impedance);
  partial_reset( mode);


  mode_reg_cr_set( mode, MODE_SA_ADC);
  mode_az_set(mode, "ch2" );
  mode_ch2_set_temp( mode);       // should get rid of accessor and manage low level details of state setup here
  // mode_gain_set( mode, 1); could use a different gain

}






range_t init_range_values[ MAX_RANGE] = {

  { DCV_REF,    "DCV REF",    "V", dcv_ref, 0, 0 },
  { DCV_10,     "DCV 10",     "V", dcv_10, 0, 0 },
  { DCV_1,      "DCV 1",      "V", dcv_1, 0, 0 },
  { DCV_01,     "DCV 0.1",    "V", dcv_01, 0, 0 },
  { DCV_001,    "DCV 0.01",   "V", NULL, 0, 0 },
  { DCV_100,    "DCV 100",    "V", dcv_100, 0, 0 },
  { DCV_1000,   "DCV 1000",   "V", dcv_1000, 0, 0 },

  { TEMP_,      "TEMP",       "degC", temp, 0, 0 }


};





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

  this would be a lot simpler...

  Like how Microsoft Office - internal COM/  works - with arrays and active index.

*/

