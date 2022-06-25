/*
  nix-shell ~/devel/nixos-config/examples/arm.nix
  rlwrap -a picocom -b 115200 /dev/ttyUSB0

  nix-shell ~/devel/nixos-config/examples/arm.nix
  make

  nix-shell ~/devel/nixos-config/examples/arm.nix
  cd smu11
  openocd -f ../../openocd.cfg

  nix-shell ~/devel/nixos-config/examples/arm.nix
  rlwrap nc localhost 4444
  reset halt ; flash write_image erase unlock ./main.elf; sleep 1500; reset run


// vim :colorscheme default. loooks good.

// cjmcu  stm32f407.
// issue. is that board/stlink doesn't appear to reset cleanly. needs sleep.

// - can plug a thermocouple straight into the sense +p, +n, and
// then use the slow (digital) integrator/pid loop - as a temperature controler..


*/

#include <libopencm3/stm32/rcc.h>

//#include <libopencm3/stm32/gpio.h>
//#include <libopencm3/cm3/nvic.h>
//#include <libopencm3/cm3/systick.h>
//#include <libopencm3/stm32/exti.h>
//#include <libopencm3/stm32/timer.h>
//#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/spi.h>   // SPI1

#include <libopencm3/stm32/gpio.h>    // led


#include <stddef.h> // size_t
#include <math.h> // nanf   fabs
#include <stdio.h>    // sscanf
#include <string.h>   // strcmp



#include "assert.h"
#include "cbuffer.h"
#include "cstring.h"
#include "streams.h"
#include "fbuffer.h"
#include "usart.h"
#include "util.h"


#include "spi1.h"
#include "mcp3208.h"
#include "w25.h"
#include "dac8734.h"
#include "ads131a04.h"


#include "format.h"
#include "stats.h"

#include "mux.h"   // some of the above files include core.h. need header guards.


#include "spi-ice40.h"
#include "reg.h"

#define UNUSED(x) (void)(x)




#include "app.h"


//////////////////////////

static void output_set(app_t *app, irange_t irange, uint8_t val);


static void state_change(app_t *app, state_t state );


/*
// TODO must change the name of this.
// conflicts with core.c core.h functions
// just rename core.c to mux.c.  because that's the function prefix of that module
*/
static void core_set( app_t *app, float v, float i, vrange_t vrange, irange_t irange);







static vrange_t range_voltage_next( vrange_t vrange, bool dir)
{
  // dir==1 ve == lower voltage == lower range

  /* could just use addition - enum addition ... etc.
    but this makes things clearer.
    and enables encoding ranges that don't support iterating.
  */

  if(dir) {
    switch(vrange)
    {
      case vrange_100mV:  return vrange_100mV;  // no change
      case vrange_1V:     return vrange_100mV;
      case vrange_10V:    return vrange_1V;
      case vrange_100V:   return vrange_10V;
    };
  } else {
    switch(vrange)
    {
      case vrange_100mV:  return vrange_1V;
      case vrange_1V:     return vrange_10V;
      case vrange_10V:    return vrange_100V;
      case vrange_100V:   return vrange_100V; // no change
    };
  }

  assert(0);
  // suppress compiler warning...
  return (vrange_t)-9999;
}




static const char * range_voltage_string(vrange_t vrange)
{

  switch(vrange)
  {
    case vrange_100V:   return "100V";
    case vrange_10V:    return "10V";
    case vrange_1V:     return "1V";
    case vrange_100mV:  return "100mV";
  }

  printf("bad vrange is %d\n", vrange);
  assert(0);
  // suppress compiler warning...
  return "error";
}

/*
  change  name
  range_voltage_unit_convert
*/

static float range_voltage_multiplier( vrange_t vrange)
{
  // ie expressed on 10V range
  switch(vrange)
  {
    case vrange_100V:   return 10.f;
    case vrange_10V:    return 1.f;   // we're actually on a 10V range by default
    case vrange_1V:     return 0.1f;
    case vrange_100mV:  return 0.01f;
  }

  // invalid
  assert(0);
  return -99999;
}





static void dac_current_set(app_t *app, float i)
{
  // wrapper over raw spi is good ...
  // can record the value
  // dac_vset, dac_iset.

  assert(i >= 0);
  mux_dac(app->spi);
  spi_dac_write_register(app->spi, DAC_DAC1_REGISTER, voltage_to_dac( i ));
}


static void dac_voltage_set(app_t *app, float v)
{
  // wrapper over raw spi is good ...
  // can record the value
  // dac_vset, dac_iset.

  assert(v >= 0);
  mux_dac(app->spi);
  spi_dac_write_register(app->spi, DAC_DAC0_REGISTER, voltage_to_dac( v ));

  /*
    OK. this works to set gain on voltage. great...
  */
  // gain. 8 bits, twos complement for gain...
  /*
  The Gain Register stores the user-calibration data that are used to eliminate the gain error. The data are eight
  bits wide, 1 LSB/step, and the total adjustment is typically –128 LSB to +127 LSB, or ±0.195% of full-scale
  range.
  */
  // spi_dac_write_register(app->spi, DAC_GAIN_REGISTER0, -0x7f );
  // spi_dac_write_register(app->spi, DAC_GAIN_REGISTER0, -31 );  // -128 min. 129 overflows... good.
  // spi_dac_write_register(app->spi, DAC_GAIN_REGISTER0, 0 );

  // zero register is 9 bits
  /*
  The Zero Register stores the user-calibration data that are used to eliminate the offset error. The data are nine
  bits wide, 0.125 LSB/step, and the total adjustment is typically –32 LSB to +31.875 LSB, or ±0.0488% of
  full-scale range.
  */
  // spi_dac_write_register(app->spi, DAC_ZERO_REGISTER0, +255 ); // 255 ok. 256 overflows.
  // spi_dac_write_register(app->spi, DAC_ZERO_REGISTER0, 0 ); // 255 ok. 256 overflows.

  // we want the ability to set these values in the  gui...

  /*
    int8 max 127
    int8 min -128
  */
  printf("int8 max %d\n", INT8_MAX);
  printf("int8 min %d\n", INT8_MIN);
  // assert(0);
}





static void range_voltage_set(app_t *app, vrange_t vrange)
{
/*
  shouldn't be called directly
  because autoranging - means we may be off the range. and dac value is 11.

  we need a function to set the voltage range. and set the dac value.

*/
  printf("range_voltage_set %s -> %s\n",
    app->vrange != 0 ? range_voltage_string(app->vrange) : "none",
    range_voltage_string(vrange)
  );
  app->vrange = vrange;

  mux_ice40(app->spi);   // would be better to avoid calling if don't need.


  switch(app->vrange)
  {

    case vrange_10V:
      // printf("10V range \n");
      // flutters at 5 digit. nice.
      // 6th digit. with 9V and 0V.
      ice40_reg_write(app->spi, REG_INA_VFB_ATTEN_SW, INA_VFB_ATTEN_SW1_CTL);       // no atten
      ice40_reg_write(app->spi, REG_INA_VFB_SW, ~INA_VFB_SW1_CTL);                  // x1 direct feedback. works.
      break;


    case vrange_1V:
      // printf("1V range \n");
      ice40_reg_write(app->spi, REG_INA_VFB_ATTEN_SW, INA_VFB_ATTEN_SW1_CTL);       // no atten
      ice40_reg_write(app->spi, REG_INA_VFB_SW, ~INA_VFB_SW2_CTL);                  // 10x gain
      break;

    case vrange_100mV:

      // printf("100mV range \n");
      ice40_reg_write(app->spi, REG_INA_VFB_ATTEN_SW, INA_VFB_ATTEN_SW1_CTL);       // no atten
      ice40_reg_write(app->spi, REG_INA_VFB_SW, ~INA_VFB_SW3_CTL);                  // 100x gain
      break;


  case vrange_100V:
      // printf("100V range \n");
      // flutters at 4th digit. with mV.  but this is on mV. range... so ok?
      // at 6th digit with V.  eg. 9V and 0.1V. - very good - will work for hv.
      ice40_reg_write(app->spi, REG_INA_VFB_ATTEN_SW, INA_VFB_ATTEN_SW2_CTL | INA_VFB_ATTEN_SW3_CTL);    // atten = 0.1x
      ice40_reg_write(app->spi, REG_INA_VFB_SW, ~INA_VFB_SW1_CTL);                  // x1 direct feedback. works.
      break;


    // IMPORTANT - remember have the other attenuation possibility... of just turning on/off sw3.
#if 0
  case vrange_10V_2:
      // flutters at 4th digit.
      ice40_reg_write(app->spi, REG_INA_VFB_ATTEN_SW, INA_VFB_ATTEN_SW2_CTL | INA_VFB_ATTEN_SW3_CTL);    // atten = 0.1x
      ice40_reg_write(app->spi, REG_INA_VFB_SW, ~INA_VFB_SW2_CTL);                                   // x10 . works.
      // vmultiplier = 1.f;
      break;
#endif

  };


}




