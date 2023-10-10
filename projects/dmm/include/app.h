

#pragma once

#include "cbuffer.h"
#include "cstring.h"
#include "fbuffer.h"


typedef struct app_t
{

  /*
    keeping all the state out of here. is nice.

    Not sure these buffers need to be exposed here. nothing else should touch them.
    no it's good. general configuration. of mcu state.
  */

  CBuf console_in;
  CBuf console_out;



  ////
  CString     command;


  uint32_t spi;

  uint32_t led_port;
  uint32_t led_out;

  // should probably be volatile. although it's only passed by reference
  uint32_t system_millis;

  ////////
  // not sure what the best way is to handle this state.
  unsigned count;

  bool led_state ;     // should rename, or just use the last bit of the count .



  unsigned test_in_progress; // enum. for test type.

  bool comms_ok;
  // we don't/shouldn't even need  to have the current state recorded here.
  // should not have more than one authoritative source on 4094 state.




} app_t;



typedef struct Mode Mode;


void do_4094_transition( unsigned spi, Mode *mode, uint32_t *system_millis);

uint32_t nplc_to_aper_n( double nplc );
double aper_n_to_nplc( uint32_t aper_n);
double aper_n_to_period( uint32_t aper_n);


bool test15( app_t *app , const char *cmd,  Mode *mode_initial);    // not sure if good to pass here, 






