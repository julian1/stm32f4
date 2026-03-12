
/*
  low-level data update

  - instead of injecting the controllers into one another
  - add the shared context model, into multiple controllers

  -  better than polymorphic update() or handler function
    note that the range information can be injectected separately also.

*/

#pragma once


#include <stdint.h>
#include <stdbool.h>

#include <device/spi-fpga0-reg.h>     // for reg_sr_t



typedef struct range_t range_t;


#define DATA_MAGIC 997878123



typedef struct data_t
{

  uint32_t  magic;

  reg_sr_t status;

  // need to support cal w
  uint32_t clk_count_refmux_pos;
  uint32_t clk_count_refmux_neg;
  uint32_t clk_count_sigmux;


  // AZ HI-LO clk count sum, weight adjusted.
  double  count_sum;

  // normalized by aperture/sigmux
  double count_sum_norm;

  // whether valid for specific update
  bool   valid;   // consider rename count_sum_norm_valid

  // reading adjusted by cal
  double  reading;
  // bool reading_valid

  // record range used
  range_t   *range;

} data_t;



void data_init( data_t *data );

