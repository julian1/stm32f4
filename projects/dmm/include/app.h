

#pragma once

#include "cbuffer.h"
#include "cstring.h"
#include "fbuffer.h"


typedef struct Mode Mode;

typedef struct app_t
{

  /*
    keeping the mode vectors out of here. is nice.

  */

  CBuf console_in;
  CBuf console_out;



  ////
  CString     command;


  uint32_t spi;

  uint32_t led_port;
  uint32_t led_out;

  // updated on interupt. should probably be declared volatile.
  // but functions that use can also declare volatile
  uint32_t system_millis;

  ////////
  // not sure what the best way is to handle this state.
  unsigned count;

  bool led_state ;     // should rename, or just use the last bit of the count .


  /*
    rather than an enum for the test - this could be changed to a dispatch function
    that return contexts back  - to a long running function, that yielded control.
    eg. quite simple and effective.
  */

  // void (*yielded_function)( app_t * ) ;
  unsigned test_in_progress; // enum. for test type.

  bool comms_ok;

  // we don't/shouldn't even need the current 4094/fpga state recorded/duplicated here.
  // not not want more than single authoritative source for 4094 state.

  /*
    mode_inthese are here to aid  access.
    use pointer to keep opaque and easier  

  */
  
  const Mode *mode_initial;      // use pointer  
  Mode *mode_current;      // all inputs turned off.


} app_t;




// better name
void do_4094_transition( unsigned spi, const Mode *mode, uint32_t *system_millis);

// need to pass line-freq as argument also
uint32_t nplc_to_aper_n( double nplc );
double aper_n_to_nplc( uint32_t aper_n);
double aper_n_to_period( uint32_t aper_n);


// TODO make mode_initial const.
bool test15( app_t *app , const char *cmd);    // not sure if good to mode_initial here,
bool test14( app_t *app , const char *cmd);    // not sure if good to pass here,

bool test16( app_t *app , const char *cmd);    // not sure if good to pass here,


bool app_extra_functions( app_t *app , const char *cmd);