/*

  done - EXTREME. remove the 10k thick film. in front of the op.
  use jumper to test. or else era thin-film.
  ---
  change 200 and 20ohm to era. thin film. for lower noise.
*/




static const char * range_current_string( irange_t irange)
{
  // can simplify - enum addition ... etc.

  switch(irange)
  {
    case irange_10nA:   return "10nA";
    case irange_100nA:  return "100nA" ;
    case irange_1uA:    return "1uA" ;
    case irange_10uA:   return "10uA" ;
    case irange_100uA:  return "100uA";
    case irange_1mA:    return "1mA";
    case irange_10mA:   return "10mA";
    case irange_100mA:  return "100mA";
    case irange_1A:     return "1A";
    case irange_10A:    return "10A";
  };

  printf("error. unknown range_current irange is %d\n", irange );
  // suppress compiler warning...
  assert(0);
  return "irange error";
}





static irange_t range_current_next( irange_t irange, bool dir)
{
  /// dir==1 ve == lower current
  // can simplify - enum addition ... etc.
  // but this is kind of cleaner. allows independent ranges.

  if(dir) {
    // lower current range. ie. higher value shunt resistor.
    switch(irange)
    {
      case irange_10nA:   return irange_10nA; // no change
      case irange_100nA:  return irange_10nA;
      case irange_1uA:    return irange_100nA;
      case irange_10uA:   return irange_1uA;
      case irange_100uA:  return irange_10uA;
      case irange_1mA:    return irange_100uA;
      case irange_10mA:   return irange_1mA;
      case irange_100mA:  return irange_10mA;
      case irange_1A:     return irange_100mA;
      case irange_10A:    return irange_1A;
    };
  } else {
    // higher current range. ie lower value shunt resistor
    switch(irange)
    {
      case irange_10nA:   return irange_100nA;
      case irange_100nA:  return irange_1uA;
      case irange_1uA:    return irange_10uA;
      case irange_10uA:   return irange_100uA;
      case irange_100uA:  return irange_1mA;
      case irange_1mA:    return irange_10mA;
      case irange_10mA:   return irange_100mA;
      case irange_100mA:  return irange_1A;
      case irange_1A:     return irange_10A;
      case irange_10A:    return irange_10A;   // no change
    };
  }

  // suppress compiler warning...
  assert(0);
  return (irange_t)-9999;
}



/*
  Extreme. we should turn on 5V digital. so that dg444 inputs don't just sink.
  eg. turn on 5V.
  then configure.
*/

static void range_current_set(app_t *app, irange_t irange)
{
  /*
    this is doing two things. muxing the sense input. and amplification.

    TODO IMPORTANT.
    if output is on. then we also have to change the output relay...
  */

  printf("range_current_switch %s -> %s\n",
    app->irange != 0 ? range_current_string(app->irange) : "none",
    range_current_string(irange)
  );
  app->irange = irange;

  mux_ice40(app->spi);


  switch(app->irange)
  {

    case irange_10A:
    case irange_1A:
    case irange_100mA:
    case irange_10mA:

      // VERY IMPORTANT - ensure high current output relay is on. and low current relay off. before open big fets.
      // before make changes that might increase current.
      // make before break.
      output_set(app, app->irange, app->output);
      msleep(3);

      // turn on current range x
      ice40_reg_write(app->spi, REG_RELAY_COM,  RELAY_COM_X);

      // turn off jfets switches for Y and Z ranges.
      ice40_reg_write(app->spi, REG_IRANGE_YZ_SW, 0);

      switch(app->irange) {

        // TODO add the actual power.

        case irange_10A:
        case irange_1A:
          // printf("1A range \n");

          // turn on sense amplifier 1
          ice40_reg_write(app->spi, REG_ISENSE_MUX,  ~ISENSE_MUX1_CTL);
          // turn on first set of big fets.
          ice40_reg_write(app->spi, REG_IRANGE_X_SW, IRANGE_X_SW1_CTL);
          // break;

          switch(app->irange) {
            // gain 10x on 0.1ohm, for 10V range. active low
            case irange_10A:
              ice40_reg_write(app->spi, REG_INA_IFB_SW,  ~INA_IFB_SW2_CTL);
              break;
            // gain 100x on 0.1ohm.  active low
            case irange_1A:
              ice40_reg_write(app->spi, REG_INA_IFB_SW,  ~INA_IFB_SW3_CTL);
              break;
            default:
              // cannot be here...
              assert(0);
              break;
          };
          break;

        // 10ohm resistor. for 10V swing.
        case irange_100mA:
          // turn on sense amplifier 2
          ice40_reg_write(app->spi, REG_ISENSE_MUX,  ~ISENSE_MUX2_CTL);
          // ensure sure the high current relay is on. before switching
          // gain 10x active low
          ice40_reg_write(app->spi, REG_INA_IFB_SW,  ~INA_IFB_SW2_CTL);
          // turn on 2nd switch fets.
          ice40_reg_write(app->spi, REG_IRANGE_X_SW, IRANGE_X_SW2_CTL);
          break;

        // 1k resistor. for 10V swing.
        case irange_10mA:
          // turn on sense amplifier 3
          ice40_reg_write(app->spi, REG_ISENSE_MUX,  ~ISENSE_MUX3_CTL);
          // gain 1x active low
          ice40_reg_write(app->spi, REG_INA_IFB_SW,  ~INA_IFB_SW1_CTL);
          // turn on 4th switch fets.
          ice40_reg_write(app->spi, REG_IRANGE_X_SW, IRANGE_X_SW4_CTL);
          break;

        default:
          // cannot be here.
          assert(0);
          break;
      }
      return;


    // x
    case irange_1mA:
    case irange_100uA:
    case irange_10uA:
    // y
    case irange_1uA:
    case irange_100nA:
    case irange_10nA:

      // turn off all fets used on comx range
      ice40_reg_write(app->spi, REG_IRANGE_X_SW, 0 );
      // turn on sense amplifier 3
      ice40_reg_write(app->spi, REG_ISENSE_MUX,  ~ISENSE_MUX3_CTL);
      // gain 1x active low
      ice40_reg_write(app->spi, REG_INA_IFB_SW,  ~INA_IFB_SW1_CTL);

      // we'll turn lc relay on if need be, after switching to lc range.

      switch(app->irange) {
        // 10k resistor. for 10V swing
        case irange_1mA:
        // 100k resistor for 10V swing.
        case irange_100uA:
        // 1M resistor for 10V swing.
        case irange_10uA:
          // turn on current range relay y
          ice40_reg_write(app->spi, REG_RELAY_COM,  RELAY_COM_Y);
          switch( app->irange) {
            case irange_1mA:
              // turn on jfet 1
              ice40_reg_write(app->spi, REG_IRANGE_YZ_SW, IRANGE_YZ_SW1_CTL);
              break;
            case irange_100uA:
              // turn on jfet 2
              ice40_reg_write(app->spi, REG_IRANGE_YZ_SW, IRANGE_YZ_SW2_CTL);
              break;
            case irange_10uA:
              // turn on jfet 2
              ice40_reg_write(app->spi, REG_IRANGE_YZ_SW, IRANGE_YZ_SW3_CTL);
              break;
            default:
              // cannot be here.
              assert(0);
              break;
          }
          break;

        case irange_1uA:
        case irange_100nA:
        case irange_10nA:

          // IMPORTANT DONT forget to add star jumper to star gnd!!!.
          // turn on current range relay Z
          ice40_reg_write(app->spi, REG_RELAY_COM,  RELAY_COM_Z);

          switch( app->irange) {
            // 10M for 10V swing.
            case irange_1uA:
              // turn on jfet 1
              ice40_reg_write(app->spi, REG_IRANGE_YZ_SW, IRANGE_YZ_SW1_CTL);
              break;
            // 100M for 10V swing.
            case irange_100nA:
              // turn on jfet 2
              ice40_reg_write(app->spi, REG_IRANGE_YZ_SW, IRANGE_YZ_SW2_CTL);
              break;
            case irange_10nA:
              // turn on jfet 3
              ice40_reg_write(app->spi, REG_IRANGE_YZ_SW, IRANGE_YZ_SW3_CTL);
              break;
            default:
              assert(0);
          }
        break;

        default:
          assert(0);
      }

      // wait until settled at lc before turn off big relay, and turn on the reed relay.
      // eg. should be done last.
      msleep(1);
      output_set(app, app->irange, app->output);

      break;

  }
}

