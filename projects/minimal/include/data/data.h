
/*
  low-level data update

  - instead of injecting the controllers into one another
  - instead, pass the shared context model, into multiple controllers
    - works the same as tcp packet handling in kernel, or packet in http server.

  -  simpler than a polymorphic update() or handler function
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



typedef struct data_t
{

  uint32_t  magic;

  reg_sr_t status;

  // need to record to support cal w
  uint32_t clk_count_refmux_pos;
  uint32_t clk_count_refmux_neg;
  uint32_t clk_count_sigmux;        // also needed to report nplc in ui.


  // AZ HI-LO clk count sum, weight adjusted.
  double  count_sum;

  // normalized by aperture/sigmux
  double count_sum_norm;

  // whether data valid for this iteration
  bool   valid;   // consider rename count_sum_norm_valid

  // reading adjusted by cal
  double  reading;
  // bool reading_valid or use NaN

  // range used
  range_t   *range;

  // can record cal used.
  cal_t     *cal;

} data_t;



void data_init( data_t *data );

