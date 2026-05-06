
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


  bool          range_10Meg ;           // auto 10Meg.

  bool          ar;                     // autoranging

  // bool          retrigger;              // for manual rangning functions ....
                                        // not very good.
                                        // EXTR. consider using a mode flag.
                                        // instead as a shared state to communicate need to update.

#if 0
  // strategy
  bool (*ranging_update_data)( ranging_t *ranging,  const data_t *data );
#endif

};






void ranging_init(
  ranging_t     *ranging,
  _mode_t       *mode,
  const range_t *ranges,
  const size_t  ranges_sz
);



const range_t * ranging_range_active_get( ranging_t *ranging);


void ranging_range_set_by_name( ranging_t *ranging, const char *name, const char *arg);


bool ranging_repl_range( ranging_t *ranging, const char *cmd);



bool ranging_update_data( ranging_t *ranging,  const data_t *data );