/*
  OK. perhaps to prevent range change instability.
  we don't zoom away from the operation range. beit voltage or current?


*/


static float range_current_multiplier(irange_t irange)
{
  assert( 1e-7f == 0.0000001f);

  switch(irange)
  {
    // ie. expressed on 10V range

    case irange_10nA:   return 1e-9f;
    case irange_100nA:  return 1e-8f;
    case irange_1uA:    return 1e-7f;
    case irange_10uA:   return 0.000001f;
    case irange_100uA:  return 0.00001f;
    case irange_1mA:    return 0.0001f;
    case irange_10mA:   return 0.001f;
    case irange_100mA:  return 0.01f;
    case irange_1A:     return 0.1f;
    case irange_10A:    return 1.f;
  };

  assert(0);
  return -9999;
}


/*
  We need to change this. so that it can correct for bad logic -
    whereby we end up on a wrong range.

  the only way to do that is to store the current dac value.
  why do we need current dac value?

  if it's on a (invalid) higher (voltage or current) range - we should always just use the set voltage.

  the issue may be stability.
*/
/*
  instead of using a filter/lagged/aggregated value - to avoid instability triggering range change.
  should instead check /variance/standard deviation - and only range switch if output is stable.

*/

static bool range_current_auto(app_t *app, float i)
{
  bool changed = false;

  if(fabs(i) < 1.f) {

    // need to switch to lower current range
    irange_t lower = range_current_next( app->irange, 1);
    if(lower != app->irange) {
      printf("i is %f\n", i);
      printf("ZOOM IN current.\n");
      range_current_set(app, lower);
      changed  = true;
    }
  }
  else if (fabs(i) > 10.5 && app->irange < app->iset_range) {

    // switch (back) to a higher current range
    irange_t higher = range_current_next( app->irange, 0);
    if(higher != app->irange) {
      printf("i is %f\n", i);
      printf("ZOOM OUT current\n");
      range_current_set(app, higher);
      changed = true;
    }
  }


  if(changed == true)
  {
    if(app->irange == app->iset_range) {

      // ranges the same
      printf("use regulation current %f\n", app->iset);
      dac_current_set(app, fabs(app->iset));
    } else if( app->irange < app->iset_range ) {

      // we're zoomed in, and in compliance
      printf("use zoomed in current 11V on range\n");
      dac_current_set(app, 11.f );

    } else {
      // zoomed out past the range we should be on. this is a fault condition.
      // bad condition
      printf("on wrong range\n");
      assert(0);
    }
  }

  return changed;
}


/*
  ok, we set the output to -5V and it tries to output 12V?????
  is this a set_core problem?

  and it's not sinking either?

  tried to output -12V....
  my goodness...

*/


/*
  We had battery around the wrong way.
  Ok. sourcing into battery is working. 1.5V   3mA.
  And so is sinking.                    1.5V  -3mA.

  but then we get bad state. when we turn off and it outputs -12V... to try to get it to sink...
  perhaps we should be jumping out a range...

  So. the off condition fails...

  OK. because it's going
  We should range change.  eg. we limit at +5V...
  But it's trying to pump -12V...  eg. the opposite direction.
  So we should be switching out to the -100V range...

  So we need to account for direction....
  Bloody hell....
  ---------------------------

  Or should we always jump out .... no. we cant ...
  eg. when turned off...

  We could jump out... and set the dac 10x tighter...
  -----------------

  OR.... do we just have the core wrong. somehow.
  No. if set to sink current. then a limit of 5V. then -99V is what is expected .

  IMPORTANT.
  OR. its actually good as it is... eg. don't come off the regulation range...
  even if we're on the wrong range.

  Albeit it would be better if adc didn't go into ovp.
  But maybe it's ok. eg. 12V.
  -14.1V   - is giving us -2.83V on the ADC.
  -----

  So perhaps just ignore the ovp???

*/





static bool range_voltage_auto(app_t *app, float v)
{
  /*
    rules are.
      that we can jump around in higher resoution measurement ranges
      for more accurate measurement,
      without compromising operation and complianc values.
  */

  bool changed = false;

  if(fabs(v) < 1.f) {

    // need to switch to lower voltage range
    vrange_t lower = range_voltage_next( app->vrange, 1);
    if(lower != app->vrange) {  // eg. there is a lower range.
      printf("v is %f\n", v);
      printf("ZOOM in voltage\n");
      range_voltage_set(app, lower);
      changed = true;
    }
  }
  else if (fabs(v) > 10.5 && app->vrange < app->vset_range )
  {
    // zoomed in. and out of range.
    // we have to jump out... but don't jump out past the actual regulation range (vset_range)
    // else we'll be regulating on higher range than the set range
    vrange_t higher = range_voltage_next( app->vrange, 0);
    if(higher != app->vrange) {     // there is a higher range.
      printf("v is %f\n", v);
      printf("ZOOM out voltage\n");
      range_voltage_set(app, higher);
      changed = true;
    }
  }
  else if( fabs(v) > 10.5) {

    // we are not zoomed in... but there is an excursion...... on opposite polarity....
    // eg. cannot regulate on this claamp.
    // eg. set -5V, +3mA   on resistor.
    printf("WHOOT bad state here.\n");

    // no this happens if sinking and power is off....

    // switching off? probably won't help...   but we can invalidate.
    // but can switch off. and stop.

  }

  if(changed == true ) {

    if(app->vrange == app->vset_range) {

      printf("use regulation voltage %f\n", app->vset);
      dac_voltage_set(app, fabs(app->vset));
    } else if (  app->vrange < app->vset_range   ) {

      printf("use zoomed in voltage 11V on range\n");
      dac_voltage_set(app, 11.f );
    } else {
      // bad condition.
      printf("on wrong voltage range\n");
      assert(0);
    }
  }

  return changed;
}

/*
  express ranging as local state machine?
  iset_range   and imeas_range

*/


// shoudl we pass the irange?
// we might be calling this due to/ or in preparation to a range change...

static void output_set(app_t *app, irange_t irange, uint8_t val)
{
  /*
    // better name
    EXTR. we could completely bypass the AB section and just use 6090
    for low current ranges. if we wanted.
  */
  // this may be being called in response to range change.
  // we have to check the range...
  // possible we should pass the range also.

  app->output = val;

  // ok. this is called when changing ranges.
  // printf("output %s\n", app->output ? "on" : "off"  );

  if(app->output) {

      // switch( app->irange)
      switch( irange)
      {
        case irange_10nA:
        case irange_100nA:
        case irange_1uA:
        case irange_10uA:
        case irange_100uA:
        case irange_1mA:
          // turn on read relay
          // and turn off the hc relay.
          ice40_reg_write(app->spi, REG_RELAY_OUT, RELAY_OUT_COM_LC);
          ice40_reg_set(app->spi, REG_LED, LED2);
          break;

        case irange_10mA:
        case irange_100mA:
        case irange_1A:
        case irange_10A:
          // high current relay
          ice40_reg_write(app->spi, REG_RELAY_OUT, RELAY_OUT_COM_HC);
          ice40_reg_clear(app->spi, REG_LED, LED2);
          break;
      }
  }
  else {


    ice40_reg_write(app->spi, REG_RELAY_OUT, 0 ); // both relays off
    ice40_reg_clear(app->spi, REG_LED, LED2);
  }
}



/*
  - we should be passing either buf,size or else some kind of streaming interface...
  - eg. console_cookie
  - a FILE is the most generic stream  ie. cookie.
  - fprintf(stream, )
  --------------------

  if we want to be able to left and right indent this stuff... (without vt100,ansi terminal commands )
  then we need a much better interface...

  ALTERNATIVELY. if have a get()
    then we don't require the reverse() function... for float/integer formatting
  --------

  // snprintf(%s
  // once have a string.
  // can indent it, quite easily.  just using sprintf...

*/



#if 0
static char * indent_left(char *s, size_t sz, int indent, const char *string)
{
  // left indent, is pad to right, for field name
  snprintf(s, sz, "%-*s", indent, string );
  return s;
}



static char * indent_right(char *s, size_t sz, int indent, const char *string)
{
  // right indent, is pad to left, for field value
  snprintf(s, sz, "%*s", indent, string);
  return s;
}



