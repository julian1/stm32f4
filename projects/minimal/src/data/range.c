


#include <stdio.h>
// #include <stddef.h>     // NULL
#include <assert.h>
#include <strings.h>      // strcasecmp
#include <math.h>         // fabs

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

static void mode_partial_reset( _mode_t *mode)
{
  // rename this.  as mode_range_reset...

  assert(mode && mode->magic == MODE_MAGIC);

  // copy the mode
  _mode_t tmp = *mode;
  UNUSED(tmp);

  // reset mode,
  mode_reset( mode);


  // persist adc parameters, aperture and reset period
  mode->adc = tmp.adc;

  // persist the sa trigger-delay,  and precharge period
  // the channel seqn and seq, will be set by the range
  mode->sa.p_trig_delay   = tmp.sa.p_trig_delay;
  mode->sa.p_precharge    = tmp.sa.p_precharge;

  // persist noaz flag
  mode->reg_cr.sa_p_noaz = tmp.reg_cr.sa_p_noaz;



  // persist the daq input selection muxes
  mode->serial.U1009  =  tmp.serial.U1009;
  mode->serial.U1010  =  tmp.serial.U1010;

  // persist sts datc value
  mode->mdac1_val     = tmp.mdac1_val;

  // persist LTS source muxes
  mode->serial.U1003 = tmp.serial.U1003;
  mode->serial.U1012 = tmp.serial.U1012;


  // 10meg. impedance flag. is persisted by mode...
  // mode->range_10Meg  = tmp.range_10Meg;
}




/*

  think the easier way to manage ...
    write the mode.

    and write an array  with co-efficients for calculating the value.
    used for the computing the value..

  - can just inject the scaling array... from app. into the decode_update_data()
      this way decode_update_data() also does not need the current range.

*/






static void range_lo( const range_t *range, _mode_t *mode, bool range_10Meg)
{
  UNUSED(range_10Meg);
  // sample ref-lo switched on input hi and lo mux.
  assert(range && range->magic == RANGE_MAGIC);
  assert(mode && mode->magic == MODE_MAGIC);

  assert(strcasecmp( range->name, "lo") == 0);

  mode_partial_reset( mode);
  cr_mode_set( &mode->reg_cr, MODE_SA_ADC);
  sa_az_set( &mode->sa, "ch2" );
  mode_ch2_set( mode, "ref-lo");

  if(strcasecmp( range->arg, "10") == 0)
    mode_gain_set(mode, 1);
  else if(strcasecmp( range->arg, "1") == 0)
    mode_gain_set(mode, 10);
  else if(strcasecmp( range->arg, "0.1") == 0)
    mode_gain_set(mode, 100);
  else if(strcasecmp( range->arg, "0.01") == 0)
    mode_gain_set(mode, 1000);
  else
    assert(0);
}


static void range_lo2( const range_t *range, _mode_t *mode, bool range_10Meg)
{
  // sample star-lo switched straight into the azmux
  // for both values.
  UNUSED(range_10Meg);

  assert(range && range->magic == RANGE_MAGIC);
  assert(mode && mode->magic == MODE_MAGIC);

  assert(strcasecmp( range->name, "lo2") == 0);

  mode_partial_reset( mode);
  cr_mode_set( &mode->reg_cr, MODE_SA_ADC);

  sa_az_set( &mode->sa, "0" ); // sample A400 gnd

  if(strcasecmp( range->arg, "10") == 0)
    mode_gain_set(mode, 1);
  else if(strcasecmp( range->arg, "1") == 0)
    mode_gain_set(mode, 10);
  else if(strcasecmp( range->arg, "0.1") == 0)
    mode_gain_set(mode, 100);
  else if(strcasecmp( range->arg, "0.01") == 0)
    mode_gain_set(mode, 1000);
  else
    assert(0);

}





