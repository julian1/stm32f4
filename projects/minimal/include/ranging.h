
#pragma once


#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>   // size_t


typedef struct range_t range_t;
typedef struct _mode_t _mode_t;
typedef struct data_t data_t;



typedef struct ranging_t
{
  uint32_t      magic;


  // ranging writes the mode
  _mode_t       *mode;


  const         range_t *ranges;      // including cal co-efficients
  const size_t  ranges_sz;                  // not sizeof() should be const

  size_t        range_idx;


  bool          range_10Meg ;

  bool          ar;                     // autoranging

  bool          retrigger;              // used to communicate out from ranging() functions ....
                                        // not very good.

} ranging_t;






void ranging_init(
  ranging_t     *ranging,
  _mode_t       *mode,
  const range_t *ranges,      // including cal co-efficients
  const size_t  ranges_sz                  // not sizeof() should be const
);



const range_t * ranging_range_active_get( ranging_t *ranging);


void ranging_range_set_by_name( ranging_t *ranging, const char *name, const char *arg);


bool ranging_repl_range( ranging_t *ranging, const char *cmd);




/*
  only need reg_sr_t from data..
  but do not want to include <device/spi-fpga0-reg.h>  here
  so use data_t
*/

void ranging_update_data( ranging_t *ranging,  const data_t *data );