static char * snprintf2(char *s, size_t sz, const char *format, ...)
{
  // same as snprintf but return the input buffer, as convenience for caller
	va_list args;
	va_start(args, format);
	vsnprintf(s, sz, format, args);
	va_end(args);

  return s;
}


// char * format_float(char *s, size_t len, double value, int digits);

#if 1

static char * format_float(char *s, size_t sz, double value, int digits)
{
  /*
    // eg. works
    printf("%0.*g\n",  5, 123.456789 );      // 123.46
    printf("%0.*g\n",  5, 12.3456789 );      // 12.346
    printf("%0.*g\n",  5, -12.3456789 );     // -12.346
  */

  snprintf(s, sz, "%0.*g\n",  digits, value);
  return s;
}

#endif

#endif




// extern char * format_float(char *buf, size_t len, double value, int digits);


static char * format_current(char *s, size_t sz, irange_t irange, float val, int digits)
{
  // beccause we use %s   we could actually do the indentation in this call...
  /*
    improtant.
      formatting measured values, according to selected range (rather than value) is correct.
      enourage drill to a higher range.
  */
  char buf[100];

  switch( irange)
  {
    // not sure whether we should care about the range....
    // looks like 34401a does. value 120mV versus 0.12V depends on active range.
    case irange_10nA:

      // when power is off... kind of nice to report...
      if(fabs(val) * 1e10f > 1.f)
        snprintf(s, sz, "%snA", format_float(buf, sizeof(buf), digits, val * 1e+9f));
      else
        snprintf(s, sz, "%spA", format_float(buf, sizeof(buf), digits, val * 1e+12f));
      break;


    case irange_100nA:
    case irange_1uA:
      snprintf(s, sz, "%snA", format_float(buf, sizeof(buf), digits, val * 1e+9f));
      break;

    case irange_10uA:
    case irange_100uA:
    case irange_1mA:
      snprintf(s, sz, "%suA", format_float(buf, sizeof(buf), digits, val * 1e+6f));
      break;

    case irange_10mA:
    case irange_100mA:
    case irange_1A:
      // TODO 0.7A better as 0.7A. 0.6A better as 600mA. think..
      snprintf(s, sz, "%smA", format_float(buf, sizeof(buf), digits, val * 1e+3f));
      break;

    case irange_10A:
      snprintf(s, sz, "%sA", format_float(buf, sizeof(buf), digits, val));
      break;

    default:
      assert(0);
  }

  return s;
}


static char * format_voltage(char *s, size_t sz, vrange_t vrange, float val, int digits)
{
  /*
    - ALTERNATIVELY could pass in a streaming interface...  with mark() reverse() etc...
  */

  char buf[100];

  // ie expressed on 10V range
  switch(vrange)
  {
    case vrange_100V:
    case vrange_10V:
      snprintf(s, sz, "%sV", format_float(buf, sizeof(buf), digits, val));
      break;

    case vrange_1V:
    case vrange_100mV:
      snprintf(s, sz, "%smV", format_float(buf, sizeof(buf), digits, val * 1e+3f));
      break;

    default:
      assert(0);
  }

  return s;
}








static void usart_print_kv(int fwidth, const char *fs, int vwidth,  const char *vs)
{
  char buf[100];
  // key/field
  printf( indent_left(buf, sizeof(buf), fwidth, fs));
  // val
  printf(indent_right(buf, sizeof(buf), vwidth, vs));
}



static void quadrant_set( app_t *app, bool v, bool i)
{
  // RULES.
  // so. if voltage is positive use clamp max.  clamp min/max follows voltage.
  // negative current. can still be source or sink. depending on polarity.
  // ie. clamp direction min/max following voltage.

  mux_ice40(app->spi);

  uint32_t vv = v ? CLAMP1_VSET_INV : CLAMP1_VSET;
  uint32_t ii = i ? CLAMP1_ISET_INV : CLAMP1_ISET;


  ice40_reg_write(app->spi, REG_CLAMP1, ~(vv | ii ));

  // rembmer inverse
  uint32_t minmax = v ? CLAMP2_MAX : CLAMP2_MIN;

  ice40_reg_write(app->spi, REG_CLAMP2, ~( minmax ) );     // min of current or voltage
}


// so can have another function. that tests the values.... v > 0 etc.
// need to hide

/*
  clamp direction follows voltage.

              clamp min  <->  clamp max

                           |+i
             (2)           |           (1)
             sink          |           source
                           |
                           |
                           |
         -ve --------------+-------------- +ve
                           |
                           |
                           |
             source        |           sink
             (3)           |           (4)
                           |-i

*/

static void core_set(app_t *app, float v, float i, vrange_t vrange, irange_t irange)
{

  printf("---------------\n");
  printf("core set\n");

  // TODO change arg order.


  // voltage
  dac_voltage_set(app, fabs( v));
  dac_current_set(app, fabs( i));


  quadrant_set( app, v > 0.f, i > 0.f ) ;

  // ignoring range
  app->vset = v;
  app->iset = i;

  app->vset_range = vrange;// vrange_10V;
  app->iset_range = irange;// irange_10mA;

  range_voltage_set(app, app->vset_range);
  range_current_set(app, app->iset_range);
}










static void spi1_interupt(app_t *app)
{
  /*
    interupt context. avoid doing work here...
  */
  if(app->adc_drdy == true) {
    // still flagged from last time, then code is too slow, and we missed an adc read
    ++app->adc_drdy_missed;
  }

  // set adc_drdy flag so that update() knows to read the adc...
  app->adc_drdy = true;
}



static void update_soft_1s(app_t *app)
{
  // maybe review this...
  UNUSED(app);


}

/*
    need to do the spi setup...
*/

static void update_soft_500ms(app_t *app)
{
  // blink the fpga led
  mux_ice40(app->spi);
  ice40_reg_toggle(app->spi, REG_LED, LED1);

/*
  // try w25 chip
  mux_w25(app->spi);
  msleep(20);
  spi_w25_get_data(app->spi);
*/


  // blink stm32/mcu led
  led_toggle();



}





