
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

typedef struct data_t
{

  uint32_t        magic;

  // uint32_t     timestamp;


  // set/stamp environment fields first.
  const range_t   *range;
  const cal_t     *cal;
  uint32_t        line_freq;

  // need 10Meg.


  ////////////////
  // acquisition/adc related fields

  reg_sr_t        status;

  // better name  is_input or not_zero?
  bool            is_hi;

  // better name. to distinguish fast ranging reading

  // OOB_reading. used for ranging .
  // need to compare the sigmux with the mode aperture
  // is_mode_aperture
  bool            is_oob;


  /*
    data_t is more high-level structure,
    but record low-level counts to ease calibration of cal w
  */
  uint32_t adc_clk_count_refmux_pos;
  uint32_t adc_clk_count_refmux_neg;
  uint32_t adc_clk_count_sigmux;        // also needed to report nplc in ui.

  double   adc_clock_count_ratio;

  // clk count sum, with weighting
  double  adc_count_sum;

  // count normalized by aperture/sigmux
  double adc_count_sum_norm;

  // reading valid for this conversion/iteration
  // or should use NaN
  bool   reading_valid;

  // reading adjusted by cal
  double  reading;



} data_t;



void data_init( data_t *data );

