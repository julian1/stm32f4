
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
  const range_t *ranges;


  unsigned    *range_idx;    // current active range
  uint32_t    *line_freq;


  // persist...  for AZ. from last reading
  uint32_t adc_clk_count_refmux_pos_lo;
  uint32_t adc_clk_count_refmux_neg_lo;

  ///////////////////////

  bool show_counts;
  // bool show_sum;
  bool show_reading;
  // bool show_ratio;


} decode_t;





void decode_init(
  decode_t    *decode,
  spi_t     *spi,
  cal_t *   cal,
  const range_t   *ranges,

  unsigned  *range_idx,
  uint32_t  *line_freq
);


bool decode_repl_statement( decode_t *decode,  const char *cmd);

void decode_update_data( decode_t *decode, data_t *data);




