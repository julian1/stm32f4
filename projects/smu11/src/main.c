/*
  TODO
  - clear the bad code.  on adc OV. which happens when change range.
  - drill in range. that puts compliance at max of range.

  - done - add better default values... eg. 0.003mA = 3uA.

  - we need offset calibration - for measurement.   eg. mV on 10 or 100V range looks wrong.
      probably because of input offset voltage.
      correcting digitally. better. if can do this.
      need to get the output value right (dac), and the measurement value (dac + adc).
      - can trim some of this with ref pin of ina154.

  - it's only the attenuation range. 100V. that's bad? or too low into the noise?
  - we have to understand how the compliance regulation works, when the measurement on is on a different range.
  - ok. think the bounce out to the next range. if exceed compliance on a zoomed range. makes sense.
  ------------
  calibration uses two points eg. 10V and 0V. good.
    - change the dac value.
    - change the adc value.
  ------------
  EXTREME.
    actually. maybe it autoranges down. that's why when disconnected it always shows on lowest current range eg. 100uA.
      compliance *will* be respected, as it jummps up ranges until hits compliance range. if DUT source. it's just going to be staggered jumping out.

    So. long as it's on a lower range than the compliance range, then it is in compliance.
    on a lower range it *is* limiting.
    Actually use the dac to set +-10V.... which will limit.
    So. KNOWING when it triggers the end of the range.  eg. running into +-10V. is very

    So compliance can be limited by being in the compliance range, and at the compliance value.
    OR being on a higher range with compliance set to +-10V..
    YES.
    so the ranging would be a bit jumpy. but that's ok.
    and it's only the compliance that does the jumping. the source function will be fixed.
    ------------------

    Ahhh. set compliance to 1.05  etc.    and then anything in the range 1.02-1.05 should trigger a range switch.
    And this could use a hardware comparitor set at 10.2 to 10.5V
    In place of the slow ADC.   or else use the second fast adc. remembering that its buffered.
    Or just use slow adc to test.
    ----

    need vset  vset_range,    iset  iset_range.
    then change range based on value <1V or >10.5V etc.
    perhaps only change the compliance range.   is_compliance. can be a flag.   or vset_auto_range.

    eg. only changing one range. good to keep the source unchanged, when output relay is off.
    -------
    being able to set the dac to (eg. 10.5) so we have 0.5V headroom, so we can know we need to switch range is very useful.
    ---------

    the quadrant when sinking, when limit voltage, so current goes up - i think is still correct.
    ----------

    think range calibration,  is going to have to be defined as ab where y = ax + b.
      for dac. and for adc.
    -------------------

    TODO
      - done - print string of current range.
      - done - suppress the continuous printing.  - only print the state changes. eg. with 'p' key
      - done - if vset_irange == irange then print vset. else print 1.1
      - done - get V auto ranging also.  eg. down to 30mV when on. up to 5V when off.

      - done - test if can regulate on 10.4V...  or if adc generates errors.
            could be old errors. due to slow read rate.
            11V ok. can read. 11.2 fails. good.

      - done - actually maybe add the 10A range. first. to get it correct.

      - done - range refactoring. eg. comX,comy etc.
      - done - state changes should be functions.  eg.  state_analog_up( ) should encode wha'ts needed then set the app->state var.
          state_change_()


      - done - irange, vrange should be set in range_current_set()... eg. they follow calls to this functino.
          because range_current_set()   is the setting of the measure range.

          IMPORTNAT
          active_irange.
          active_vrange.
          active_range_current_set()...
          active_range_voltage_set()...

      - want to count the number of adc interupts/s.

      - should populate and test the single-ended gain stage.
          good to be complete - and see in practice.  even if offset voltage is not adjusted.
          could  probably even bodge a dac output - to the amp03 ref .
          no. because want resistor divider/ and op-amp for more prec than the dac.
          actually could try without.


        - also want to check that we service them all. unbalanced.
        - also want freq control over adc.  so can do interval of 50/60ms.
      - also count /s the number of times update is called. and we sample. the rails monitors.

    - smartsmu. quicksmu.

    - 10^4 gain. use for LNA noise measurement. of vrefs?  just need large AC blocking caps.
        we can bodge. by lifting the amp03 ref pin - and connecting the dac to it.
        set to sink current. and it should go into voltage compliance mode. if connected to a cap. to measure noise.
        - actually just measure on the sense input. no need to source or sink. 
        - adc using 24 bit adc.
        - low pass. using adc values.

    - mcp3208 - driven by spi. Can we make it so that the spi never blocks. no. because we use same spi channel for all other coms.
        we have to offload it to the fpga. if want high speed continuous.

    - think calibration - is easy. just do the offset. for zero V.  then adjust slope.
        both dac, and adc. get done the same way.

    - use uint64_t ? if available for systick? to avoid rollover?

    - using 74hc4094 if we need it is easy. we just do the mux_io thing.  and then toggle nss as a strobe instead of cs. using gpio.
        simple. eg. we recognifure spi in other contexts so don't see there's any issue.

    - test bav199 with 10G input impedance of 34401a.

    - // want a function that can print max of 4,5,6 digits...

      - add a logic check somewhere. that comz relay is not on and output lc relay also on.
          in main update loop. should be simple.

      - change the output relay logic.  if on, then large relay always on. just turn the lc relay off if on high-current range to protect it.

      - note. input bias currents - on schematic - next to ops, and jfets.

      - support console,  input values for vset, iset. then we have a working unit.
        perhaps numeric num system.

      - use case stmt with float format to control output digits

      - promopt for number eg.    'v 30.1mA'. 1A  etc.  then select range etc.
          need sscanf...

      - check we're actually using hsi. clocks.  we're using 16MHz. i think because that's the systick divider.
          check spi performance.

      - update to libc print format
        sizes,
        using miniprintf code.
           text    data     bss     dec     hex filename
          13860       8    4072   17940    4614 main.elf
          353K  main.elf

          using gnu libc. vsnprintf, sscanf etc.
           text    data     bss     dec     hex filename
          31864    2484    4168   38516    9674 main.elf
          354K   main.elf

      - small cheap bipolar 12bit dac.  would be better than adg333 and prec resistors. for offset nulling . for voltage ranging.
          or use the dac8734


      - oranganize high/low level. even if share same data structures .
          console processing. from low level core control.
          use files.
          regulate.c   operation.c

      - we may need an adc filter. lowpass for the ranging.

      - add a adc halt current condition. on any range.

      - use fast loop, for adc range / and auto range.
        - this is more complicated. actually need interupt / register on the fpga/ spi interupt line.
        - actually could probably poll/read fpga.
        - need to propagate through fpga. pin. is on the the mcu gpio. so use mcu interupt.

      - change to 40k count dac.
      - use wrapper functions for setting dac values - actually maybe not.  but need to handle vset.
      - get adc slope and offset. working. should then be able to do a two point cal.

      - think that using the slow adc. for autoranging may actually be better.
          can also mix fast and slow reading.

      - naming modes.    operation vs compliance.

      - need to test the fpga high-z state. with a 10k pull-up. and see what values we get. eg. to test fpga witing flash.

    --------------

    A value like 1.02V needs to be able to be regulate on either range (either 1.02 or 0.102). else won't get range switch stability.

*/
// vim :colorscheme default. loooks good.

