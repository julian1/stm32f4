
/*
  low-level data update

*/

#pragma once


#include <stdbool.h>

#include <device/spi-fpga0-reg.h>     // for reg_sr_t



typedef struct cal_t cal_t;
typedef struct spi_t spi_t;
typedef struct range_t range_t;
typedef struct data_t data_t;


// consider move to data.c
#define DECODE_MAGIC 123




typedef struct decode_t
{
  uint32_t    magic;

  spi_t       *spi ;
  cal_t       *cal;
  range_t     *ranges;
  unsigned    *range_idx;    // current active range

  data_t      *data;


  //////////////////////////////////
  // first reading


  ////////////////////////////////


  // persist...  for AZ. from last reading
  uint32_t clk_count_refmux_pos_lo;
  uint32_t clk_count_refmux_neg_lo;

  ///////////////////////


#if 0
  reg_sr_t status;

  // AZ HI-LO clk count sum, weight adjusted.
  double  count_sum;

  // scaled by sigmux
  double count_norm;

  // computed using range
  bool   valid;   // rename value_valid

  // reading scaled and offset
  double reading;

#endif

  bool show_counts;
  // bool show_sum;
  bool show_reading;
  // bool show_ratio;


} decode_t;





void decode_init(
  decode_t    *decode,
  spi_t     *spi,
  cal_t *   cal,
  range_t   *ranges,
  unsigned  *range_idx// ,
  // data_t *   data
);


bool decode_repl_statement( decode_t *decode,  const char *cmd );

void decode_update( decode_t *decode, data_t *data );




