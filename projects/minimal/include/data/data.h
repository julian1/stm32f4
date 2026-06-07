
/*
  low-level data update

  - instead of injecting the controllers into one another
  - instead, pass the shared context model, into multiple controllers
    - works the same as tcp packet handling in kernel, or http/html packet

  - also simpler than a polymorphic update() or handler function
    note that the range information can be injectected separately also.
  --------

  note subsequnt controllers in chain, can test for the most updated field
    in order, to determine which field to use as input.
    either by checking valid flag, or NaN value etc.
*/

#pragma once


#include <stdint.h>
#include <stdbool.h>

#include <device/spi-fpga0-reg.h>     // for reg_sr_t



typedef struct range_t range_t;
typedef struct cal_t cal_t;


#define DATA_MAGIC 997878123


/*
  consider.  that data_t  should record everything for subsequent processing / reporting / display, after decode.
  it is almost like a command.

  - eg. do not want display_vfd  needing to reference the mode. to determine/figure out the 10Meg. setting.
  - instead 10Meg. should be stamped. in data_t.
  - likewise line_freq. etc.

*/


typedef struct data_t data_t;

struct data_t
{

  uint32_t      magic;


  // uint32_t     timestamp;


  // set/stamp environment fields first.
  const range_t *range;

  /*
    TODO.  just use double cal_w/
			decoding does not need the full cal. here.
  */
  // const cal_t   *cal;
  double		cal_w;
  uint32_t      line_freq;

  // need 10Meg.


  ////////////////
  // low-level acquisition/adc related fields


  reg_sr_t      status;

  term_t     term;

  uint32_t adc_refmux_pos;
  uint32_t adc_refmux_neg;
  uint32_t adc_sigmux;        // also needed to report nplc in ui.

  //
  double   ratio_refmux;


  ////////////////
  // decode values... consider drop adc_ prefix
  // these are decoded values, calculated by decode

/*
  EXTR. may want to completely hide. the count_sum.
    then the decoder.

    so the decoder handles sigmux.  and raw count_raw.

*/
  // clk count sum, with ref weighting
  // double  count_sum;

  // count normalized by aperture/sigmux
  double count_sum_norm;


  double count_sum_norm2;

  ///////////////////
  // these are readings. after decode
  // not adc

  // wehter reading valid for this conversion/iteration
  // or use NaN?
  bool   reading_valid;

  // for ranging
  // for amp-out
  // double reading_nominal;


  // reading adjusted by cal, and range
  double  reading;


  /*
    consider - instead of a single reading_valid flag to indicate to the display functions.
    just use reading != NAN.  for the different stages.

    not sure. the reading_valid is very clear. for the code that needs to check this

    reading_nominal, reading_normal, reading_aggregate  etc.
  */

} ;



void data_init( data_t *data );

