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

      - done - separate the float and char circular buffers - to separate files.

      - done - want to count the number of adc interupts/s.

      - done - adc want to display the clk registers in the register output.
          want bitmask and offset to read and write registers.


      - done - get the stddev of vfb. etc. on raw values? 1-10.5V. for test of adc.
      - done - should report v and i also... it's simple to do.

      - done - terminal char - to clear screen. for better logging.

      -       FILE *fopencookie(void *cookie, const char *mode,
                         cookie_io_functions_t io_funcs);
              Nice. allows a structure of custom functions. to impement.
              cookie is just the context... that then gets past.

      - done - rather than report vfb ifb every 500ms etc.
          - should report it as a multiple of the adc interupt count. eg. NPLC.
          - NPLC=1/50th.  setting for 50=1s, 10=1/5th s, 100=2s etc.

      - done - want a adc value buffer.  for stddev. etc.


      - maybe. change so cmd input buffer. maybe need ':' char. just test if non empty (meaning not consumed by single char action).
        and then treat as a command.


      - print the rails voltages as well. maybe.

      - ncurses?
          https://github.com/infinnovation-dev/incurses
          https://www.youtube.com/watch?v=g7Woz3YVgvQ

          https://github.com/ChrisMicro/mcurses

      - range switching...

      - it ought to be possible to cal both voltage, and current (using the 10M and 10G voltage ranges).

      - should be trying to restrict the scope of some functions.
          - eg. struct core. rather than struct app. even if put output bool inside core. etc.
          - or have to inject a few vars.
          - likewise adc functionality.


      - should populate and test the single-ended gain stage.
          good to be complete - and see in practice.  even if offset voltage is not adjusted.
          could  probably even bodge a dac output - to the amp03 ref .
          no. because want resistor divider/ and op-amp for more prec than the dac.
          actually could try without.


        - also want to check that we service them all. unbalanced.
        - also want freq control over adc.  so can do interval of 50/60ms.
      - also count /s the number of times update is called. and we sample. the rails monitors.

    - smartsmu. quicksmu.
      good names.

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


#include "assert.h"
#include "cbuffer.h"
#include "fbuffer.h"
#include "usart2.h"
#include "util.h"


#include "spi1.h"
#include "mcp3208.h"
#include "w25.h"
#include "dac8734.h"
#include "ads131a04.h"

#include "format_float.h"
#include "stats.h"

#include "core.h"   // some of the above files include core.h. need header guards.

#define UNUSED(x) (void)(x)

// here? or in mux.h.
// #define SPI_ICE40       SPI1





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

  CBuf console_in;
  CBuf console_out;


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


  /////////////////////////
  uint32_t  update_count;

  float     lp15v;
  float     ln15v;


  /////////////////////////
  // adc data ready, given by interupt
  bool      adc_drdy;
  uint32_t  adc_drdy_count;
  uint32_t  adc_ov_count;

  // adc last read values
  // float     vfb;
  // float     ifb;

  /////////////
  uint32_t  adc_nplc_measure;
  uint32_t  adc_nplc_range;


  FBuf      vfb_measure;
  FBuf      vfb_range;

  FBuf      ifb_measure;
  FBuf      ifb_range;



  ////
  CBuf    cmd_in;


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

  ASSERT(0);
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





static void dac_current_set(app_t *app, float i)
{
  // wrapper over raw spi is good ...
  // can record the value
  // dac_vset, dac_iset.

  ASSERT(i >= 0);
  mux_dac(app->spi);
  spi_dac_write_register(app->spi, DAC_VOUT1_REGISTER, voltage_to_dac( i ));
}


