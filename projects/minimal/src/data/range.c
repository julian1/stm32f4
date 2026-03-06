


#include <stdio.h>
// #include <stddef.h>     // NULL
#include <assert.h>
#include <strings.h>      // strcasecmp

#include <mode.h>
#include <data/cal.h>
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

  mode_reset() is different from the partial state reset required to change ranges

*/

static void partial_reset( _mode_t *mode)
{
  assert(mode);


  _mode_t tmp = *mode;
  UNUSED(tmp);

  // reset to known  good state
  mode_reset( mode);


  // TODO. change this to just persist sa and adc complete structure ...
  // adc. is ok.
  // but the precharge stuff changes.

  // persist adc parameters, aperture and reset period
  mode->adc = tmp.adc;

  // persist the sa trigger-delay,  and precharge period
  mode->sa.p_clk_count_trig_delay   = tmp.sa.p_clk_count_trig_delay;
  mode->sa.p_clk_count_precharge    = tmp.sa.p_clk_count_precharge;

  // persist noaz flag
  mode->reg_cr.sa_p_noaz = tmp.reg_cr.sa_p_noaz;

  // persist the 10meg. impedance flag
  mode->reg_cr._10meg_impedance =  tmp.reg_cr._10meg_impedance;


  // persist the daq input selection muxes
  // this is silly having serial,second states for 4094.
  mode->serial.U1009 =  tmp.serial.U1009;
  mode->serial.U1010 =  tmp.serial.U1010;


  // persist sts datc
  // Not. sure .   we want to maintain... across tests.
  mode->mdac1_val     = tmp.mdac1_val;

  // persist LTS state
  // mode->second.U1003 = tmp.second.U1003;
  // mode->second.U1012 = tmp.second.U1012;

}









// static void mode_ref( _mode_t *mode, const char *arg)
static void mode_ref( range_t *range, _mode_t *mode )
{
  /*
    internal ref.  not a particularly useful function
    only 1x gain applies
  */

  partial_reset( mode);

  reg_cr_mode_set( &mode->reg_cr, MODE_SA_ADC);
  mode_az_set(mode, "ch2" );


  if(strcasecmp( range->arg, "lo") == 0) {

    mode_ch2_set( mode, "ref-lo");
  } else {

    mode_ch2_set( mode, "ref");
  }

}




// static void mode_temp( _mode_t *mode, const char *arg)
static void mode_temp( range_t *range, _mode_t *mode)
{
  // consider - get rid of accessor and manage low level details of state setup here
  // mode_gain_set( mode, 1); could use a different gain
  UNUSED( range);

  partial_reset( mode);

  reg_cr_mode_set( &mode->reg_cr, MODE_SA_ADC);
  mode_az_set(mode, "ch2" );
  mode_ch2_set( mode, "temp");
}




// static void mode_lts( _mode_t *mode, const char *arg)
static void mode_lts( range_t *range, _mode_t *mode)
{
  partial_reset( mode);

  reg_cr_mode_set( &mode->reg_cr, MODE_SA_ADC);
  mode_az_set(mode, "ch2" );
  mode_ch2_set( mode, "lts");


  if(strcasecmp( range->arg, "10") == 0)
    ;
  else if(strcasecmp( range->arg, "1") == 0)
    mode_gain_set(mode, 10);

  else if(strcasecmp( range->arg, "0.1") == 0)
    mode_gain_set(mode, 100);

  else if(strcasecmp( range->arg, "0.01") == 0)
    mode_gain_set(mode, 1000);

  else
    assert( 0);
}



// static void mode_dcv( _mode_t *mode, const char *arg )
static void mode_dcv( range_t *range, _mode_t *mode)
{
/*
  instead of passing string arg.
  why not pass the range_t structure.
*/

  partial_reset( mode);

  reg_cr_mode_set( &mode->reg_cr, MODE_SA_ADC);
  mode_az_set(mode, "ch1");
  mode->serial.K402 = SR_SET;

  // apply impedance
  mode->serial.K403 = mode->reg_cr._10meg_impedance ? SR_SET : SR_RESET;


  if(strcasecmp( range->arg, "10") == 0)
    ;

  else if(strcasecmp( range->arg, "1") == 0)
    mode_gain_set( mode, 10);

  else if(strcasecmp( range->arg, "0.1") == 0)
    mode_gain_set( mode, 100);

  else if(strcasecmp( range->arg, "0.01") == 0)
    mode_gain_set( mode, 1000);

  else
    assert( 0);
}