static void update_nplc_measure(app_t *app)
{

  assert(app->state ==  STATE_ANALOG_UP);

  assert(fBufCount(&app->vfb_measure) == app->nplc_measure);
  assert(fBufCount(&app->vfb_measure) > 0);
  assert(fBufCount(&app->ifb_measure) == fBufCount(&app->vfb_measure));

  // normal operation
  if(app->print_adc_values) {

    // works in screen and picocom
    // https://stackoverflow.com/questions/60293014/how-to-clear-screen-of-minicom-terminal-using-serial-uart
    // printf("%c%c%c%c",0x1B,0x5B,0x32,0x4A);
    // https://electronics.stackexchange.com/questions/8874/is-it-possible-to-send-characters-through-serial-to-go-up-a-line-on-the-console

    // position cursor top left?

    // when we set the range. we should set the default format.
    // the format prec wants to be able to user modified.
    /////////////////

/*
  we need to know the cost of this calculation.
  since it's possible we could miss reading the adc on the power-line cycle.
  ----
  is there a way to tell... if we missed....

*/
    // most recent measurements
    float vfb = fBufPeekLast(&app->vfb_measure);
    float ifb = fBufPeekLast(&app->ifb_measure);

    /////////////////////
    // stats
    float vs[200];    // implies max nplc of 200
    float is[200];

    assert(app->nplc_measure <= ARRAY_SIZE(vs));

    size_t vn = fBufCopy(&app->vfb_measure, vs, ARRAY_SIZE(vs));
    assert(vn >= 1);

    size_t in = fBufCopy(&app->ifb_measure, is, ARRAY_SIZE(is));
    assert(in >= 1);
    assert(vn == in);


    /* TODO a single stats core function that computes all of these
    */
    float vmean = mean(vs, vn);
    float vsd   = stddev(vs, vn);
    float vmin, vmax;
    minmax(vs, vn, &vmin, &vmax);

    float imean = mean(is, in);
    float isd   = stddev(is, in);
    float imin, imax;
    minmax(is, in, &imin, &imax);


    char buf[100];

    // clear screen only after calculation... for cleaner update
#if 1
    printf("\033[2J");    // clear screen
    printf("\033[0;0H");  // cursor to top left
    // printf("\033[10B");   // move cursor down 10 lines ... works
    // printf("\033[3B");   // move cursor down 3 lines ... works
#endif



    printf("smart source measure unit\n");
    printf("\n");

    usart_print_kv( 6, "vmean:",  11,  format_voltage(buf, sizeof(buf), app->vrange, /*vfb*/ vmean * range_voltage_multiplier(app->vrange), 6 ) );

    printf("  ");
    usart_print_kv( 6, "vset:",   11,  format_voltage(buf, sizeof(buf), app->vset_range, app->vset * range_voltage_multiplier(app->vset_range), 6) );

    printf("  ");
    usart_print_kv( 10,"vset_range:", 5, range_voltage_string(app->vset_range));

    printf("  ");
    usart_print_kv( 8, "vrange:", 5, range_voltage_string(app->vrange));

    if(app->vrange == app->vset_range) {
      printf("*");
    }

    //////////
    // 11 gdigits needed. eg. -999.999nA
    printf("\n\n");

    usart_print_kv( 6, "imean:",  11,  format_current(buf, sizeof(buf), app->irange, /*ifb*/ imean * range_current_multiplier(app->irange), 6));

    printf("  ");
    usart_print_kv( 6, "iset:",   11,   format_current(buf, sizeof(buf), app->iset_range, app->iset * range_current_multiplier(app->iset_range), 6));

    printf("  ");
    usart_print_kv( 10, "iset_range:", 5, range_current_string(app->iset_range));

    printf("  ");
    usart_print_kv( 8, "irange:", 5, range_current_string(app->irange));

    if(app->irange == app->iset_range) {
      printf("*");
    }

    printf("\n\n");

    printf("raw\n");
    // raw vals
    printf("v");
    usart_print_kv( 4, "  fb:",      9, snprintf2(buf, sizeof(buf), "%f", vfb));
    usart_print_kv( 7, "  mean:",   9, snprintf2(buf, sizeof(buf), "%f", vmean));
    usart_print_kv( 9, "  stddev:", 9, snprintf2(buf, sizeof(buf), "%f", vsd));
    usart_print_kv( 5, "  min:",    9, snprintf2(buf, sizeof(buf), "%f", vmin));
    usart_print_kv( 5, "  max:",    9, snprintf2(buf, sizeof(buf), "%f", vmax));
    printf("\n");


    printf("i");
    usart_print_kv( 4, "  fb:",      9, snprintf2(buf, sizeof(buf), "%f", ifb));
    usart_print_kv( 7, "  mean:",   9, snprintf2(buf, sizeof(buf), "%f", imean));
    usart_print_kv( 9, "  stddev:", 9, snprintf2(buf, sizeof(buf), "%f", isd));
    usart_print_kv( 5, "  min:",    9, snprintf2(buf, sizeof(buf), "%f", imin));
    usart_print_kv( 5, "  max:",    9, snprintf2(buf, sizeof(buf), "%f", imax));

    printf("\n");



    // other stats
    printf("\n");
    usart_print_kv( 15, "nplc_measure:", 6, snprintf2(buf, sizeof(buf), "%d", app->nplc_measure));

    printf("\n");
    usart_print_kv( 15, "nplc_range:",   6, snprintf2(buf, sizeof(buf), "%d", app->nplc_range));

    printf("\n");
    usart_print_kv( 15, "pl_freq:",   6, "50");   // TODO



    printf("\n");
    usart_print_kv( 15, "millis", 6,      snprintf2(buf, sizeof(buf), "%d", system_millis - app->measure_millis_last));    // millis is 32 bit.
    app->measure_millis_last = system_millis;


    printf("\n");
    usart_print_kv( 15, "update_count:", 6,      snprintf2(buf, sizeof(buf), "%d", app->update_count));
    app->update_count = 0;



    printf("\n");
    usart_print_kv( 15, "drdy_missed:", 6,      snprintf2(buf, sizeof(buf), "%d", app->adc_drdy_missed));
    app->adc_drdy_missed = 0;



    printf("\n");
    usart_print_kv( 15, "adc_ov_count:", 6, snprintf2(buf, sizeof(buf), "%d", app->adc_ov_count));

    // change name display_digits?
    printf("\n");
    usart_print_kv( 15, "digits:", 6,      snprintf2(buf, sizeof(buf), "%d", app->digits));


    // rails
    // Math.log10( Math.pow(2, 12) ) == 3.6 digits for 12 bits rep.
    // appropriate format_voltage() function to format the rails voltages
    printf("\n");

    /*
      // something weird here....  when set for output 3mA. output rises to 16.00V... error in formatting?

      5V rail is at 4V. perhaps current draw from output relay is too much ...
      should measure this.
      usb to 5V. barrel jack.
      or could even be something short somewhere... really kind of need pwr supply monitor for 5V.

      -------
      ok. 1mA, 1uA, 1nA holds 6th current digit just about still. with nplc 50.
      that's pretty damn good.


    */

    // printf("%f   %f\n", app->lp15v, app->ln15v);

    usart_print_kv( 15, "lp15v:", 6, format_voltage(buf, sizeof(buf), vrange_10V, app->lp15v, 4));
    printf("\n");
    usart_print_kv( 15, "ln15v:", 6, format_voltage(buf, sizeof(buf), vrange_10V, app->ln15v, 4));


    printf("\n");
    usart_print_kv( 15, "output:", 6, (app->output) ? "on" : "off");

    printf("\n\n");

    // print the current console input buffer
    // If did not clear screen, then would not need to do this...
    printf("> ");


    // JA
    // cBufCopyString2(&app->cmd_in, buf, sizeof(buf));
    // printf("%s", buf);
  }
}



static void update_nplc_range(app_t *app)
{
  assert(app->state ==  STATE_ANALOG_UP);

  assert(fBufCount(&app->vfb_range) == app->nplc_range);
  assert(fBufCount(&app->vfb_range) > 0);
  assert(fBufCount(&app->ifb_range) == fBufCount(&app->vfb_range));


  // most recent rangements
  float vfb = fBufPeekLast(&app->vfb_range);
  float ifb = fBufPeekLast(&app->ifb_range);

  if(0) {
    range_current_auto(app, ifb );
    range_voltage_auto(app, vfb);
  }


  fBufClear(&app->vfb_range);
  fBufClear(&app->ifb_range);

}



static void update_fault_check(app_t *app)
{
  /* called on every read of slow-adc. eg. 50 or 60Hz
  */

  // float vfb = fBufPeekLast(&app->vfb_measure);
  float ifb = fBufPeekLast(&app->ifb_measure);


  if(fabs(ifb) > 11.5f)
  {
    /*
      11V is the dac hold value.  10.5 current range trigger. 11.5 is fault.
    */
    printf("ifb > 11.5V, fault current condition\n");
    state_change(app, STATE_HALT);
    assert(0);
  }

  // Think this is problematic..
  // 2 doesn't hold it.
  // OK. 3A works for -1.05A on ...
  // but sometimes triggers analog ovp...

  if(fabs(ifb) > 3.0f && app->irange == irange_10A)
  {
    /* unknown over-current condition
      probable hardware condition
      This gets triggered... on relay turn on.
    */
    /*
      OK. weirdness...
      this gets triggered - before it settles down.
      possible indicative of feedback stability on 10A range?..
    */
    printf("ifb is %f\n", ifb);
    printf("fault overcurrent condition\n");

    state_change(app, STATE_HALT);
    assert(0);
  }
}






static void update_adc_drdy(app_t *app)
{

  assert(app->adc_drdy && app->state == STATE_ANALOG_UP);


  float ar[4];
  int32_t ret = spi_adc_do_read(app->spi, ar, 4);
  app->adc_drdy = false;

  if(ret < 0) {
    // error
    // printf("adc error\n");
    ++app->adc_ov_count;
  } else {
    // no errors.
  }

  /*
    - must continue eve in presence of adc out-of-bound. in order to run ranging to get to a valid range
    - ranging needs the actual value (not adjusted for gain/attenuation
      so record and use in common units
  */
  float x = 0.435;
  // shouldn't record twice...
  float vfb = ar[0] * x;
  float ifb = ar[1] * x;

  fBufPush(&app->vfb_measure, vfb );
  fBufPush(&app->ifb_measure, ifb);

  assert(fBufPeekLast(&app->vfb_measure) == vfb);
  assert(fBufPeekLast(&app->ifb_measure) == ifb);
  assert(fBufCount(&app->vfb_measure) ==  fBufCount(&app->ifb_measure));

  /*
    we record adc values twice. for separate application
      - once for measure,
      - once for ranging.
  */

  fBufPush(&app->vfb_range, vfb );
  fBufPush(&app->ifb_range, ifb);

  assert(fBufPeekLast(&app->vfb_range) == vfb);
  assert(fBufPeekLast(&app->ifb_range) == ifb);
  assert(fBufCount(&app->vfb_range) ==  fBufCount(&app->ifb_range)); // if ov error reading... should perhaps be text error?


  // check for fault conditions
  update_fault_check(app);


  // do measure reporting
  if(fBufCount(&app->vfb_measure) == app->nplc_measure)
  {
    update_nplc_measure(app);
    // should be done where?...
    // change name measure_ov...  should
    app->adc_ov_count = 0;
    assert(fBufCount(&app->vfb_measure) == 0);
    assert(fBufCount(&app->ifb_measure) == 0);
  }

  // do ranging
  if( fBufCount(&app->vfb_range) == app->nplc_range)
  {
    update_nplc_range(app);
    assert(fBufCount(&app->vfb_range) == 0);
    assert(fBufCount(&app->ifb_range) == 0);
  }

}