// cjmcu  stm32f407.
// issue. is that board/stlink doesn't appear to reset cleanly. needs sleep.

// - can plug a thermocouple straight into the sense +p, +n, and
// then use the slow (digital) integrator/pid loop - as a temperature controler..


#include <libopencm3/stm32/rcc.h>

//#include <libopencm3/stm32/gpio.h>
//#include <libopencm3/cm3/nvic.h>
//#include <libopencm3/cm3/systick.h>
//#include <libopencm3/stm32/exti.h>
//#include <libopencm3/stm32/timer.h>
//#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/spi.h>


#include <stddef.h> // size_t
#include <math.h> // nanf   fabs
#include <stdio.h>    // sscanf
#include <string.h>   // strcmp


#include "buffer.h"
#include "usart2.h"
#include "util.h"


#include "spi1.h"
#include "mcp3208.h"
#include "w25.h"
#include "dac8734.h"
#include "ads131a04.h"

#include "core.h"   // some of the above files include core.h. need header guards.

#define UNUSED(x) (void)(x)

// here? or in mux.h.
#define SPI_ICE40       SPI1





/*
  AHHH. is clever
    74hc125.
    if fpga gpio comes up high - OE - is disabled. so out is low.
    if fpga gpio comes up lo   - buffer is disabled so out low.

    there seems to be a 50-100uS spike though on creset.
    hopefully too short a time to turn on opto coupler.
*/





// TODO prefix these... ST_FIRST... etc.
// also want a DONE state.



typedef enum state_t {
  FIRST,        // change name INITIAL?
  DIGITAL_UP,   // DIGIAL_DIGITAL_UP
  // ERROR,
  ANALOG_UP,
  HALT
} state_t;

// static
// should probably be put in state record structure, rather than on the stack?
// except would need to pass by reference.
// static state_t state = FIRST;




/*
  VERY IMPORTANT.
  we need to iterate all the ranges. even if we don't use them. so that can test them.
  A switch statement. is almost certainly going to be easier separate functions.
*/


typedef enum vrange_t
{
  vrange_100mV = 3,
  vrange_1V,
  vrange_10V,
  vrange_100V // ,

  // vrange_10V_2,

} vrange_t;



/*
  there is no *off* or *alternate* current range/path. when output is disconnected.
    - instead when off / output is disconnected - auto ranging should zoom down to COM_Z  highest value resistor.
    - so there is always an active current path.

    that keeps com_lc parked near gnd. and allows VFB to work. and a current reading for high-impedance disconnect state.
*/


typedef enum irange_t
{
  // TODO rename range_current_none, range_current_1x etc.

  irange_1uA = 3,

  irange_10uA ,
  irange_100uA,
  irange_1mA,
  irange_10mA,
  irange_100mA,
  irange_1A,
  irange_10A

} irange_t;




typedef struct app_t
{
  uint32_t spi;

  state_t   state;

  ////////////////
  // the current active ranges used for regulation and measurement.
  // may be narrower than the set range
  vrange_t  vrange;
  irange_t  irange;

  ////////////////
  float     vset;
  vrange_t  vset_range;

  float     iset;
  irange_t  iset_range;


  bool      print_adc_values;

  bool      auto_range_measurement;   // use 'a' to toggle. would be useful to test.
                                      // when turn off. will need a core reset?

  // bool      last_char_newline; // last console char
  // we could eliminate this. if we were to read the relay register...
  bool      output;   // whether output on/off



} app_t;


//////////////////////////

static void output_set(app_t *app, irange_t irange, uint8_t val);


static void state_change(app_t *app, state_t state );

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

  // suppress compiler warning...
  critical_error_blink();
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
  // suppress compiler warning...
  return "error";
}


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
  return -99999;
}



static void error(app_t *app, const char *msg)
{
  // actually going to halt condition. is better...  except we have to pass the app argument...
  // critical_error_blink();

  usart_printf("error\n");
  usart_printf( msg);
  usart_printf("\n");
  state_change(app, HALT);
}



static void dac_current_set(app_t *app, float i)
{
  // wrapper over raw spi is good ...
  // can record the value
  // dac_vset, dac_iset.


  // value should always be positive...
  if(i < 0) {
    error(app, "dac_current set i negative");
    return;
  }

  mux_dac(app->spi);
  spi_dac_write_register(app->spi, DAC_VOUT1_REGISTER, voltage_to_dac( i ));
}


