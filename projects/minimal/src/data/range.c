


#include <stdio.h>
// #include <stddef.h>     // NULL
#include <assert.h>
#include <strings.h>      // strcasecmp
#include <string.h>      // strcasecmp
#include <math.h>         // fabs

#include <mode.h>
#include <data/cal.h>
#include <data/range.h>

#include <lib3/util.h>   // UNUSED()
#include <support.h>   // format_value()


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
  sa_set( &mode->sa, "ch2" );
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

  sa_set( &mode->sa, "0" ); // sample A400 gnd

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
  sa_set( &mode->sa, "ch2" );
  mode_ch2_set( mode, "ref");
}



static void range_temp( const range_t *range, _mode_t *mode, bool range_10Meg)
{
  UNUSED(range_10Meg);
  assert(range && range->magic == RANGE_MAGIC);
  assert(mode && mode->magic == MODE_MAGIC);

  mode_partial_reset( mode);
  cr_mode_set( &mode->reg_cr, MODE_SA_ADC);
  sa_set( &mode->sa, "ch2" );
  mode_ch2_set( mode, "temp");
}



static void range_lts( const range_t *range, _mode_t *mode, bool range_10Meg)
{
  UNUSED(range_10Meg);
  assert(range && range->magic == RANGE_MAGIC);
  assert(mode && mode->magic == MODE_MAGIC);

  mode_partial_reset( mode);

  cr_mode_set( &mode->reg_cr, MODE_SA_ADC);
  sa_set( &mode->sa, "ch2" );
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

  sa_set( &mode->sa, "ch1");

  // close relay - select external terminal input
  mode->serial.K402 = SR_SET;

  printf("dcv range_10Meg is %u\n", range_10Meg );

  if(strcasecmp( range->arg, "1000") == 0) {

    mode->serial.K403 = SR_SET;
    mode_gain_set( mode, 1);
    mode_ch2_set( mode, "div");
    sa_set( &mode->sa, "ch2" );
  }
  else if(strcasecmp( range->arg, "100") == 0) {

    mode->serial.K403 = SR_SET;
    mode_gain_set( mode, 10);
    mode_ch2_set( mode, "div");
    sa_set( &mode->sa, "ch2" );
  }
  else if(strcasecmp( range->arg, "10") == 0) {

    mode->serial.K403 = range_10Meg ? SR_SET : SR_RESET;
    mode_gain_set( mode, 1);
    sa_set( &mode->sa, "ch1" );
  }
  else if(strcasecmp( range->arg, "1") == 0) {

    mode->serial.K403 = range_10Meg ? SR_SET : SR_RESET;
    mode_gain_set( mode, 10);
    sa_set( &mode->sa, "ch1" );
  }
  else if(strcasecmp( range->arg, "0.1") == 0) {

    mode->serial.K403 = range_10Meg ? SR_SET : SR_RESET;
    mode_gain_set( mode, 100);
    sa_set( &mode->sa, "ch1" );
  }
  else if(strcasecmp( range->arg, "0.01") == 0) {

    mode->serial.K403 = range_10Meg ? SR_SET : SR_RESET;
    mode_gain_set( mode, 1000);
    sa_set( &mode->sa, "ch1" );
  }
  else
    assert( 0);
}



///////////////////////////

/* we may need to pass whether the front or rear terminal inputs
  are active.
*/

static double range_reading_normal( const range_t *range, const cal_t *cal, double adc_count_sum_norm)
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

    return cal->b * adc_count_sum_norm;
  }
  else if(strcasecmp( range->arg, "1") == 0)
  {
    // may want default adc_count_sum_norms
    // or express as or cal->b * cal->b10.
    return cal->b10 * adc_count_sum_norm;
  }
  else if(strcasecmp( range->arg, "0.1") == 0)
  {
    return cal->b100 * adc_count_sum_norm;
  }
  else if(strcasecmp( range->arg, "0.01") == 0)
  {
    return cal->b1000 * adc_count_sum_norm;
  }
  else
    assert( 0);

  // compiler
  return 0;
}




static double range_reading_dcv( const range_t *range, const cal_t *cal, double adc_count_sum_norm)
{
  assert(range && range->magic == RANGE_MAGIC);
  // eg. with input terminl offset


  if(strcasecmp( range->arg, "1000") == 0) {

    // TODO add front or rear terminal offset.
    // but not if using internal...
    return cal->div1000 * adc_count_sum_norm;
  }
  if(strcasecmp( range->arg, "100") == 0) {

    return cal->div100 * adc_count_sum_norm;
  }
  else  {

    // TODO add offset
    return range_reading_normal( range, cal, adc_count_sum_norm);
  }

  // compiler
  return 0;
}




static double range_reading_temp( const range_t *range, const cal_t *cal, double adc_count_sum_norm)
{
  assert(range && range->magic == RANGE_MAGIC);

  // can delegate here
  // lm35d
  // return cal->b * adc_count_sum_norm * 100;

  return range_reading_normal( range, cal, adc_count_sum_norm) * 100.f;
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
  // So. use the adc_count_sum_norm. * b;
  // rather than the specific range.
  // or else we an use arg. to  work out the value we want.

  UNUSED(v);

  return 0;
}


///////////////////////////////////



//   void (*range_reading_format)( const range_t *range, format_val_t *fval, unsigned ndigits, double value);

