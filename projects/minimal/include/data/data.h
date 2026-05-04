
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
  consider.  that data_t  should record everything needed for forther processing / display, after decode.
  it is almost like a command.

  - eg. do not want display_vfd  needing to reference the mode. to determine/figure out the 10Meg. setting.
  - instead 10Meg. should be stamped. in data_t.
  - likewise line_freq. etc.

*/

typedef struct data_t
{

  uint32_t        magic;

  // uint32_t     timestamp;


  // set the environmental fields first.

  const range_t   *range;

  const cal_t     *cal;

  uint32_t        line_freq;


  // TODO move. below status.
  bool            is_hi;


  ////////////////

  reg_sr_t status;

  /*
    data_t is more high-level structure
    only reason to record the counts here is to support cal w
  */
  uint32_t adc_clk_count_refmux_pos;
  uint32_t adc_clk_count_refmux_neg;
  uint32_t adc_clk_count_sigmux;        // also needed to report nplc in ui.

  double  clk_count_ratio;


  // AZ HI-LO clk count sum, weight adjusted.
  double  count_sum;

  // count normalized by aperture/sigmux
  double count_sum_norm;

  // reading valid for this conversion/iteration
  bool   reading_valid;

  // reading adjusted by cal
  double  reading;
  // bool reading_valid or use NaN

  /*
    move range_t, line_freq  to the top of this structure
    if we decide to stamp. in app_update().
  */



} data_t;



void data_init( data_t *data );