static void dac_voltage_set(app_t *app, float v)
{
  // wrapper over raw spi is good ...
  // can record the value
  // dac_vset, dac_iset.


  // value should always be positive...
  if(v < 0) {
    error(app, "dac_voltage set v negative");
    return;
  }

  mux_dac(app->spi);
  // spi_dac_write_register(app->spi, DAC_VOUT0_REGISTER, voltage_to_dac( fabs( v )) );
  spi_dac_write_register(app->spi, DAC_VOUT0_REGISTER, voltage_to_dac( v ));
}





static void range_voltage_set(app_t *app, vrange_t vrange)
{
/*
  shouldn't be called directly
  because autoranging - means we may be off the range. and dac value is 11.

  we need a function to set the voltage range. and set the dac value.

*/
  usart_printf("range_voltage_set %s -> %s\n", range_voltage_string(app->vrange), range_voltage_string(vrange));

  mux_io(app->spi);   // would be better to avoid calling if don't need.

  app->vrange = vrange;

  switch(app->vrange)
  {

    case vrange_10V:
      // usart_printf("10V range \n");
      // flutters at 5 digit. nice.
      // 6th digit. with 9V and 0V.
      io_write(app->spi, REG_INA_VFB_ATTEN_SW, INA_VFB_ATTEN_SW1_CTL);       // no atten
      io_write(app->spi, REG_INA_VFB_SW, ~INA_VFB_SW1_CTL);                  // x1 direct feedback. works.
      break;


    case vrange_1V:
      // usart_printf("1V range \n");
      io_write(app->spi, REG_INA_VFB_ATTEN_SW, INA_VFB_ATTEN_SW1_CTL);       // no atten
      io_write(app->spi, REG_INA_VFB_SW, ~INA_VFB_SW2_CTL);                  // 10x gain
      break;

    case vrange_100mV:

      // usart_printf("100mV range \n");
      io_write(app->spi, REG_INA_VFB_ATTEN_SW, INA_VFB_ATTEN_SW1_CTL);       // no atten
      io_write(app->spi, REG_INA_VFB_SW, ~INA_VFB_SW3_CTL);                  // 100x gain
      break;


  case vrange_100V:
      // usart_printf("100V range \n");
      // flutters at 4th digit. with mV.  but this is on mV. range... so ok?
      // at 6th digit with V.  eg. 9V and 0.1V. - very good - will work for hv.
      io_write(app->spi, REG_INA_VFB_ATTEN_SW, INA_VFB_ATTEN_SW2_CTL | INA_VFB_ATTEN_SW3_CTL);    // atten = 0.1x
      io_write(app->spi, REG_INA_VFB_SW, ~INA_VFB_SW1_CTL);                  // x1 direct feedback. works.
      break;


    // IMPORTANT - remember have the other attenuation possibility... of just turning on/off sw3.
#if 0
  case vrange_10V_2:
      // flutters at 4th digit.
      io_write(app->spi, REG_INA_VFB_ATTEN_SW, INA_VFB_ATTEN_SW2_CTL | INA_VFB_ATTEN_SW3_CTL);    // atten = 0.1x
      io_write(app->spi, REG_INA_VFB_SW, ~INA_VFB_SW2_CTL);                                   // x10 . works.
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

    case irange_1uA:    return "1uA" ;

    case irange_10uA:   return "10uA" ;
    case irange_100uA:  return "100uA";
    case irange_1mA:    return "1mA";
    case irange_10mA:   return "10mA";
    case irange_100mA:  return "100mA";
    case irange_1A:     return "1A";
    case irange_10A:    return "10A";
  };

  // suppress compiler warning...
  return "error";
}