static void range_ref( const range_t *range, _mode_t *mode, bool range_10Meg)
{
  UNUSED(range_10Meg);
  assert(range && range->magic == RANGE_MAGIC);
  assert(mode && mode->magic == MODE_MAGIC);

  /*
    internal ref.  not a particularly useful function
    only 1x gain applies
  */

  assert(strcasecmp( range->name, "ref") == 0);

  mode_partial_reset( mode);
  cr_mode_set( &mode->reg_cr, MODE_SA_ADC);
  sa_az_set( &mode->sa, "ch2" );
  mode_ch2_set( mode, "ref");
}



static void range_temp( const range_t *range, _mode_t *mode, bool range_10Meg)
{
  UNUSED(range_10Meg);
  assert(range && range->magic == RANGE_MAGIC);
  assert(mode && mode->magic == MODE_MAGIC);

  mode_partial_reset( mode);
  cr_mode_set( &mode->reg_cr, MODE_SA_ADC);
  sa_az_set( &mode->sa, "ch2" );
  mode_ch2_set( mode, "temp");
}



static void range_lts( const range_t *range, _mode_t *mode, bool range_10Meg)
{
  UNUSED(range_10Meg);
  assert(range && range->magic == RANGE_MAGIC);
  assert(mode && mode->magic == MODE_MAGIC);

  mode_partial_reset( mode);

  cr_mode_set( &mode->reg_cr, MODE_SA_ADC);
  sa_az_set( &mode->sa, "ch2" );
  mode_ch2_set( mode, "lts");


  if(strcasecmp( range->arg, "10") == 0)
    mode_gain_set(mode, 1);
  else if(strcasecmp( range->arg, "1") == 0)
    mode_gain_set(mode, 10);
  else if(strcasecmp( range->arg, "0.1") == 0)
    mode_gain_set(mode, 100);
  else if(strcasecmp( range->arg, "0.01") == 0)
    mode_gain_set(mode, 1000);
  else
    assert( 0);
}



static void range_dcv( const range_t *range, _mode_t *mode, bool range_10Meg)
{
  assert(range && range->magic == RANGE_MAGIC);
  assert(mode && mode->magic == MODE_MAGIC);

  assert( strcasecmp( range->name, "dcv") == 0);

  mode_partial_reset( mode);

  cr_mode_set( &mode->reg_cr, MODE_SA_ADC);

  sa_az_set( &mode->sa, "ch1");

  // close relay - select external terminal input
  mode->serial.K402 = SR_SET;

  printf("dcv range_10Meg is %u\n", range_10Meg );

  if(strcasecmp( range->arg, "1000") == 0) {

    mode->serial.K403 = SR_SET;
    mode_gain_set( mode, 1);
    mode_ch2_set( mode, "div");
    sa_az_set( &mode->sa, "ch2" );
  }
  else if(strcasecmp( range->arg, "100") == 0) {

    mode->serial.K403 = SR_SET;
    mode_gain_set( mode, 10);
    mode_ch2_set( mode, "div");
    sa_az_set( &mode->sa, "ch2" );
  }
  else if(strcasecmp( range->arg, "10") == 0) {

    mode->serial.K403 = range_10Meg ? SR_SET : SR_RESET;
    mode_gain_set( mode, 1);
    sa_az_set( &mode->sa, "ch1" );
  }
  else if(strcasecmp( range->arg, "1") == 0) {

    mode->serial.K403 = range_10Meg ? SR_SET : SR_RESET;
    mode_gain_set( mode, 10);
    sa_az_set( &mode->sa, "ch1" );
  }
  else if(strcasecmp( range->arg, "0.1") == 0) {

    mode->serial.K403 = range_10Meg ? SR_SET : SR_RESET;
    mode_gain_set( mode, 100);
    sa_az_set( &mode->sa, "ch1" );
  }
  else if(strcasecmp( range->arg, "0.01") == 0) {

    mode->serial.K403 = range_10Meg ? SR_SET : SR_RESET;
    mode_gain_set( mode, 1000);
    sa_az_set( &mode->sa, "ch1" );
  }
  else
    assert( 0);
}



///////////////////////////

/* we may need to pass whether the front or rear terminal inputs
  are active.
*/