static void range_reading_format(
  const range_t *range,
  format_val_t  *val,
  unsigned    ndigits,
  double      value
) {


  // rename range_value_string.
  // could handle ',' or ' ' intersperse here with flags
  // or where used.

  assert(range && range->magic == RANGE_MAGIC);

  /*  we do not want digit positions to change between a value of 900 and 1000
      so must manage leading digits statically per range, rather than dynamically and per value
  */

  // val->leading = 0;

  /*
    for degC.  can convert ch 'C' to 'degC' or °C locally
    except we may want the full range.   eg. DCV
    No. keep separate.
  */


  if(strcasecmp( range->arg, "1000") == 0) {
    val->leading = 4;
    val->m = ' ';
  }
  else if(strcasecmp( range->arg, "100") == 0) {
    val->leading = 3;
    val->m = ' ';
  }
  else if(strcasecmp( range->arg, "") == 0
    || strcasecmp( range->arg, "10") == 0
  ) {
    val->leading = 2;
    val->m = ' ';
  }
  else if(strcasecmp( range->arg, "1") == 0)
  {
    val->leading = 1;
    val->m = ' ';
  }
  else if(strcasecmp( range->arg, "0.1") == 0)
  {
    val->leading = 3;
    value *= 1000.;
    val->m = 'm';
    // can change unit to "mV"
    // have to add 'm'
  }
  else if(strcasecmp( range->arg, "0.01") == 0)
  {
    val->leading = 2;
    value *= 1000.;
    val->m = 'm';
    // have to add 'm'
  }
  else
    assert( 0);

  // could actually look at the arg here... if wanted.

  // format unit
  snprintf( val->u, ARRAY_SIZE(val->u) - 1, "V");

  // format value
  str_format_value( val->s, ARRAY_SIZE( val->s) - 1,  ndigits, val->leading, value);



  // format everything - as convenience for callers
  snprintf(
    val->all,
    ARRAY_SIZE(val->all) - 1,
    "%c%s%c%s",
    value >= 0 ? '+' : '-',
    val->s,
    val->m,
    val->u
  );

}



/*
  34401a.
    up button is higher voltage.
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

/*
  - auto-ranging with shunts - may be a little tricky.
      unless organize the shunt values - so comparators at amplifier output are triggered where want for range switching.

  - consider remove the unit string.
    hiding capability behind a function is more flexible and more localized.

    better hiding/ encapsulation.   and more flexible.
    range_format_unit()   function
*/






size_t ranges_init( range_t *ranges, size_t sz)
{

  const range_t temp[] = {

    // magic        name    arg     sentinels         unit  set_mode    convert to reading    format          autorange predicate
    { RANGE_MAGIC,  "REF",  "",     true,   true,     range_ref,  range_reading_normal, range_reading_format,   NULL,             },

    { RANGE_MAGIC,  "LO",   "0.01", true,   false,    range_lo,   range_reading_normal, range_reading_format,   NULL,             },
    { RANGE_MAGIC,  "LO",   "0.1",  false,  false,    range_lo,   range_reading_normal, range_reading_format,   NULL,             },
    { RANGE_MAGIC,  "LO",   "1",    false,  false,    range_lo,   range_reading_normal, range_reading_format,   NULL,             },
    { RANGE_MAGIC,  "LO",   "10",   false,  true,     range_lo,   range_reading_normal, range_reading_format,   NULL,             },

    { RANGE_MAGIC,  "LO2",  "0.01", true,   false,    range_lo2,  range_reading_normal, range_reading_format,   NULL,             },
    { RANGE_MAGIC,  "LO2",  "0.1",  false,  false,    range_lo2,  range_reading_normal, range_reading_format,   NULL,             },
    { RANGE_MAGIC,  "LO2",  "1",    false,  false,    range_lo2,  range_reading_normal, range_reading_format,   NULL,             },
    { RANGE_MAGIC,  "LO2",  "10",   false,  true,     range_lo2,  range_reading_normal, range_reading_format,   NULL,             },


    { RANGE_MAGIC,  "DCV",  "0.01", true,   false,    range_dcv,  range_reading_dcv,    range_reading_format,   range_dcv_pred,   },
    { RANGE_MAGIC,  "DCV",  "0.1",  false,  false,    range_dcv,  range_reading_dcv,    range_reading_format,   range_dcv_pred,   },
    { RANGE_MAGIC,  "DCV",  "1",    false,  false,    range_dcv,  range_reading_dcv,    range_reading_format,   range_dcv_pred,   },
    { RANGE_MAGIC,  "DCV",  "10",   false,  false,    range_dcv,  range_reading_dcv,    range_reading_format,   range_dcv_pred,   },
    { RANGE_MAGIC,  "DCV",  "100",  false,  false,    range_dcv,  range_reading_dcv,    range_reading_format,   range_dcv_pred,   },
    { RANGE_MAGIC,  "DCV",  "1000", false,  true,     range_dcv,  range_reading_dcv,    range_reading_format,   range_dcv_pred,   },

    { RANGE_MAGIC,  "TEMP", "",     true,   true,     range_temp, range_reading_temp,   range_reading_format,   NULL,             },

    { RANGE_MAGIC,  "LTS",  "0.01", true,   false,    range_lts,  range_reading_normal, range_reading_format,   NULL,             },  // better name, LTS or DCV LTS.
    { RANGE_MAGIC,  "LTS",  "0.1",  false,  false,    range_lts,  range_reading_normal, range_reading_format,   NULL,             },
    { RANGE_MAGIC,  "LTS",  "1",    false,  false,    range_lts,  range_reading_normal, range_reading_format,   NULL,             },
    { RANGE_MAGIC,  "LTS",  "10",   false,  true,     range_lts,  range_reading_normal, range_reading_format,   NULL,             }

  };


  printf( "ranges sz %u,  max %u\n", ARRAY_SIZE( temp), sz);

  assert( ARRAY_SIZE( temp) < sz);

  memcpy( ranges, &temp, sizeof( temp));

  return ARRAY_SIZE( temp);
}



/*
Passing a NULL pointer as an argument to strcasecmp results in undefined
behavior. The function expects valid, null-terminated strings as its arguments.
*/