static irange_t range_current_next( irange_t irange, bool dir)
{
  /// dir==1 ve == lower current
  // can simplify - enum addition ... etc.
  // but this makes it pretty clear

  if(dir) {
    // lower current range. ie. higher value shunt resistor.
    switch(irange)
    {
      case irange_1uA:   return irange_1uA;  // no change

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
      case irange_1uA:   return irange_10uA;

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

  usart_printf("range_current_switch %s -> %s\n", range_current_string(app->irange), range_current_string(irange));

  mux_io(app->spi);   // would be better to avoid calling if don't need.

  app->irange = irange;


  switch(app->irange)
  {

    case irange_10A:
    case irange_1A:
    case irange_100mA:
    case irange_10mA:

      // VERY IMPORTANT - ensure high current output relay is on. and low current relay off.
      // before make changes that might increase current.
      output_set(app, app->irange, app->output);
      msleep(1);

      // turn on current range x
      io_write(app->spi, REG_RELAY_COM,  RELAY_COM_X);

      // turn off jfets switches for Y and Z ranges.
      io_write(app->spi, REG_IRANGE_YZ_SW, 0);

      switch(app->irange) {

        case irange_10A:
        case irange_1A:
          // usart_printf("1A range \n");

          switch(app->irange) {
            // gain 10x on 0.1ohm, for 10V range. active low
            case irange_10A:
              io_write(app->spi, REG_INA_IFB_SW,  ~INA_IFB_SW2_CTL);
              break;
            // gain 100x on 0.1ohm.  active low
            case irange_1A:
              io_write(app->spi, REG_INA_IFB_SW,  ~INA_IFB_SW3_CTL);
              break;
            default:
              // cannot be here...
              critical_error_blink();
              return;
          };

          // turn on sense amplifier 1
          io_write(app->spi, REG_ISENSE_MUX,  ~ISENSE_MUX1_CTL);
          // turn on first set of big fets.
          io_write(app->spi, REG_IRANGE_X_SW, IRANGE_X_SW1_CTL);
          break;


        // 10ohm resistor. for 10V swing.
        case irange_100mA:
          // ensure sure the high current relay is on. before switching
          // gain 10x active low
          io_write(app->spi, REG_INA_IFB_SW,  ~INA_IFB_SW2_CTL);
          // turn on sense amplifier 2
          io_write(app->spi, REG_ISENSE_MUX,  ~ISENSE_MUX2_CTL);
          // turn on 2nd switch fets.
          io_write(app->spi, REG_IRANGE_X_SW, IRANGE_X_SW2_CTL);
          break;

        // 1k resistor. for 10V swing.
        case irange_10mA:
          // gain 1x active low
          io_write(app->spi, REG_INA_IFB_SW,  ~INA_IFB_SW1_CTL);
          // turn on sense amplifier 3
          io_write(app->spi, REG_ISENSE_MUX,  ~ISENSE_MUX3_CTL);
          // turn on 4th switch fets.
          io_write(app->spi, REG_IRANGE_X_SW, IRANGE_X_SW4_CTL);
          break;

        default:
          // cannot be here.
          critical_error_blink();
          return;
      }
      return;



    // 10k resistor. for 10V swing
    case irange_1mA:
    // 100k resistor for 10V swing.
    case irange_100uA:
    // 1M resistor for 10V swing.
    case irange_10uA:
      // turn on current range relay y
      io_write(app->spi, REG_RELAY_COM,  RELAY_COM_Y);
      // turn off all fets used on comx range
      io_write(app->spi, REG_IRANGE_X_SW, 0 );
      // gain 1x active low
      io_write(app->spi, REG_INA_IFB_SW,  ~INA_IFB_SW1_CTL);
      // turn on sense amplifier 3
      io_write(app->spi, REG_ISENSE_MUX,  ~ISENSE_MUX3_CTL);

      switch( app->irange) {
        case irange_1mA:
          // turn on jfet 1
          io_write(app->spi, REG_IRANGE_YZ_SW, IRANGE_YZ_SW1_CTL);
          break;
        case irange_100uA:
          // turn on jfet 2
          io_write(app->spi, REG_IRANGE_YZ_SW, IRANGE_YZ_SW2_CTL);
          break;
        case irange_10uA:
          // turn on jfet 2
          io_write(app->spi, REG_IRANGE_YZ_SW, IRANGE_YZ_SW3_CTL);
          break;
        default:
          // cannot be here.
          critical_error_blink();
          return;
      }

      // turn off high current output relay... if need be. only after new range in effect
      msleep(1);
      output_set(app, app->irange, app->output);
      break;

    // 10M for 10V swing.
    case irange_1uA:
      // IMPORTANT DONT forget to add star jumper to star gnd!!!.
      // turn on current range relay Z
      io_write(app->spi, REG_RELAY_COM,  RELAY_COM_Z);
      // turn off all fets used on comx range
      io_write(app->spi, REG_IRANGE_X_SW, 0 );
      // gain 1x active low
      io_write(app->spi, REG_INA_IFB_SW,  ~INA_IFB_SW1_CTL);
      // turn on sense amplifier 3
      io_write(app->spi, REG_ISENSE_MUX,  ~ISENSE_MUX3_CTL);

      // turn on jfet 1
      io_write(app->spi, REG_IRANGE_YZ_SW, IRANGE_YZ_SW1_CTL);

      // turn off high current output relay... if need be. only after new range in effect
      msleep(1);
      output_set(app, app->irange, app->output);
      break;


  }
}

/*
  OK. perhaps to prevent range change instability.
  we don't zoom away from the operation range. beit voltage or current?


*/


static float range_current_multiplier( irange_t irange)
{
  switch(irange)
  {
    // ie. expressed on 10V range

    case irange_1uA:    return 1e-7f;
    // case irange_1uA:    return 0.0000001f;

    case irange_10uA:   return 0.000001f;
    case irange_100uA:  return 0.00001f;
    case irange_1mA:    return 0.0001f;
    case irange_10mA:   return 0.001f;
    case irange_100mA:  return 0.01f;
    case irange_1A:     return 0.1f;
    case irange_10A:    return 1.f;
  };

  return -9999;
}



static bool range_current_auto(app_t *app, float i)
{
  bool changed = false;



  if(fabs(i) < 1.f) {

    // need to switch to lower current range
    irange_t lower = range_current_next( app->irange, 1);
    if(lower != app->irange) {
      usart_printf("i is %f\n", i);
      usart_printf("ZOOM IN current.\n");
      range_current_set(app, lower);
      changed  = true;
    }
  }
  else if (fabs(i) > 10.5 && app->irange < app->iset_range) {

    // switch out to a higher current range
    irange_t higher = range_current_next( app->irange, 0);
    if(higher != app->irange) {
      usart_printf("i is %f\n", i);
      usart_printf("ZOOM OUT current\n");
      range_current_set(app, higher);
      changed = true;
    }
  }


  if(changed == true)
  {
    if(app->irange == app->iset_range) {

      // ranges the same
      usart_printf("use regulation current %f\n", app->iset);
      dac_current_set(app, fabs(app->iset));
    } else if( app->irange < app->iset_range ) {

      // we're zoomed in,
      usart_printf("use zoomed in current 11V on range\n");
      dac_current_set(app, 11.f );

    } else {
      // bad condition
      usart_printf("BAD\n");
    }
  }

  return changed;
}



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
      usart_printf("v is %f\n", v);
      usart_printf("ZOOM in voltage\n");
      range_voltage_set(app, lower);
      changed = true;
    }
  }
  else if (fabs(v) > 10.5 && app->vrange < app->vset_range )
  {
    // we have to jump out... but don't jump out past the actual regulation range (vset_range)
    // else we'll be regulating on higher range than the set range
    vrange_t higher = range_voltage_next( app->vrange, 0);
    if(higher != app->vrange) {     // there is a higher range.
      usart_printf("v is %f\n", v);
      usart_printf("ZOOM out voltage\n");
      range_voltage_set(app, higher);
      changed = true;
    }
  }


  if(changed == true ) {

    if(app->vrange == app->vset_range) {

      usart_printf("use regulation voltage %f\n", app->vset);
      dac_voltage_set(app, fabs(app->vset));
    } else if (  app->vrange < app->vset_range   ) {

      usart_printf("use zoomed in voltage 11V on range\n");
      dac_voltage_set(app, 11.f );
    } else {
      // bad condition.
      usart_printf("HERE BAD v.\n");
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
  // usart_printf("output %s\n", app->output ? "on" : "off"  );

  if(app->output) {

      // switch( app->irange)
      switch( irange)
      {
        case irange_1uA:

        case irange_10uA:
        case irange_100uA:
        case irange_1mA:
          // turn on read relay
          // and turn off the hc relay.
          io_write(app->spi, REG_RELAY_OUT, RELAY_OUT_COM_LC);
          io_set(app->spi, REG_LED, LED2);
          break;

        case irange_10mA:
        case irange_100mA:
        case irange_1A:
        case irange_10A:
          // high current relay
          io_write(app->spi, REG_RELAY_OUT, RELAY_OUT_COM_HC);
          io_clear(app->spi, REG_LED, LED2);
          break;
      }
  }
  else {


    io_write(app->spi, REG_RELAY_OUT, 0 ); // both relays off
    io_clear(app->spi, REG_LED, LED2);
  }
}












// want a function that can print max of 4,5,6 digits...





static void print_current(irange_t irange, float val)
{
  /*
    improtant.
      formatting measured values, according to selected range (rather than value) is correct.
      enourage drill to a higher range.

  */
  // usart_printf(" here " );

  switch( irange)
  {
    // not sure whether we should care about this...

    case irange_1uA:
      // when power is off... kind of nice to report...
      if(fabs(val) * 1e9f > 1)
        usart_printf("%fnA", val * 1e9f);
      else
        // this will be more valid, with a higher valued resistor 100M or 1G.
        usart_printf("%fpA", val * 1e12f);
      break;


    case irange_10uA:
    case irange_100uA:
    case irange_1mA:
      usart_printf("%fuA", val * 1000000.f);
      break;

    case irange_10mA:
    case irange_100mA:
    case irange_1A:
      usart_printf("%fmA", val * 1000.f);   // TODO 0.7A better as 0.7A. 0.6A better as 600mA. think..
      break;


    // case irange_1A:
    case irange_10A:
      usart_printf("%fA", val);
      break;


  }
}


static void print_voltage(vrange_t vrange, float val)
{
  // ie expressed on 10V range
  switch(vrange)
  {
    case vrange_100V:
    case vrange_10V:
      usart_printf("%fV", val);
      break;

    case vrange_1V:
    case vrange_100mV:
      usart_printf("%fmV", val * 1000.f);
      break;

  }
}


/*
  EXTR.
  This 500ms. update. is very good. allows relays to all settle
  before consider ranging again.

  can speed it up. eg. 50ms. for more prod like.
  and accumulate or filter the adc.


  but we need to have the adc read, on the adc interupt.
  then have this soft range update. use the last recorded adc value , rather than read the adc here.
*/


/*
  OK. the DRDY propagated through ice40.
  it's only a 2uS. pulse. every 10ms.
  So we will have to be interupt to guarantee catching it.

  should be an exposed test-point.

*/


static void update_soft_500ms(app_t *app )
{


  static uint32_t count = 0; // -1
  ++count;    // increment first. because state machine funcs return early.


  // blink mcu led
  led_toggle();

  mux_io(app->spi);

  ////////////////////////////////
  // clear led1


  io_toggle(app->spi, REG_LED, LED1);



  switch(app->state) {


    case ANALOG_UP: {
      // normal operation


      // ... ok.
      // how to return. pass by reference...
      float ar[4];
      // change name adc_spi_do_read
      // spi_adc_do_read(app->spi, ar, 4);

      int32_t ret = spi_adc_do_read(app->spi, ar, 4);
      if(ret < 0)
        break;

      /*
        ranging and format precision are separate and vary independently.
        so need to use common unit approach.
      */
      float x = 0.435;

      // convert to standard unit. eg. volts or amps.
      // change name range_voltage_si_coeff or similar
      float v = ar[0] * x;      // these are the current ranges....
      float i = ar[1] * x;


      if(app->print_adc_values) {

        // when we set the range. we should set the default format.
        // the format prec wants to be able to user modified.
        /////////////////

        usart_printf("vrange: %s", range_voltage_string(app->vrange));
        usart_printf("(%s)",       range_voltage_string(app->vset_range));

        if(app->vrange == app->vset_range) {
          usart_printf("*");
          usart_printf(", vset ");
          print_voltage(app->vrange, app->vset * range_voltage_multiplier(app->vrange));
        }
        usart_printf(", vfb ");
        print_voltage(app->vrange, v * range_voltage_multiplier(app->vrange)  );

        /////////////////

        usart_printf("    ");


        usart_printf("irange: %s", range_current_string(app->irange));
        usart_printf("(%s)",       range_current_string(app->iset_range));

        if(app->irange == app->iset_range) {
          usart_printf("*");
          usart_printf(", iset ");
          print_current(app->irange, app->iset * range_current_multiplier(app->irange) );
        }
        usart_printf(", ifb ");
        print_current(app->irange, i * range_current_multiplier(app->irange));
        usart_printf("\n");
      }

      // usart_printf("i is %f\n", i);
      // usart_printf("v is %f\n", v);


      // TODO. change back so that can change both together,

      bool changed_current = range_current_auto(app, i );
      if(!changed_current)
        range_voltage_auto(app, v);

      break;
    }

    default: ;
  };
}




static void quadrant_set( app_t *app, bool v, bool i)
{
  // RULES.
  // so. if voltage is positive use clamp max.  clamp min/max follows voltage.
  // negative current. can still be source or sink. depending on polarity.
  // ie. clamp direction min/max following voltage.

  mux_io(app->spi);

  uint32_t vv = v ? CLAMP1_VSET_INV : CLAMP1_VSET;
  uint32_t ii = i ? CLAMP1_ISET_INV : CLAMP1_ISET;


  io_write(app->spi, REG_CLAMP1, ~(vv | ii ));


  uint32_t minmax = v ?  CLAMP2_MAX : CLAMP2_MIN;

  io_write(app->spi, REG_CLAMP2, ~( minmax ) );     // min of current or voltage
}


// so can have another function. that tests the values.... v > 0 etc.
// need to hide

static void core_set( app_t *app, float v, float i, vrange_t vrange, irange_t irange)
{
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







static void state_change(app_t *app, state_t state )
{
  switch(state) {

    case FIRST:
      usart_printf("-------------\n" );
      usart_printf("first\n" );


      app->state = FIRST;
      break;


    case HALT: {

      usart_printf("-------------\n" );
      usart_printf("change to halt state\n" );

      mux_io(app->spi);
      // turn off power
      io_clear(app->spi, REG_RAILS, RAILS_LP15V | RAILS_LP30V | RAILS_LP60V);

      // turn off output relays
      output_set(app, app->irange, false );
      app->state = HALT;
      break;
    }


    case DIGITAL_UP: {

      // if any of these fail, this should progress to error
      usart_printf("-----------\n");
      usart_printf("digital start\n" );

      mux_io(app->spi);

      ////////////
      // soft reset is much better here.
      // avoid defining initial condition. in more than one place
      // so define in fpga.
      io_clear(app->spi, CORE_SOFT_RST, 0);    // any value addressing this register.. to clear

      // no. needs dg444/mux stuff. pulled high. for off.
      // BUT I THINK we should probably hold RAILS_OE high / deasserted.


      // test the flash
      // TODO. check responses.
      mux_w25(app->spi);
      spi_w25_get_data(app->spi);

      // dac init
      int ret = dac_init(app->spi, REG_DAC); // bad name?
      if(ret != 0) {
        state_change(app, HALT);
        // app->state = ERROR;
        return;
      }

      // TODO remove.... fix regualte on vfb.
      usart_printf("-------------\n" );
/*
      usart_printf("set voltage range\n" );
      range_voltage_set(app, vrange_10V);

      usart_printf("set current range\n" );
      range_current_set(app, irange_10mA);
*/
      // progress to digital up?
      usart_printf("digital up ok\n" );
      app->state = DIGITAL_UP;
      break;
    }


    case ANALOG_UP: {


      usart_printf("turn on lp5v\n" );
      mux_io(app->spi);
      // assert rails oe
      io_clear(app->spi, REG_RAILS_OE, RAILS_OE);

      // turn on 5V digital rails
      io_set(app->spi, REG_RAILS, RAILS_LP5V );
      msleep(50);

      // turn on +-15V rails
      usart_printf("turn on analog rails - lp15v\n" );
      io_set(app->spi, REG_RAILS, RAILS_LP15V );
      msleep(50);

      // LP30 - needed to power the vfb topside op amp. ltc6090/ bootstrapped
      // io_set(spi, REG_RAILS, RAILS_LP30V );
      // msleep(50);
       /*
      io_set(spi, REG_RAILS, RAILS_LP60V );
       */

#if 0
      // TODO EXTREME . set the gain switches before turning on rails.
      // IMPORTANT - should probably do this. before switching on the supplies.
      // so that vrange ops are not high-Z
      usart_printf("turn on voltage range\n" );
      range_voltage_set(spi);

      usart_printf("turn on current range\n" );
      range_current_set(spi);
#endif

      // turn on refs for dac
      //mux_dac(spi);
      usart_printf("turn on ref a for dac\n" );
      mux_io(app->spi);
      io_write(app->spi, REG_DAC_REF_MUX, ~(DAC_REF_MUX_A | DAC_REF_MUX_B)); // active lo

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

      // core_set( app, -5.f , -5.f );    // -5V compliance, -1mA  sink.
      // core_set( app, 5.f , 3.f, vrange_10V, irange_10mA );         // 5V source, 5mA compliance,
      // core_set( app, 0.5f , 3.f, vrange_10V, irange_10mA );         // oscillates.
      core_set( app, 5.f , 3.f, vrange_1V, irange_10mA );         // 5V source, 5mA compliance,
      // core_set( app, -5.f , -3.f, vrange_1V, irange_10mA );         // 5V source, 5mA compliance,
      // core_set( app, 11.0f , 3.0f );         // 5V source, 5mA compliance,
      // core_set( app, -5.f , -3.0f );         // 5V source, 5mA compliance,

      // 9.8 no.

      // 6.8V + 6.8mA = 13.6V which is +-15V limit.   OK. we're limited by supply headroom. for current sense drop and voltage drop. hmmm.
      //
/*
      app->vset_range = vrange_10V;
      app->iset_range = irange_10mA;

      range_voltage_set(app, vrange_10V);
      range_current_set(app, irange_10mA);
*/

      // the voltage - is not actually changing with voltage set... ?/

      /////////////
      // working as bipolar.
      spi_dac_write_register(app->spi, DAC_VOUT2_REGISTER, voltage_to_dac( -2.f ) );  // outputs -4V to tp15.  two's complement works. TODO but need to change gain flag?
      spi_dac_write_register(app->spi, DAC_VOUT3_REGISTER, voltage_to_dac( 0.f ) );  // outputs 4V to tp11.



      // change namem output relay?
      output_set(app, app->irange, true );   // turn on



      /////////////////
      // adc init has to be done after rails are up...
      // but doesn't need xtal, to respond to spi.
      // adc init
      int ret = adc_init(app->spi, REG_ADC);
      if(ret != 0) {
        // app->state = ERROR;

        state_change(app, HALT );
        return;
      }


      app->state = ANALOG_UP;

    }


    default:;

  }
  // should do the actions here.
  // app->state = HALT;


}



static void update(app_t *app)
{
  // called as often as possible

  /*
    querying adc03 via spi, is slow (eg. we also clock spi slower to match read speed) .
    so it should only be done in soft timer eg. 10ms is probably enough.
    preferrably should offload to fpga with set voltages, -  and fpga can raise an interupt.
  */

  // get supply voltages,
  mux_adc03(app->spi);
  // TODO put cal values in state
  float lp15v = spi_mcp3208_get_data(app->spi, 0) * 0.92 * 10.;
  float ln15v = spi_mcp3208_get_data(app->spi, 1) * 0.81 * 10.;
  // UNUSED(lp15v);
  // UNUSED(ln15v);
  // usart_printf("lp15v %f    ln15v %f\n", lp15v, ln15v);



  switch(app->state) {

    case FIRST:

      state_change(app, DIGITAL_UP);
      break;


    case DIGITAL_UP:
      if(lp15v > 15.0 && ln15v > 15.0 )
      {
        usart_printf("-----------\n");

        usart_printf("lp15v %f    ln15v %f\n", lp15v, ln15v);
        usart_printf("doing analog up -  supplies ok \n");
        state_change(app, ANALOG_UP);
      }

      break ;


    case ANALOG_UP:

      if((lp15v < 14.7 || ln15v < 14.7)  ) {

        usart_printf("supplies bad - turn off rails\n");
        usart_printf("lp15v %f    ln15v %f\n", lp15v, ln15v);

        state_change(app, HALT);
      }

      break;


    case HALT:
      break;


    default:
      ;
  };

}




////////////////////////////////////////////





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
#if 0
  if(i > 100) {
    // error
    *irange = 0;
    *iset = 0;
    return -123;
  } else
#endif
  if(i > 3.f ) {
    *irange = 0;
    *iset = 0;
    return -123;
    // *irange = irange_100V;
    // *iset = i * 0.1;
  } else if(i > 1) {
    *iset = i * 1;
    *irange = irange_10A;
  } else if(i > 1e-1f) {
    *irange = irange_1A;
    *iset = i * 1e+1f;
  } else if(i > 1e-2f)  {
    *irange = irange_100mA;
    *iset = i * 1e+2f;
  } else if(i > 1e-3f)  {
    *irange = irange_10mA;
    *iset = i * 1e+3f;

  } else if(i > 1e-4f)  {
    *irange = irange_1mA;
    *iset = i * 1e+4f;

  } else if(i > 1e-5f)  {
    *irange = irange_100uA;
    *iset = i * 1e+5f;

  } else { // if(i > 1e-6f)  {
    *irange = irange_10uA;
    *iset = i * 1e+6f;
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
  // range and vset from
  // we have to extract the range and the adjusted float value...
  if(v > 100) {
    // error
    *vrange = 0;
    *vset = 0;
    return -123;
  } else if(v > 10) {
    *vrange = vrange_100V;
    *vset = v * 0.1;
  } else if(v > 1) {
    *vset = v * 1;
    *vrange = vrange_10V;
  } else if(v > 0.1) {
    *vrange = vrange_1V;
    *vset = v * 10;
  } else  {
    *vrange = vrange_100mV;
    *vset = v * 100;
  }
  return 0;
}




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
        usart_printf("error converting voltage and unit\n");
        return;
      }
      usart_printf("voltage %gV\n", v);
      vrange_t vset_range;
      float vset;
      if(vrange_and_vset_from_voltage(v, &vset_range, &vset) < 0) {
        usart_printf("error converting voltage to range and vset\n");
        return;
      }
      usart_printf("vrange %s, vset %gV\n", range_voltage_string(vset_range), vset);
      core_set( app, vset, app->iset, vset_range, app->iset_range);
    }
    else if(strequal(param, "i")) {
      float i;
      if(current_from_unit(value, unit,  &i ) < 0) {
        usart_printf("error converting current and unit\n");
        return;
      }
      usart_printf("current %gV\n", i);
      irange_t iset_range;
      float iset;
      if(irange_and_iset_from_current(i, &iset_range, &iset) < 0) {
        usart_printf("error converting current to range and iset\n");
        return;
      }
      usart_printf("irange %s, iset %gV\n", range_current_string(iset_range), iset);
      core_set( app, app->vset, iset, app->vset_range, iset_range);
    }
    else {

      usart_printf("unrecognized parameter '%s'\n", param);
    }

  } else {

      usart_printf("unrecognized command '%s'   tokens=%d, cmd='%s' param='%s'\n", s, n, cmd, param);
  }
}