static double range_reading_normal( const range_t *range, const cal_t *cal, double count_sum_norm)
{
  /*
    consider change name cal_internal.  or cal_nooffset.
    where we dont consider input terminal
  */

  assert(range && range->magic == RANGE_MAGIC);
  // for ref and lts.
  // consider

  if(strcasecmp( range->arg, "") == 0       // for ref/ temp has no argument.
    || strcasecmp( range->arg, "10") == 0   // lts, daq etc.
  ) {

    return cal->b * count_sum_norm;
  }
  else if(strcasecmp( range->arg, "1") == 0)
  {
    // may want default count_sum_norms
    // or express as or cal->b * cal->b10.
    return cal->b10 * count_sum_norm;
  }
  else if(strcasecmp( range->arg, "0.1") == 0)
  {
    return cal->b100 * count_sum_norm;
  }
  else if(strcasecmp( range->arg, "0.01") == 0)
  {
    return cal->b1000 * count_sum_norm;
  }
  else
    assert( 0);

  // compiler
  return 0;
}




static double range_reading_dcv( const range_t *range, const cal_t *cal, double count_sum_norm)
{
  assert(range && range->magic == RANGE_MAGIC);
  // eg. with input terminl offset


  if(strcasecmp( range->arg, "1000") == 0) {

    // TODO add front or rear terminal offset.
    // but not if using internal...
    return cal->div1000 * count_sum_norm;
  }
  if(strcasecmp( range->arg, "100") == 0) {

    return cal->div100 * count_sum_norm;
  }
  else  {

    // TODO add offset
    return range_reading_normal( range, cal, count_sum_norm);
  }

  // compiler
  return 0;
}




static double range_reading_temp( const range_t *range, const cal_t *cal, double count_sum_norm)
{
  assert(range && range->magic == RANGE_MAGIC);

  // can delegate here
  // lm35d
  // return cal->b * count_sum_norm * 100;

  return range_reading_normal( range, cal, count_sum_norm) * 100.f;
}







static int32_t range_dcv_pred( range_t *range, /*reg_status */ double v)
{
/* autoranging predicate test here also
      return value 1,0,-1  for up,stay,no change.
  */

  // we could pass some extra state here.
  // if needed.

  assert(range && range->magic == RANGE_MAGIC);

  assert( strcasecmp( range->name, "dcv") == 0);

  // we just assume > 10 , then switch the range.
  // So. use the count_sum_norm. * b;
  // rather than the specific range.
  // or else we an use arg. to  work out the value we want.

  UNUSED(v);

  return 0;
}


///////////////////////////////////


static void format_value( char *s, size_t n,  unsigned total, unsigned leading, double value )
{
  printf("%u\n", leading);

  // +1 for dot and +1 for sign,
  total += 2;
  leading += 2;
  int trailing = total  - leading ;

  snprintf(s, n, "%0*.*f", total, trailing, fabs(value));

  // both 34470a, dmm7510 preserve leading positive sign in display
  s[ 0] = value >= 0 ? '+' : '-';
}




static void range_string( const range_t *range, char *s, size_t sz, unsigned ndigits, double value)
{
  // rename range_value_string.
  //

  assert(range && range->magic == RANGE_MAGIC);


  unsigned nprefix = 0;

  if(strcasecmp( range->arg, "1000") == 0) {
    nprefix = 4;
  }
  else if(strcasecmp( range->arg, "100") == 0) {
    nprefix = 3;
  }
  else if(strcasecmp( range->arg, "") == 0
    || strcasecmp( range->arg, "10") == 0
  ) {
    nprefix = 2;
  }
  else if(strcasecmp( range->arg, "1") == 0)
  {
    nprefix = 1;
  }
  else if(strcasecmp( range->arg, "0.1") == 0)
  {
    nprefix = 3;
    value *= 1000.;
    // can change unit to "mV"
    // have to add 'm'
  }
  else if(strcasecmp( range->arg, "0.01") == 0)
  {
    nprefix = 2;
    value *= 1000.;
    // have to add 'm'
  }
  else
    assert( 0);


  format_value( s, sz,  ndigits, nprefix, value);

  // consider can encode full unit here. 'mV'
  // no. use a separate function

}



