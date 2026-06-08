

#pragma once

#include <stdbool.h>
#include <stdint.h>


typedef struct data_t data_t ;


#include <device/spi-fpga0-reg.h>   // for term_t


/*
  sample/sequence acquisition
  control state
  conversion terms.

  TODO consider rename to sa_t
  it is put in mode.
*/

typedef struct sa_state_t
{
  // just about need a magic.

  uint32_t p_trig_delay;

  uint32_t p_precharge;


  /*
    driving state.
    used to compile/build the low-level conversion terms, and decode funcs.

  */
  char    input[ 10];   // "0", "ch1", "ch2", "ratio"  etc.
  bool    noaz;
  // bool oob;       // no_oob.

  // aggregate counts
  // consider rename p_aggregate to indicatea parameter
  unsigned  aggregate;



  // conversion terms. consider name - terms, elts, phases
  term_t  terms[ 4];


  /*
    not strict board state.

    keeping the decode strategy associated with the conversion terms
    and localizing in one place, makes things simpler.
    but consider move to app_t.
  */

  void (*decode_normal)( void *ctx, data_t *data);    // NOAZ, AZ, AGGREGATING, or RATIO...
  void *ctx_normal;
  void *ctx_second;     // uses decode_normal also.

  // OOB. which is always hi first.
  void (*decode_oob)( void *ctx, data_t *data);       // use when havve .oob flag.
  void *ctx_oob;

  unsigned verbose;

} sa_state_t;




void sa_set_input( sa_state_t *sa, const char *s);

void sa_decode_reading( const sa_state_t *sa, data_t *data );

void sa_trig_delay_set( sa_state_t *sa, uint32_t u);

bool sa_repl_statement( sa_state_t *sa, const char  *cmd /*, const environment_t *environment */);