static void process_ch(app_t *app, const char ch )
{

  // usart_printf("char code %d\n", ch );

  // change the actual current range
  if(ch == 'u' || ch == 'i') {

      irange_t new_irange = range_current_next( app->iset_range, ch == 'u' );
      if(new_irange != app->iset_range) {
        usart_printf("change iset_range %s\n", range_current_string(new_irange) );
        app->iset_range = app->irange = new_irange;
        range_current_set(app, new_irange);
        dac_current_set(app, fabs(app->iset));
       //  core_set( app, app->vset, app->iset, app->vset_range, new_irange );
      }
  }
  // for voltage
  else if(ch == 'j' || ch == 'k') {

    vrange_t new_vrange = range_voltage_next( app->vset_range, ch == 'j' );
    if(new_vrange != app->vset_range) {
      usart_printf("change vset_range %s\n", range_voltage_string(new_vrange ) );
      app->vset_range = app->vrange = new_vrange;
      range_voltage_set(app, new_vrange);
      dac_voltage_set(app, fabs(app->vset));
      // core_set( app, app->vset, app->iset, new_vrange, app->iset_range );
    }
  }
  // toggle output... on/off. must only process char once. avoid relay oscillate
  else if( ch == 'o') {
    usart_printf("output %s\n", (!app->output) ? "on" : "off" );
    mux_io(app->spi);
    output_set(app, app->irange, !app->output);
    // cBufPut(console_out, '\n');
  }
  // toggle printing of adc values.
  else if( ch == 'p') {
    usart_printf("printing %s\n", (!app->print_adc_values) ? "on" : "off" );
    app->print_adc_values = ! app->print_adc_values;
    // cBufPut(console_out, '\n');
  }
  // halt
  else if(ch == 'h') {
    usart_printf("halt \n");
    state_change(app, HALT);
    return;
  }
  // restart
  else if(ch == 'r') {
    usart_printf("restart\n"); // not resume
    state_change(app, FIRST);
    return;
  }


}