static void update(app_t *app)
{
  // called as often as possible

  ++app->update_count;

  /*
      main adc, ready to be read, do first
  */
  if(app->state == STATE_ANALOG_UP && app->adc_drdy) {

    update_adc_drdy(app);
  }


  /*
    - these block... while value is read, and is the main source if this loop being slow. which also doesn't matter.
    could offload spi reading ot the fpga. along with test against threshold values.
  */

  if( /*false &&*/ app->state == STATE_HALT) {

    // no need to read rails in halt state
    // more useful, to know ice40 current
    app->lp15v = 0.f;
    app->ln15v = 0.f;
  } else {

    // otherwise read rails
    // 9k/1k dividers.
    mux_adc03(app->spi);
    app->lp15v = spi_mcp3208_get_data(app->spi, 0) * 1.00 * 10.;
    app->ln15v = spi_mcp3208_get_data(app->spi, 1) * 0.9  * 10.;

    // printf("lp15v %f    ln15v %f\n", app->lp15v, app->ln15v);
  }



  switch(app->state) {

    case STATE_FIRST:
      state_change(app, STATE_DIGITAL_UP);
      break;

    case STATE_DIGITAL_UP:
      if(app->lp15v > 15.0 && app->ln15v > 15.0 )
      {
        printf("-----------\n");
        printf("lp15v %f    ln15v %f\n", app->lp15v, app->ln15v);
        printf("15V analog rails ok - state change analog-up\n");
        state_change(app, STATE_ANALOG_UP);
      }
      break ;

    case STATE_ANALOG_UP:
      // this is the high speed adc, rails fault detection
      if((app->lp15v < 14.7 || app->ln15v < 14.7)  )
      {
        printf("lp15v %f    ln15v %f\n", app->lp15v, app->ln15v);
        printf("15V analog rails undervoltage condition\n");
        // assert(0);
      }
      else if((app->lp15v > 15.3 || app->ln15v > 15.3)  )
      {
        printf("lp15v %f    ln15v %f\n", app->lp15v, app->ln15v);
        printf("15V analog rails overvoltage condition\n");
        assert(0);
      }
      break;

    case STATE_HALT:
      break;


    default:
      ;
  };
}



static void state_change(app_t *app, state_t state )
{
  switch(state) {

    case STATE_FIRST:
      printf("-------------\n" );
      printf("first\n" );


      app->state = STATE_FIRST;
      break;


    case STATE_HALT: {

      printf("-------------\n" );
      printf("change to halt state\n" );

      /*
        IMPORTANT.
        OK. we have to be turn off the adc as well which is complicated....
        else it keeps outputting data...
        or disable the interupt.

      */

      mux_ice40(app->spi);

       // disconnect output
      printf("turn off output\n" );
      output_set(app, app->irange, false );
      msleep(20);

      // turn off high power rails
      printf("turn off rails +-30V\n" );
      ice40_reg_clear(app->spi, REG_RAILS, RAILS_LP30V );
      msleep(10);

      // analog rails
      printf("turn off rails +-15V\n" );
      ice40_reg_clear(app->spi, REG_RAILS, RAILS_LP15V);
      msleep(10);

      // 5V
      printf("turn off rails +5V\n" );
      ice40_reg_clear(app->spi, REG_RAILS, RAILS_LP5V);
      msleep(10);

#if 1
      printf("turn off adc\n" );
      // hardware reset adc, to stop generating interupts on read
      adc_reset( app->spi, REG_ADC);
      mux_ice40(app->spi);
#endif

      app->state = STATE_HALT;
      break;
    }


    case STATE_DIGITAL_UP: {

      // if any of these fail, this should progress to error
      printf("-----------\n");
      printf("digital start\n" );

      mux_ice40(app->spi);

      ////////////
      // soft reset is much better here.
      // avoid defining initial condition. in more than one place
      // so define in fpga.
      ice40_reg_clear(app->spi, CORE_SOFT_RST, 0);    // any value addressing this register.. to clear

      // no. needs dg444/mux stuff. pulled high. for off.
      // BUT I THINK we should probably hold RAILS_OE high / deasserted.


      // test the flash
      // TODO. check responses.
      mux_w25(app->spi);
      spi_w25_get_data(app->spi);


      // dac init
      int ret = dac_init(app->spi, REG_DAC); // bad name?
      if(ret != 0) {
        state_change(app, STATE_HALT);
        return;
      }

      // TODO remove.... fix regualte on vfb.
      printf("-------------\n" );


      // progress to digital up?
      printf("digital up ok\n" );
      app->state = STATE_DIGITAL_UP;
      break;
    }


    case STATE_ANALOG_UP: {


      printf("turn on lp5v\n" );
      mux_ice40(app->spi);
      // assert rails oe
      ice40_reg_clear(app->spi, REG_RAILS_OE, RAILS_OE);

      // turn on 5V digital rails
      ice40_reg_set(app->spi, REG_RAILS, RAILS_LP5V );
      msleep(50);

      // turn on +-15V rails
      printf("turn on analog rails - lp15v\n" );
      ice40_reg_set(app->spi, REG_RAILS, RAILS_LP15V );
      msleep(50);

#if 1
      // turn on +-30V rails. think this is ok here...
      printf("turn on power rails \n" );
      ice40_reg_set(app->spi, REG_RAILS, RAILS_LP30V );
      msleep(50);
#endif



      // LP30 - needed to power the vfb topside op amp. ltc6090/ bootstrapped
      // ice40_reg_set(spi, REG_RAILS, RAILS_LP30V );
      // msleep(50);
       /*
      ice40_reg_set(spi, REG_RAILS, RAILS_LP60V );
       */

#if 0
      // TODO EXTREME . set the gain switches before turning on rails.
      // IMPORTANT - should probably do this. before switching on the supplies.
      // so that vrange ops are not high-Z
      printf("turn on voltage range\n" );
      range_voltage_set(spi);

      printf("turn on current range\n" );
      range_current_set(spi);
#endif

      // turn on refs for dac
      //mux_dac(spi);
      printf("turn on ref a for dac\n" );
      mux_ice40(app->spi);
      ice40_reg_write(app->spi, REG_DAC_REF_MUX, ~(DAC_REF_MUX_A | DAC_REF_MUX_B)); // active lo

      // dac naked register references should be wrapped by functions
      // unipolar.
      // voltage

      /*
        https://www.youtube.com/watch?v=qFVhe_uzxnE
        source current. 100mA.   with compliance +21V.   DUT resistive load.
        source current -100mA.   with compliance +21V.   with power supply.
      */

      // range iteration changes the meaning of the set voltage.

      // regulating on 10V ok. 10.5V also ok. 11V. also ok. great.
      // 11.5V reads correctly, but also ovp
      // 11.2 ovp .
      // 11.1 ok.

      // INITIAL
      // core_set( app, -5.f , -5.f );    // -5V compliance, -1mA  sink.
      // core_set( app, 5.f , 3.f, vrange_10V, irange_10mA );         // 5V source, 5mA compliance,
      // core_set( app, 0.5f , 3.f, vrange_10V, irange_10mA );         // oscillates.
      core_set( app, 5.f , 3.f, vrange_10V, irange_10mA );         // 5V source, 5mA compliance,

      // the voltage - is not actually changing with voltage set... ?/

      /////////////
      // working as bipolar.
      spi_dac_write_register(app->spi, DAC_DAC2_REGISTER, voltage_to_dac( -2.f ) );  // outputs -4V to tp15.  two's complement works. TODO but need to change gain flag?
      spi_dac_write_register(app->spi, DAC_DAC3_REGISTER, voltage_to_dac( 0.f ) );  // outputs 4V to tp11.



      // change namem output relay?
      /*
        starting from off. means it has more work to do, jumping up through ranges...
        unless
      */
      output_set(app, app->irange, false );   // turn off by default...


#if 1
      // EXTR. TODO. move this. to initialize adc before setting the core.
      /////////////////
      // adc init has to be done after rails are up...
      // but doesn't need xtal, to respond to spi.
      // adc init
      int ret = adc_init(app->spi, REG_ADC);
      if(ret != 0) {
        // app->state = ERROR;

        state_change(app, STATE_HALT );
        return;
      }
#endif


      app->state = STATE_ANALOG_UP;

    }


    default:;

  }
  // should do the actions here.
  // app->state = STATE_HALT;


}