static void dac_voltage_set(app_t *app, float v)
{
  // wrapper over raw spi is good ...
  // can record the value
  // dac_vset, dac_iset.

  ASSERT(v >= 0);
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

      // VERY IMPORTANT - ensure high current output relay is on. and low current relay off. before open big fets.
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

          // turn on sense amplifier 1
          io_write(app->spi, REG_ISENSE_MUX,  ~ISENSE_MUX1_CTL);
          // turn on first set of big fets.
          io_write(app->spi, REG_IRANGE_X_SW, IRANGE_X_SW1_CTL);
          // break;

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
              ASSERT(0);
              // critical_error_blink();
              // return;
              break;
          };
          break;

        // 10ohm resistor. for 10V swing.
        case irange_100mA:
          // turn on sense amplifier 2
          io_write(app->spi, REG_ISENSE_MUX,  ~ISENSE_MUX2_CTL);

          // ensure sure the high current relay is on. before switching
          // gain 10x active low
          io_write(app->spi, REG_INA_IFB_SW,  ~INA_IFB_SW2_CTL);
          // turn on 2nd switch fets.
          io_write(app->spi, REG_IRANGE_X_SW, IRANGE_X_SW2_CTL);
          break;

        // 1k resistor. for 10V swing.
        case irange_10mA:
          // turn on sense amplifier 3
          io_write(app->spi, REG_ISENSE_MUX,  ~ISENSE_MUX3_CTL);
          // gain 1x active low
          io_write(app->spi, REG_INA_IFB_SW,  ~INA_IFB_SW1_CTL);
          // turn on 4th switch fets.
          io_write(app->spi, REG_IRANGE_X_SW, IRANGE_X_SW4_CTL);
          break;

        default:
          // cannot be here.
          critical_error_blink();
          return;
      }
      return;


    // x
    case irange_1mA:
    case irange_100uA:
    case irange_10uA:
    // y
    case irange_1uA:


      // turn off all fets used on comx range
      io_write(app->spi, REG_IRANGE_X_SW, 0 );
      // gain 1x active low
      io_write(app->spi, REG_INA_IFB_SW,  ~INA_IFB_SW1_CTL);
      // turn on sense amplifier 3
      io_write(app->spi, REG_ISENSE_MUX,  ~ISENSE_MUX3_CTL);


      switch(app->irange) {

        // 10k resistor. for 10V swing
        case irange_1mA:
        // 100k resistor for 10V swing.
        case irange_100uA:
        // 1M resistor for 10V swing.
        case irange_10uA:
          // turn on current range relay y
          io_write(app->spi, REG_RELAY_COM,  RELAY_COM_Y);
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
              ASSERT(0);
              // critical_error_blink();
              // return;
          }
          break;

        // 10M for 10V swing.
        case irange_1uA:
          // IMPORTANT DONT forget to add star jumper to star gnd!!!.
          // turn on current range relay Z
          io_write(app->spi, REG_RELAY_COM,  RELAY_COM_Z);

          // turn on jfet 1
          io_write(app->spi, REG_IRANGE_YZ_SW, IRANGE_YZ_SW1_CTL);
          break;


        default:
          ASSERT(0);

      }

      // wait until have settled at lower current before turn off big relay, and on the reed relay. if coming from high-current range.
      // eg. should be done last.
      output_set(app, app->irange, app->output);
      msleep(1);

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

    // case irange_1uA:    return 1e-7f;
    case irange_1uA:    return 0.0000001f;

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


