
#pragma once


#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>   // size_t


typedef struct range_t range_t;
typedef struct _mode_t _mode_t;
typedef struct data_t data_t;


typedef struct ranging_t ranging_t;

struct ranging_t
{
  uint32_t      magic;


  // ranging writes the mode
  _mode_t       *mode;


  const         range_t *ranges;        // including cal co-efficients
  const size_t  ranges_sz;              // not sizeof() should be const

  size_t        range_idx;

  /*
    putting this in the mode/ sa structure ... would  simplify arg passing to range_set_mode()
    but we can just as easily create a local structure

    the bigger issue is that this is only applied on range changes.
    so if we store in mode_t. there is no context, to apply the changes to the actual relay state.
    as well as to force a retrigger.

    so it is reasonable here.
    note that the transfer cal routines will clear/reset
  */
  bool          range_10Meg ;           // auto 10Meg.

  bool          ar;                     // autoranging


};






void ranging_init(
  ranging_t     *ranging,
  _mode_t       *mode,
  const range_t *ranges,
  const size_t  ranges_sz
);



const range_t * ranging_range_active_get( const ranging_t *ranging);


void ranging_range_set_by_name( ranging_t *ranging, const char *name, const char *arg);


bool ranging_repl_range( ranging_t *ranging, const char *cmd);



bool ranging_update_data( ranging_t *ranging,  const data_t *data );