////////////////////////////////////////////


// JA remove
static bool strequal(const char *s1, const char *s2)
{
  return (strcmp(s1, s2) == 0);
}







static int current_from_unit(float i, const char *unit,  float *ii)
{
  // no unit, then assume voltage?

  if(strequal(unit, "A")) {
    *ii = i;
  } else if(strequal(unit, "mA")) {
    *ii = i * 1e-3f ;
  } else if(strequal(unit, "uA")) {
    *ii = i * 1e-6f ;
  } else if(strequal(unit, "nA")) {
    *ii = i * 1e-9f ;
  } else if(strequal(unit, "pA")) {
    *ii = i * 1e-12f ;
  } else {
    printf("unknown unit\n");
    // TODO error...
    *ii = 0;
    return -123;
  }
  return 0;
}


static int irange_and_iset_from_current(float i, irange_t *irange, float *iset)
{
  // range and iset from
  // we haie to extract the range and the adjusted float ialue...

  // assert(i >= 0);
  float ai = fabs(i);

#if 0
  if(i > 100) {
    // error
    *irange = 0;
    *iset = 0;
    return -123;
  } else
#endif
  if(ai > 3.f ) {
    *irange = 0;
    *iset = 0;
    return -123;
    // *irange = irange_100V;
    // *iset = i * 0.1;
  }
  else if(ai > 1) {
    *iset = i * 1;
    *irange = irange_10A;
  }
  else if(ai > 1e-1f) {
    *irange = irange_1A;
    *iset = i * 1e+1f;
  }
  else if(ai > 1e-2f)  {
    *irange = irange_100mA;
    *iset = i * 1e+2f;
  }
  else if(ai > 1e-3f)  {
    *irange = irange_10mA;
    *iset = i * 1e+3f;
  }
  else if(ai > 1e-4f)  {
    *irange = irange_1mA;
    *iset = i * 1e+4f;
  }
  else if(ai > 1e-5f)  {
    *irange = irange_100uA;
    *iset = i * 1e+5f;
  }
  else if(ai > 1e-6f)  {
    *irange = irange_10uA;
    *iset = i * 1e+6f;
  }
  else if(ai > 1e-7f)  {
    *irange = irange_1uA;
    *iset = i * 1e+7f;
  }
  else if(ai > 1e-8f)  {
    *irange = irange_100nA;
    *iset = i * 1e+8f;
  }
  else {
    *irange = irange_10nA;
    *iset = i * 1e+9f;
  }

  return 0;
}



static int voltage_from_unit(float v, const char *unit,  float *vv)
{
  // no unit, then assume voltage?

  if(strequal(unit, "V")) {
    *vv = v;
  } else if(strequal(unit, "mV")) {
    *vv = v * 1e-3f ;
  } else if(strequal(unit, "uV")) {
    *vv = v * 1e-6f ;
  } else {
    printf("unknown unit\n");
    // TODO error...
    *vv = 0;
    return -123;
  }
  return 0;
}

static int vrange_and_vset_from_voltage(float v, vrange_t *vrange, float *vset)
{
  // assert(v >= 0);
  float av = fabs(v);

  // range and vset from
  // we have to extract the range and the adjusted float value...
  if(av > 100) {
    // error
    *vrange = 0;
    *vset = 0;
    return -123;
  }
  else if(av > 10) {
    *vrange = vrange_100V;
    *vset = v * 0.1;
  }
  else if(av > 1) {
    *vset = v * 1;
    *vrange = vrange_10V;
  }
  else if(av > 0.1) {
    *vrange = vrange_1V;
    *vset = v * 10;
  }
  else  {
    *vrange = vrange_100mV;
    *vset = v * 100;
  }
  return 0;
}



#if 0

static void process_cmd(app_t *app, const char *s )
{
  UNUSED(app);

 // interesting.
  char  cmd[100];  *cmd = 0;
  char  param[100]; *param = 0;
  float value ;
  char  unit[100];  *unit = 0;

  // strtok. would be better, but will fail to distinguish value and unit without whitespace.
  // strtok, also modifies it's argument which isn't nice.
  // ie. ":set  v  123.4mV"
  // int n = sscanf(":set  v  123.45mV", "%100s %100s %f %100s", cmd, param, &value, unit);
  int n = sscanf(s, "%100s %100s %f %100s", cmd, param, &value, unit);
  if(n == 4 && strequal(cmd, ":set")  ) {

    if(strequal(param, "v")) {
      // lower(value);
      float v;
      if(voltage_from_unit(value, unit,  &v ) < 0) {
        printf("error converting voltage and unit\n");
        return;
      }
      printf("voltage %gV\n", v);
      vrange_t vset_range;
      float vset;
      if(vrange_and_vset_from_voltage(v, &vset_range, &vset) < 0) {
        printf("error converting voltage to range and vset\n");
        return;
      }
      printf("vrange %s, vset %gV\n", range_voltage_string(vset_range), vset);
      core_set( app, vset, app->iset, vset_range, app->iset_range);
    }

    else if(strequal(param, "i")) {
      float i;
      if(current_from_unit(value, unit,  &i ) < 0) {
        printf("error converting current and unit\n");
        return;
      }
      printf("current %gV\n", i);
      irange_t iset_range;
      float iset;
      if(irange_and_iset_from_current(i, &iset_range, &iset) < 0) {
        printf("error converting current to range and iset\n");
        return;
      }
      printf("irange %s, iset %gV\n", range_current_string(iset_range), iset);
      core_set( app, app->vset, iset, app->vset_range, iset_range);
    }
    else {

      printf("unrecognized parameter '%s'\n", param);
    }

  } else {

      printf("unrecognized command '%s'   tokens=%d, cmd='%s' param='%s'\n", s, n, cmd, param);
  }
}




static void update_console_ch(app_t *app, const char ch )
{
  // hange name update_console_ch()
  // printf("char code %d\n", ch );

  // change the actual current range
  if(ch == 'u' || ch == 'i') {

      // u - left is higher current, i right is lower
      irange_t new_irange = range_current_next( app->iset_range, ch == 'i' );
      if(new_irange != app->iset_range) {
        printf("change iset_range %s\n", range_current_string(new_irange) );
        app->iset_range = app->irange = new_irange;
        range_current_set(app, new_irange);
        dac_current_set(app, fabs(app->iset));
       //  core_set( app, app->vset, app->iset, app->vset_range, new_irange );
      }
  }
  // for voltage
  else if(ch == 'j' || ch == 'k') {

    // left is higher voltage, right is lower voltage.
    vrange_t new_vrange = range_voltage_next( app->vset_range, ch == 'k' );
    if(new_vrange != app->vset_range) {
      printf("change vset_range %s\n", range_voltage_string(new_vrange ) );
      app->vset_range = app->vrange = new_vrange;
      range_voltage_set(app, new_vrange);
      dac_voltage_set(app, fabs(app->vset));
      // core_set( app, app->vset, app->iset, new_vrange, app->iset_range );
    }
  }
  // toggle output... on/off. must only process char once. avoid relay oscillate
  else if( ch == 'o') {
    printf("output %s\n", (!app->output) ? "on" : "off" );
    mux_ice40(app->spi);
    output_set(app, app->irange, !app->output);
    // cBufPush(console_out, '\n');
  }
  // toggle printing of adc values.
  else if( ch == 'p') {
    printf("printing %s\n", (!app->print_adc_values) ? "on" : "off" );
    app->print_adc_values = ! app->print_adc_values;
    // cBufPush(console_out, '\n');
  }
  // halt
  else if(ch == 'h') {
    printf("halt \n");
    state_change(app, STATE_HALT);
    return;
  }
  // restart
  else if(ch == 'r') {
    printf("restart\n"); // not resume
    state_change(app, STATE_FIRST);
    return;
  }
}

#endif