// static double cal_dcv( const cal_t *cal, const char *arg, double value)
static double cal_dcv( range_t *range, const cal_t *cal, double value)
{
  // eg. with input terminl offset

  if(strcasecmp( range->arg, "10") == 0) {

    return cal->b * value;
    // add front or rear terminal offset.
  }
  else
    assert( 0);

  return value;
}


// static double cal_normal( const cal_t *cal, const char *arg, double value)
static double cal_normal( range_t *range, const cal_t *cal, double value)
{
  // for ref and lts.
  // consider

  if(strcasecmp( range->arg, "") == 0       // ref has no argument.
    || strcasecmp( range->arg, "10") == 0   // lts, daq etc.
    || strcasecmp( range->arg, "LO") == 0   // ref-lo
  ) {

    return cal->b * value;
  }

  else if(strcasecmp( range->arg, "1") == 0)
  {
    // So if we set the gain...
    // OK. kkkkkkkk


    return cal->b * value;

  }
  else
    assert( 0);

  return value;
}




// static double cal_temp( const cal_t *cal, const char *arg, double value)
static double cal_temp( range_t *range, const cal_t *cal, double value)
{
  UNUSED( range);
  return cal->b * value * 100;
}



// how to manage optional flags eg. is not clear
// can inject into mode or cal... or
//



range_t init_range_values[] = {

  {   "REF",    "",     "V",  mode_ref,   cal_normal,   true},
  {   "REF",    "LO",   "V",  mode_ref,   cal_normal,  true  },

  {   "DCV",    "1000", "V",  mode_dcv,   cal_dcv,  true },
  {   "DCV",    "100",  "V",  mode_dcv,   cal_dcv,  false  },
  {   "DCV",    "10",   "V",  mode_dcv,   cal_dcv,  false },
  {   "DCV",    "1",    "V",  mode_dcv,   cal_dcv,  false },
  {   "DCV",    "0.1",  "V",  mode_dcv,   cal_dcv,  false  },
  {   "DCV",    "0.01", "V",  mode_dcv,   cal_dcv,  true },

  {   "TEMP",   "",     "°C", mode_temp,  cal_temp,  true },

  {   "LTS",    "10",   "V",  mode_lts,   cal_normal,  true },       // LTS or DCV LTS.
  {   "LTS",    "1",    "V",  mode_lts,   cal_normal,  true },
  {   "LTS",    "0.1",  "V",  mode_lts,   cal_normal,  true },
  {   "LTS",    "0.01", "V",  mode_lts,   cal_normal,  true }


};

const size_t init_ranges_sz = ARRAY_SIZE( init_range_values );




int32_t range_get_idx( range_t *ranges, size_t sz, const char *name, const char *arg )
{
  // TODO consider rename  lookup..  range_find_idx.

  for( size_t i = 0; i < sz; ++i) {

    range_t *range = & ranges[ i];

    if( strcasecmp( name, range->name) == 0
      && strcasecmp( arg, range->arg) == 0
    ) {

      printf("found %s-%s\n", range->name, range->arg );
      return i ;
    }
  }

  return -1;
}



/*
Passing a NULL pointer as an argument to strcasecmp results in undefined
behavior. The function expects valid, null-terminated strings as its arguments.
*/



#if 0



static void dcv_10( _mode_t *mode)
{

  partial_reset( mode);

  reg_cr_mode_set( &mode->reg_cr, MODE_SA_ADC);
  mode_az_set(mode, "ch1");
  mode->serial.K402 = SR_SET;

  // apply impedance
  mode->serial.K403 = mode->reg_cr._10meg_impedance ? SR_SET : SR_RESET;
}

static void dcv_1( _mode_t *mode)
{
  dcv_10( mode);
  mode_gain_set(mode, 10);
}

static void dcv_01( _mode_t *mode)
{
  dcv_10( mode);
  mode_gain_set(mode, 100);
}



static void dcv_100( _mode_t *mode)
{

  partial_reset( mode);

  reg_cr_mode_set( &mode->reg_cr, MODE_SA_ADC);
  mode_az_set(mode, "ch2");
  mode_ch2_set( mode, "dcv-div");

  mode_gain_set( mode, 10);

  // main divider on
  mode->serial.K403 = SR_SET;
}


static void dcv_1000( _mode_t *mode)
{
  dcv_100( mode);
  mode_gain_set( mode, 1);
}


#endif



#if 0
  // check range idx matches id.
  // consider factor this out to range.c
  for( unsigned i = 0; i < app.ranges_sz ; ++i )  {

    range_t *range = &app.ranges[ i];
    assert( range->id == i);
    assert( range->name);
  }
#endif




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