/*
  We need to change this. so that it can correct for bad logic -
    whereby we end up on a wrong range.

  the only way to do that is to store the current dac value.
*/

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
      usart_printf("v is %f\n", v);
      usart_printf("ZOOM in voltage\n");
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
      usart_printf("v is %f\n", v);
      usart_printf("ZOOM out voltage\n");
      range_voltage_set(app, higher);
      changed = true;
    }
  }
  else if( fabs(v) > 10.5) {

    // we are not zoomed in... but there is an excursion...... on opposite polarity....
    // eg. cannot regulate on this claamp.
    // eg. set -5V, +3mA   on resistor.
    usart_printf("WHOOT bad state here.\n");

    // switching off? probably won't help...   but we can invalidate.
    // but can switch off. and stop.

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






static void print_current(irange_t irange, float val)
{
  /*
    improtant.
      formatting measured values, according to selected range (rather than value) is correct.
      enourage drill to a higher range.
  */

  char buf[100];


  // usart_printf(" here " );

  switch( irange)
  {
    // not sure whether we should care about this...

    case irange_1uA:
      // when power is off... kind of nice to report...
      if(fabs(val) * 1e9f > 1)
        // usart_printf("%fnA", val * 1e9f);
        usart_printf("%snA", format_float(buf, ARRAY_SIZE(buf), val * 1e+9f, 6) ); // 6 digits
      else
        // this will be more valid, with a higher valued resistor 100M or 1G.
        // usart_printf("%fpA", val * 1e12f);
        usart_printf("%spA", format_float(buf, ARRAY_SIZE(buf), val * 1e+12f, 6) ); // 6 digits
      break;


    case irange_10uA:
    case irange_100uA:
    case irange_1mA:
      // usart_printf("%fuA", val * 1000000.f);
      usart_printf("%suA", format_float(buf, ARRAY_SIZE(buf), val * 1e+6f, 6) ); // 6 digits
      break;

    case irange_10mA:
    case irange_100mA:
    case irange_1A:
      // usart_printf("%fmA", val * 1000.f);   // TODO 0.7A better as 0.7A. 0.6A better as 600mA. think..
      usart_printf("%smA", format_float(buf, ARRAY_SIZE(buf), val * 1e+3f, 6) ); // 6 digits
      break;


    // case irange_1A:
    case irange_10A:
      // usart_printf("%fA", val);
      usart_printf("%sA", format_float(buf, ARRAY_SIZE(buf), val, 6) ); // 6 digits
      break;


  }
}


static void print_voltage(vrange_t vrange, float val)
{
  char buf[100];

  // ie expressed on 10V range
  switch(vrange)
  {
    case vrange_100V:
    case vrange_10V:
      // usart_printf("%fV", val);
      usart_printf("%sV", format_float(buf, ARRAY_SIZE(buf), val, 6) ); // 6 digits
      break;

    case vrange_1V:
    case vrange_100mV:
      // usart_printf("%fmV", val * 1000.f);
      // TODO 1e+3f
      usart_printf("%smV", format_float(buf, ARRAY_SIZE(buf), val * 1000, 6) ); // 6 digits
      break;

  }
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

  // rembmer inverse
  uint32_t minmax = v ? CLAMP2_MAX : CLAMP2_MIN;

  io_write(app->spi, REG_CLAMP2, ~( minmax ) );     // min of current or voltage
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

  usart_printf("---------------\n");
  usart_printf("core set\n");

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
  // interupt context. avoid work here...

  ++app->adc_drdy_count;

  // set update to read...
  app->adc_drdy = true;
}



static void update_soft_1s(app_t *app)
{
  // maybe review this...
  UNUSED(app);

  // reset the housekeeping counts
  // IMPORTANT keep. we can report this stuff easily on 1s interval - if use ncurses.
  app->adc_drdy_count  = 0;
  app->update_count    = 0;

  // TODO should be checking adc_read count matches what we expect, on 1s interval.
  // but we have removed this var...

}





static void update_soft_500ms(app_t *app)
{
  UNUSED(app);

  mux_io(app->spi);
  io_toggle(app->spi, REG_LED, LED1);
}



static void update_nplc_measure(app_t *app)
{

  ASSERT(app->state ==  ANALOG_UP);

  ASSERT(fBufCount(&app->vfb_measure) == app->adc_nplc_measure);
  ASSERT(fBufCount(&app->vfb_measure) > 0);
  ASSERT(fBufCount(&app->ifb_measure) == fBufCount(&app->vfb_measure));

  // normal operation
  if(app->print_adc_values) {

    // works in screen and picocom
    // https://stackoverflow.com/questions/60293014/how-to-clear-screen-of-minicom-terminal-using-serial-uart
    // usart_printf("%c%c%c%c",0x1B,0x5B,0x32,0x4A);
    // https://electronics.stackexchange.com/questions/8874/is-it-possible-to-send-characters-through-serial-to-go-up-a-line-on-the-console
#if 1
    usart_printf("\033[2J");    // clear screen
    usart_printf("\033[0;0H");  // cursor to top left
    // usart_printf("\033[10B");   // move cursor down 10 lines ... works
    // usart_printf("\033[3B");   // move cursor down 3 lines ... works
#endif


    // position cursor top left?

    // when we set the range. we should set the default format.
    // the format prec wants to be able to user modified.
    /////////////////

    // most recent measurements
    float vfb = fBufPeekLast(&app->vfb_measure);
    float ifb = fBufPeekLast(&app->ifb_measure);

    usart_printf("source measure unit\n");
    usart_printf("\n");

    usart_printf("vfb ");
    print_voltage(app->vrange, vfb * range_voltage_multiplier(app->vrange)  );

    usart_printf("\t");
    usart_printf("vset ");
    print_voltage(app->vset_range, app->vset * range_voltage_multiplier(app->vset_range));

    usart_printf("\t");
    usart_printf("vset_range: %s",  range_voltage_string(app->vset_range));
    usart_printf("\t");
    usart_printf("vrange: %s",      range_voltage_string(app->vrange)); // measure/active range

    if(app->vrange == app->vset_range) {
      usart_printf("*");
    }


    /////////////////
    // to format this stuff...  actually add a fixed space for '-' and 'V' instead of 'mV' etc?
    // no.
    // it would be useful to be have a mark function ....   which returns the line offset.
    // actually ought to be easy...
    // printf char position in line
    // actually just a mark()   would work. if count output chars. from start.

    // or does vt100 or ansi terminal support pos in line?

    usart_printf("\n\n");

    usart_printf("ifb ");
    print_current(app->irange, ifb * range_current_multiplier(app->irange));

    usart_printf("\t");
    usart_printf("iset ");
    print_current(app->iset_range, app->iset * range_current_multiplier(app->iset_range) );

    usart_printf("\t");
    usart_printf("iset_range: %s",  range_current_string(app->iset_range));
    usart_printf("\t");
    usart_printf("irange: %s",      range_current_string(app->irange));   // measure/active range

    if(app->irange == app->iset_range) {
      usart_printf("*");
    }

    usart_printf("\n\n");

  /////////////////////
// stats
    // GOOD...
    float vs[100];
    float is[100];

    size_t vn = fBufCopy(&app->vfb_measure, vs, ARRAY_SIZE(vs));
    ASSERT(vn >= 1);

    size_t in = fBufCopy(&app->ifb_measure, is, ARRAY_SIZE(is));
    ASSERT(in >= 1);

    ASSERT(vn == in);


    float vmean = mean(vs, vn);
    float vsd =  stddev(vs, vn);
    usart_printf("vfb last %f    vmean %f    vstddev %f\n", vfb, vmean, vsd);

    float imean = mean(is, in);
    float isd =  stddev(is, in);
    usart_printf("ifb last %f    imean %f    istddev %f\n", ifb, imean, isd);


    usart_printf("adc_nplc_measure %d\n",app->adc_nplc_measure);
    usart_printf("adc_nplc_range   %d\n",app->adc_nplc_range);

    usart_printf("adc ov %d\n", app->adc_ov_count);


    // char buf[100];
    // usart_printf("%sV", format_float(buf, ARRAY_SIZE(buf), val, 5) ); // 6 digits
 
    usart_printf("lp15v %f    ln15v %f\n", app->lp15v, app->ln15v);

    usart_printf("output %s\n", (app->output) ? "on" : "off" );

    usart_printf("\n");

    // print the current console input buffer
    // OK... No...

    usart_printf("> ");

    char buf[100];
    cBufCopyString2(&app->cmd_in, buf, ARRAY_SIZE(buf));
    usart_printf("%s", buf);

    // perhaps we can print the current command buffer also....

  }


}



static void update_nplc_range(app_t *app)
{
  ASSERT(app->state ==  ANALOG_UP);

  ASSERT(fBufCount(&app->vfb_range) == app->adc_nplc_range);
  ASSERT(fBufCount(&app->vfb_range) > 0);
  ASSERT(fBufCount(&app->ifb_range) == fBufCount(&app->vfb_range));


  // most recent rangements
  float vfb = fBufPeekLast(&app->vfb_range);
  float ifb = fBufPeekLast(&app->ifb_range);

  // TODO. change back so that can change both together,
  range_current_auto(app, ifb );
  range_voltage_auto(app, vfb);



  fBufClear(&app->vfb_range);
  fBufClear(&app->ifb_range);

}






static void update_adc_drdy(app_t *app)
{

  ASSERT(app->adc_drdy && app->state == ANALOG_UP);


  float ar[4];
  int32_t ret = spi_adc_do_read(app->spi, ar, 4);
  app->adc_drdy = false;

  if(ret < 0) {
    // error
    // usart_printf("adc error\n");
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

  ASSERT(fBufPeekLast(&app->vfb_measure) == vfb);
  ASSERT(fBufPeekLast(&app->ifb_measure) == ifb);
  ASSERT(fBufCount(&app->vfb_measure) ==  fBufCount(&app->ifb_measure));

  fBufPush(&app->vfb_range, vfb );
  fBufPush(&app->ifb_range, ifb);
  ASSERT(fBufPeekLast(&app->vfb_range) == vfb);
  ASSERT(fBufPeekLast(&app->ifb_range) == ifb);
  ASSERT(fBufCount(&app->vfb_range) ==  fBufCount(&app->ifb_range));


  if(ifb > 1.f /*3.f*/ && app->irange == irange_10A) {

    usart_printf("unknown overcurrent condition\n");
    // unknown over-current condition
    // probable hardware condition
    ASSERT(0);
  }

  /*
    think we should record adc measurements twice.
    eg.
      - once for measure, and once for range switching.
      - once for ranging.
    even if choose to use only use/peek for most recent value for range switching.
    don't particularly see why need the circular buffer.  rather than a buffer?
    void push(buf, n, val ) // do range chanch
    push(buf, ARRAY_SIZE(buf), val);
    use two - and we can use the element count for both
  */


  // do measure reporting
  if(fBufCount(&app->vfb_measure) == app->adc_nplc_measure)
  {
    update_nplc_measure(app);
    // should be done where?...
    // change name measure_ov...  should
    app->adc_ov_count = 0;
    ASSERT(fBufCount(&app->vfb_measure) == 0);
    ASSERT(fBufCount(&app->ifb_measure) == 0);
  }

  // do ranging
  if( fBufCount(&app->vfb_range) == app->adc_nplc_range)
  {
    update_nplc_range(app);
    ASSERT(fBufCount(&app->vfb_range) == 0);
    ASSERT(fBufCount(&app->ifb_range) == 0);
  }

}


static void update(app_t *app)
{
  // called as often as possible

  ++app->update_count;

  /*
      main adc, ready to be read
  */
  if(app->adc_drdy && app->state == ANALOG_UP) {

    update_adc_drdy(app);
  }


  /*
    these block... while value is read, and is the main source if this loop being slow. which also doesn't matter.
    could offload spi reading ot the fpga. along with test against threshold values.
  */
  mux_adc03(app->spi);
  app->lp15v = spi_mcp3208_get_data(app->spi, 0) * 0.92 * 10.;
  app->ln15v = spi_mcp3208_get_data(app->spi, 1) * 0.81 * 10.;

  // usart_printf("lp15v %f    ln15v %f\n", lp15v, ln15v);

  switch(app->state) {

    case FIRST:
      state_change(app, DIGITAL_UP);
      break;

    case DIGITAL_UP:
      if(app->lp15v > 15.0 && app->ln15v > 15.0 )
      {
        usart_printf("-----------\n");
        usart_printf("lp15v %f    ln15v %f\n", app->lp15v, app->ln15v);
        usart_printf("15V analog rails ok - state change analog-up\n");
        state_change(app, ANALOG_UP);
      }
      break ;

    case ANALOG_UP:
      if((app->lp15v < 14.7 || app->ln15v < 14.7)  )
      {
        usart_printf("lp15v %f    ln15v %f\n", app->lp15v, app->ln15v);
        usart_printf("15V analog rails low - state change halt\n");
        state_change(app, HALT);
      }
      break;

    case HALT:
      break;


    default:
      ;
  };
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

      // INITIAL
      // core_set( app, -5.f , -5.f );    // -5V compliance, -1mA  sink.
      // core_set( app, 5.f , 3.f, vrange_10V, irange_10mA );         // 5V source, 5mA compliance,
      // core_set( app, 0.5f , 3.f, vrange_10V, irange_10mA );         // oscillates.
      core_set( app, 5.f , 3.f, vrange_10V, irange_10mA );         // 5V source, 5mA compliance,

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

  // ASSERT(i >= 0);
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
  else {
    *irange = irange_1uA;
    *iset = i * 1e+7f;
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
  // ASSERT(v >= 0);
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




static void update_console_ch(app_t *app, const char ch )
{
  // hange name update_console_ch()
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
    // cBufPush(console_out, '\n');
  }
  // toggle printing of adc values.
  else if( ch == 'p') {
    usart_printf("printing %s\n", (!app->print_adc_values) ? "on" : "off" );
    app->print_adc_values = ! app->print_adc_values;
    // cBufPush(console_out, '\n');
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



static void update_console_cmd(app_t *app)
{
  /*
    TODO

    should pass the app structure.
    for app->cmd_in at least

    put these buffers. in the app structure.
    Actually. no. it's neater that they're not.
  */

  ASSERT(&app->cmd_in);


  while( ! cBufisEmpty(&app->console_in)) {

    // got a character
    int32_t ch = cBufPop(&app->console_in);
    ASSERT(ch >= 0);


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

    // usart_printf("got CR\n");

    // we got a carriage return
    static char tmp[1000];

    size_t nn = cBufCount(&app->cmd_in);

    size_t n = cBufCopyString(&app->cmd_in, tmp, ARRAY_SIZE(tmp));
    ASSERT(n <= sizeof(tmp));
    ASSERT(tmp[n - 1] == 0);

    ASSERT( nn == n - 1);

    // TODO first char 'g' gets omitted/chopped here, why? CR handling?
    usart_printf("got command '%s'\n", tmp);

    process_cmd(app, tmp);

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

    update(app);


    // update_console_cmd(app, &app->console_in, &app->console_out);
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





/////////////////////////
/*
  TODO.
  Maybe move raw buffers into app structure?
*/

static char buf_console_in[1000];
static char buf_console_out[1000];

static char buf_cmds[1000];




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

  // this is the mcu clock.  not the adc clock. or the fpga clock.
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




  //////////////////////
  // main app setup

  memset(&app, 0, sizeof(app_t));

  app.spi = SPI1 ;
  app.print_adc_values = true;
  app.output = false;

  app.adc_nplc_measure = 50;
  app.adc_nplc_range   = 20;

  // uart/console
  cBufInit(&app.console_in,  buf_console_in, sizeof(buf_console_in));
  cBufInit(&app.console_out, buf_console_out, sizeof(buf_console_out));

  // command buffer
  cBufInit(&app.cmd_in, buf_cmds, sizeof(buf_cmds));

  // vfb buffer
  // fBufInit(&app.vfb_cbuf, buf_vfb, ARRAY_SIZE(buf_vfb));

  // measure
  fBufInit(&app.vfb_measure, buf_vfb_measure, ARRAY_SIZE(buf_vfb_measure));
  fBufInit(&app.ifb_measure, buf_ifb_measure, ARRAY_SIZE(buf_ifb_measure));

  // range
  fBufInit(&app.vfb_range, buf_vfb_range, ARRAY_SIZE(buf_vfb_range));
  fBufInit(&app.ifb_range, buf_ifb_range, ARRAY_SIZE(buf_ifb_range));


/*
static float buf_vfb_measure[100];
static float buf_ifb_measure[100];


static float buf_vfb_range[100];
static float buf_ifb_range[
*/

  // setup buffers
  usart_setup_gpio_portA();
  usart_setup(&app.console_in, &app.console_out);

  // setup print
  usart_printf_init(&app.console_out);


  ////////////////
  spi1_port_setup();
  spi1_special_gpio_setup();
  // adc interupt...
  spi1_interupt_gpio_setup( (void (*) (void *))spi1_interupt, &app);


  ////////////////////


  usart_printf("\n--------\n");
  usart_printf("starting loop\n");

  printf("sizeof bool   %u\n", sizeof(bool));
  printf("sizeof float  %u\n", sizeof(float));
  printf("sizeof double %u\n", sizeof(double));


  state_change(&app, FIRST );

  loop(&app);

	for (;;);
	return 0;
}