static void update_console_cmd(app_t *app, CBuf *console_in, CBuf* console_out, CBuf *cmd_in )
{
  /*
    TODO
    put these buffers. in the app structure.
    Actually. no. it's neater that they're not.
  */


  int32_t ch;

  while((ch = cBufPop(console_in)) >= 0) {
    // got a character

    /*
      these are not actually useful UI functions....
    */

    // we're in a command
    if( cBufPeekFirst(cmd_in) == ':') {
      // only push chars if we're in a command that starts with ':'
      // push ch to cmd buffer
      cBufPut(cmd_in, ch);
      // echo the char to console
      cBufPut(console_out, ch);
    }


    // not in a command...  so process ch
    else {

      // start a command
      if(ch == ':') {
        // push ch to cmd buffer
        cBufPut(cmd_in, ch);
        // echo the char to console
        cBufPut(console_out, ch);
      }

      process_ch(app, ch );
    }
  }


  if(cBufPeekLast(cmd_in) == '\r') {

    // usart_printf("got CR\n");

    // we got a carriage return
    static char tmp[1000];
    size_t n = cBufCopy(cmd_in, tmp, sizeof(tmp));
    tmp[n - 1] = 0;   // drop tailing line feed
                      // TODO. cBufCopy should potentially do this...
                      // no. if want non-sentinal terminaed raw bytes. eg. for network code...

    // usart_printf("got command '%s'   %d\n", tmp, n);
    usart_printf("got command '%s'\n", tmp);

    process_cmd(app, tmp);

  }
}










