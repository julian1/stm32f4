
/*
  low-level data update

*/

#pragma once


#include <stdbool.h>

#include <device/spi-fpga0-reg.h>     // for reg_sr_t



typedef struct cal_t cal_t;
typedef struct spi_t spi_t;
typedef struct range_t range_t;


// consider move to data.c
#define DATA_MAGIC 123


typedef struct data_t
{
  uint32_t    magic;

  spi_t       *spi ;


  // double      *cal_w;

  cal_t      *cal;

  range_t     *ranges;
  unsigned    *range_idx;    // current active range


  //////////////////////////////////
  // first reading

  reg_sr_t status;
  uint32_t clk_count_refmux_pos ;
  uint32_t clk_count_refmux_neg;
  uint32_t clk_count_sigmux;


  ////////////////////////////////


  // persist...  for AZ. from last reading
  uint32_t clk_count_refmux_pos_lo;
  uint32_t clk_count_refmux_neg_lo;

  ///////////////////////

  // AZ HI-LO clk count sum, weight adjusted.
  double  count_sum;

  // scaled by sigmux
  double count_norm;

  // computed using range
  bool   valid;   // rename value_valid

  // reading scaled and offset
  double reading;

  bool show_counts;
  // bool show_sum;
  bool show_reading;
  // bool show_ratio;


} data_t;





void data_init(
  data_t    *data,
  spi_t     *spi,
  // double    *cal_w,
  cal_t *   cal,
  range_t   *ranges,
  unsigned  *range_idx
);


bool data_repl_statement( data_t *data,  const char *cmd );

void data_update( data_t *data );