/*
  34401a.
    up is higher voltage.
    down is lower
*/

// how to manage optional flags eg. is not clear
// can inject into mode or cal... or
//

/*
  EXTR> to include 20V,2V,200mV. in autorannging.
  can repeat the dcv range sequence with the values includeds.

  just need to change the initial lookup  in app_repl_range()0 etc.
  to find which to start at. control with a boolean.

*/

range_t range_init_values[] = {

  //              name    arg     sentinels         unit  set_mode    convert to reading    format          autorange predicate
  { RANGE_MAGIC,  "REF",  "",     true,   true,     "V",  range_ref,  range_reading_normal, range_string,   NULL,             },

  { RANGE_MAGIC,  "LO",   "0.01", true,   false,    "mV", range_lo,   range_reading_normal, range_string,   NULL,             },
  { RANGE_MAGIC,  "LO",   "0.1",  false,  false,    "mV", range_lo,   range_reading_normal, range_string,   NULL,             },
  { RANGE_MAGIC,  "LO",   "1",    false,  false,    "V",  range_lo,   range_reading_normal, range_string,   NULL,             },
  { RANGE_MAGIC,  "LO",   "10",   false,  true,     "V",  range_lo,   range_reading_normal, range_string,   NULL,             },

  { RANGE_MAGIC,  "LO2",  "0.01", true,   false,    "mV", range_lo2,  range_reading_normal, range_string,   NULL,             },
  { RANGE_MAGIC,  "LO2",  "0.1",  false,  false,    "mV", range_lo2,  range_reading_normal, range_string,   NULL,             },
  { RANGE_MAGIC,  "LO2",  "1",    false,  false,    "V",  range_lo2,  range_reading_normal, range_string,   NULL,             },
  { RANGE_MAGIC,  "LO2",  "10",   false,  true,     "V",  range_lo2,  range_reading_normal, range_string,   NULL,             },


  { RANGE_MAGIC,  "DCV",  "0.01", true,   false,    "mV", range_dcv,  range_reading_dcv,    range_string,   range_dcv_pred,   },
  { RANGE_MAGIC,  "DCV",  "0.1",  false,  false,    "mV", range_dcv,  range_reading_dcv,    range_string,   range_dcv_pred,   },
  { RANGE_MAGIC,  "DCV",  "1",    false,  false,    "V",  range_dcv,  range_reading_dcv,    range_string,   range_dcv_pred,   },
  { RANGE_MAGIC,  "DCV",  "10",   false,  false,    "V",  range_dcv,  range_reading_dcv,    range_string,   range_dcv_pred,   },
  { RANGE_MAGIC,  "DCV",  "100",  false,  false,    "V",  range_dcv,  range_reading_dcv,    range_string,   range_dcv_pred,   },
  { RANGE_MAGIC,  "DCV",  "1000", false,  true,     "V",  range_dcv,  range_reading_dcv,    range_string,   range_dcv_pred,   },

  { RANGE_MAGIC,  "TEMP", "",     true,   true,     "°C", range_temp, range_reading_temp,   range_string,   NULL,             },

  { RANGE_MAGIC,  "LTS",  "0.01", true,   false,    "mV", range_lts,  range_reading_normal, range_string,   NULL,             },  // better name, LTS or DCV LTS.
  { RANGE_MAGIC,  "LTS",  "0.1",  false,  false,    "mV", range_lts,  range_reading_normal, range_string,   NULL,             },
  { RANGE_MAGIC,  "LTS",  "1",    false,  false,    "V",  range_lts,  range_reading_normal, range_string,   NULL,             },
  { RANGE_MAGIC,  "LTS",  "10",   false,  true,     "V",  range_lts,  range_reading_normal, range_string,   NULL,             }


};

const size_t range_init_sz = ARRAY_SIZE( range_init_values );




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