/*
  TODO.
  Move. these into the app structure.
  and mvoe the app structure off the stack.
*/

// should pass the console to routines that need it...
static char buf1[1000];
static char buf2[1000];

static CBuf console_in;
static CBuf console_out;


// should probably be in the app structure...
static char buf3[1000];
static CBuf cmd_in;


static void loop(app_t *app)
{

  // move this into the app var.
  static uint32_t soft_500ms = 0;
  UNUSED(soft_500ms);



  while(true) {

    // EXTREME - could actually call update at any time, in a yield()...
    // so long as we wrap calls with a mechanism to avoid stack reentrancy
    // led_update(); in systick.
    // but better just to flush() cocnsole queues.conin/out


    // usart_input_update();
    usart_output_update();

    // update_console_cmd(spi, &console_in);
    update_console_cmd(app, &console_in, &console_out, &cmd_in);

    update(app);

#if 1
    // 500ms soft timer
    if( system_millis > soft_500ms) {
      soft_500ms = system_millis + 500;
      update_soft_500ms(app);
#endif
    }


  }
}


static void spi1_interupt(void *ctx)
{
  UNUSED(ctx);


  // usart_printf("u");

}





int main(void)
{
  // high speed internal!!!
  // TODO. not using.

  systick_setup(16000);


  // clocks
  rcc_periph_clock_enable(RCC_SYSCFG); // maybe required for external interupts?

  // LED
  rcc_periph_clock_enable(RCC_GPIOE);

  // USART
  rcc_periph_clock_enable(RCC_GPIOA);     // f407
  // rcc_periph_clock_enable(RCC_GPIOB); // F410
  rcc_periph_clock_enable(RCC_USART1);


  // spi / ice40
  rcc_periph_clock_enable(RCC_SPI1);

  //////////////////////
  // setup

  // led
  led_setup();

  // uart/console
  cBufInit(&console_in,  buf1, sizeof(buf1));
  cBufInit(&console_out, buf2, sizeof(buf2));


  cBufInit(&cmd_in, buf3, sizeof(buf3));


  usart_setup_gpio_portA();
  usart_setup(&console_in, &console_out);
  usart_printf_init(&console_out);


  ////////////////
  spi1_port_setup();
  spi1_special_gpio_setup();
  spi1_interupt_gpio_setup( spi1_interupt, NULL);


  ////////////////////


  usart_printf("\n--------\n");
  usart_printf("starting loop\n");

  printf("sizeof bool   %u\n", sizeof(bool));
  printf("sizeof float  %u\n", sizeof(float));
  printf("sizeof double %u\n", sizeof(double));

  usart_flush();
  // usart_printf("size %d\n", sizeof(fbuf) / sizeof(float));

  ////////
  // put this in spi1.h.  i think....
  // uint32_t spi = SPI_ICE40;


  app_t app;
  memset(&app, 0, sizeof(app_t));

  app.spi = SPI_ICE40;
  app.print_adc_values = true;
  app.output = false;


  // app.state = FIRST;
  state_change(&app, FIRST );

  loop(&app);

	for (;;);
	return 0;
}




