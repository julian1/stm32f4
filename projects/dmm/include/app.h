

#pragma once

#include "cbuffer.h"
#include "cstring.h"
#include "fbuffer.h"



// define here? or in mux.h.
// #define SPI_ICE40       SPI1



/*
  AHHH. is clever
    74hc125.
    if fpga gpio comes up high - OE - is disabled. so out is low.
    if fpga gpio comes up lo   - buffer is disabled so out low.

    there seems to be a 50-100uS spike though on creset.
    hopefully too short a time to turn on opto coupler.
*/




// TODO - must rename - very confusing.  state_powerup_t  or similar.
typedef enum state_t {
/*  enum should change to lower case. But do later.
*/
  STATE_FIRST,    // change name initial
  STATE_DIGITAL_UP,
  STATE_ANALOG_UP,
  STATE_HALT
} state_t;




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
  vrange_100V,

  // vrange_none,    // initial condition on start...


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

  irange_10nA = 3,
  irange_100nA,
  irange_1uA,

  irange_10uA ,
  irange_100uA,
  irange_1mA,
  irange_10mA,
  irange_100mA,
  irange_1A,
  irange_10A,

  // irange_none

} irange_t;




typedef struct app_t
{

  /*
    JA.
    Not sure these buffers need to be exposed here. nothing else should touch them.
    Perhaps just put in main stack..
    Getting the led, and uart are low level
  */

  CBuf console_in;
  CBuf console_out;



  ////
  // CBuf      cmd_in;
  CString     command;





  uint32_t spi;


  // we don't/shouldn't even need  to have the current state recorded here.
  // should not have more than one authoritative source on 4094 state.

  // uint8_t state_4094[ 5 ];  // how many
  // uint8_t state_4094[ 3 ];  // how many

#if 0
  uint32_t   u304;
  uint32_t   u514;



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
  uint32_t  adc_drdy_missed;
  uint32_t  adc_ov_count;

  // adc last read values
  // float     vfb;
  // float     ifb;

  /////////////
  uint32_t  measure_millis_last;

  uint32_t  nplc_measure;
  uint32_t  nplc_range;


  FBuf      vfb_measure;
  FBuf      vfb_range;

  FBuf      ifb_measure;
  FBuf      ifb_range;




  // led blink off/on.

  //////////////

  int       digits;

#endif

} app_t;




void state_change(app_t *app, state_t state );

void app_start( app_t * app );
void app_start2( app_t * app );

void app_goto_fail_state( app_t * app );