#if 0
static void update_console_cmd(app_t *app)
{
  /*
    TODO

    should pass the app structure.
    for app->cmd_in at least

    put these buffers. in the app structure.
    Actually. no. it's neater that they're not.
  */

  // assert(&app->cmd_in);
  assert(&app->command);


  while( ! cBufisEmpty(&app->console_in)) {

    // got a character
    int32_t ch = cBufPop(&app->console_in);
    assert(ch >= 0);


    /*
      these are not actually useful UI functions....
    */

    // OK... we need to copy out the input buffer without consuming it...
    // or change this....

    // we're in a command
    if( !cBufisEmpty(&app->cmd_in) && cBufPeekFirst(&app->cmd_in) == ':') {
      // only push chars if we're in a command that starts with ':'
      // push ch to cmd buffer
      cBufPush(&app->cmd_in, ch);
      // echo the char to console
      cBufPush(&app->console_out, ch);
    }


    // not in a command...  so process ch
    else {

      // start a command
      if(ch == ':') {
        // push ch to cmd buffer
        cBufPush(&app->cmd_in, ch);
        // echo the char to console
        cBufPush(&app->console_out, ch);
      }

      update_console_ch(app, ch );
    }
  }


  if( !cBufisEmpty(&app->cmd_in) && cBufPeekLast(&app->cmd_in) == '\r') {

    // printf("got CR\n");

    // we got a carriage return
    static char tmp[1000];

    size_t nn = cBufCount(&app->cmd_in);

    size_t n = cBufCopyString(&app->cmd_in, tmp, ARRAY_SIZE(tmp));
    assert(n <= sizeof(tmp));
    assert(tmp[n - 1] == 0);

    assert( nn == n - 1);

    // TODO first char 'g' gets omitted/chopped here, why? CR handling?
    printf("got command '%s'\n", tmp);

    process_cmd(app, tmp);

  }
}

#endif




static void update_console_cmd(app_t *app)
{


  while( ! cBufisEmpty(&app->console_in)) {

    // got a character
    int32_t ch = cBufPop(&app->console_in);
    assert(ch >= 0);

    if(ch != '\r' && cStringCount(&app->command) < cStringReserve(&app->command) ) {
      // normal character
      cStringPush(&app->command, ch);
      // echo to output. required for minicom.
      putchar( ch);

    }  else {

      // newline or overflow
      putchar('\n');

      char *cmd = cStringPtr(&app->command);

      // printf("cmd whoot is '%s'\n", cmd);


      uint32_t u0;

      if( strcmp(cmd, "test") == 0) {

        printf("got test\n");

      }



      if( sscanf(cmd, "freq %lu", &u0 ) == 1) {

      }

      if( sscanf(cmd, "deadtime %lu", &u0 ) == 1) {



      }

      // reset buffer
      cStringClear( &app->command);

      // issue new command prompt
      printf("> ");
    }
  }
}
















/*
  is there something taking a really long time? async flush?
  OR are we blocking???? in the flush?

  such that we miss... the adc read??


*/



static void loop(app_t *app)
{
  // move this into the app var structure ?.
  static uint32_t soft_500ms = 0;
  static uint32_t soft_1s = 0;

  /*
    Think all of this should be done/moved to update()...

  */
  while(true) {


    // EXTREME - could actually call update at any time, in a yield()...
    // so long as we wrap calls with a mechanism to avoid stack reentrancy
    // led_update(); in systick.
    // but better just to flush() cocnsole queues.conin/out

//    update(app);

     /*
        JA
        we always want to update console first. so that we can always issue commands.
        and know that we have control
        -----------------

        All of this code needs to be refactored. so that the the command dispatch happens in the top-level loop.
    */

    update_console_cmd(app);


    // 500ms soft timer. should handle wrap around
    if( (system_millis - soft_500ms) > 500) {
      soft_500ms += 500;
      update_soft_500ms(app);
    }

    if( (system_millis - soft_1s) > 1000 ) {

      // THIS IS FUNNY....
      soft_1s += 1000;
      update_soft_1s(app);
    }



  }
}






static void assert_app(app_t *app, const char *file, int line, const char *func, const char *expr)
{
  /*
    note the usart tx interupt will continue to flush output buffer,
    even after jump to critical_error_blink()
  */
  printf("\nassert_app failed %s: %d: %s: '%s'\n", file, line, func, expr);

  state_change(app, STATE_HALT );

#if 1
  // we have to do a critical error here... else caller code will just progress,
  // if we were in a state transition, then it will continue to just progress...
  critical_error_blink();
#endif
}










/////////////////////////
/*
  TODO.
  Maybe move raw buffers into app structure?


  Why not put on the stack?
*/

static char buf_console_in[1000];
static char buf_console_out[1000];

// static char buf_cmds[1000];

static char buf_command[1000];



static float buf_vfb_measure[100];
static float buf_ifb_measure[100];


static float buf_vfb_range[100];
static float buf_ifb_range[100];





// move init to a function?
// no... because we collect/assemble dependencies. ok in main()
static app_t app;



/*
  TODO.
  OK. it would be very nice to know how many values are in the
  float circular buffer...

  Rather than counting interupts in a separate var .

*/












int main(void)
{
  // high speed internal!!!
  // TODO. not using.

	// rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ] );  // stm32f411  upto 100MHz.

  // this is the mcu clock.  not the adc clock. or the fpga clock.
  systick_setup(16000);

  // systick_setup(84000);  // 84MHz.

  // clocks
  rcc_periph_clock_enable(RCC_SYSCFG); // maybe required for external interupts?

  // LED
  // rcc_periph_clock_enable(RCC_GPIOE);
  rcc_periph_clock_enable(RCC_GPIOA);

  // USART
  // rcc_periph_clock_enable(RCC_GPIOA);     // f407
  rcc_periph_clock_enable(RCC_GPIOB); // F410 / f411

  rcc_periph_clock_enable(RCC_USART1);


  // spi / ice40
  rcc_periph_clock_enable(RCC_SPI1);

  /*
    Do led first, even if it only updates in loop().
  */

  //////////////////////
  // setup

  // led
  // led_setup();

  // JA
  // led blink
  // stm32f411...
#define LED_PORT  GPIOA
#define LED_OUT   GPIO9

  led_setup(LED_PORT, LED_OUT);






  //////////////////////
  // main app setup

  memset(&app, 0, sizeof(app_t));


  // JA
/*
  // uart/console
  cBufInit(&app.console_in,  buf_console_in, sizeof(buf_console_in));
  cBufInit(&app.console_out, buf_console_out, sizeof(buf_console_out));


  // command buffer
  cBufInit(&app.cmd_in, buf_cmds, sizeof(buf_cmds));
*/

  // uart/console
  cBufInit(&app.console_in,  buf_console_in, sizeof(buf_console_in));
  cBufInit(&app.console_out, buf_console_out, sizeof(buf_console_out));


  cStringInit(&app.command, buf_command, buf_command + sizeof( buf_command));

  // standard streams for printf, fprintf, putc.
  cbuf_init_stdout_streams(  &app.console_out );
  // for fread, fgetch etc
  cbuf_init_stdin_streams( &app.console_in );



  //////////////
  // initialize usart before start all the app constructors, so that can print.
  // uart
  // usart1_setup_gpio_portA();
  usart1_setup_gpio_portB();

  usart1_set_buffers(&app.console_in, &app.console_out);



  printf("\n\n\n\n--------\n");
  printf("addr main() %p\n", main );



  printf("\n--------");
  printf("\nstarting\n");


  assert( sizeof(bool) == 1);
  assert( sizeof(float) == 4);
  assert( sizeof(double ) == 8);





  //////////////////////////////


  // vfb buffer
  // fBufInit(&app.vfb_cbuf, buf_vfb, ARRAY_SIZE(buf_vfb));

  // measure
  fBufInit(&app.vfb_measure, buf_vfb_measure, ARRAY_SIZE(buf_vfb_measure));
  fBufInit(&app.ifb_measure, buf_ifb_measure, ARRAY_SIZE(buf_ifb_measure));

  // range
  fBufInit(&app.vfb_range, buf_vfb_range, ARRAY_SIZE(buf_vfb_range));
  fBufInit(&app.ifb_range, buf_ifb_range, ARRAY_SIZE(buf_ifb_range));




  
  ////////////////
  spi1_port_setup();
#if 0
  spi1_special_gpio_setup();
#endif
  // adc interupt...
  spi1_interupt_gpio_setup( (void (*) (void *))spi1_interupt, &app);


  ////////////////////

  app.spi = SPI1 ;
  app.print_adc_values = true;
  app.output = false;

  app.nplc_measure = 50;
  app.nplc_range   = 20;
  app.digits = 6;

  // app.vrange = 0;
  // app.irange = 0;



  state_change(&app, STATE_FIRST );

  loop(&app);

	for (;;);
	return 0;
}




