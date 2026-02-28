
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


  double      *cal_w;
  range_t     *ranges;
  unsigned    *range_idx;    // current active range


  //////////////////////////////////
  // first reading
  // TODO we should just copy the status register here... to get the flags.
  // bool  first;
  reg_sr_t  status;

  uint32_t clk_count_refmux_pos ;
  uint32_t clk_count_refmux_neg;
  uint32_t clk_count_sigmux;


  // reading AZ. value weight adjusted.
  double  clk_count_sum;

  ////////////////////////////////


  // persist...  for AZ. from last reading
  uint32_t clk_count_refmux_pos_lo;
  uint32_t clk_count_refmux_neg_lo;

  ///////////////////////

  // computed using range
  bool   valid;   // rename reading_valid

  // reading
  // reading_value or just reading
  double value;
  // adjusted by range
  double reading;

/*
  // repl control stuff
  bool show_counts;
  bool show_stats;
  bool show_extra;
*/

  bool show_counts;
  bool show_sum;
  bool show_reading;



} data_t;





void data_init(
  data_t    *data,
  spi_t     *spi,
  double    *cal_w,
  range_t   *ranges,
  unsigned  *range_idx
);


bool data_repl_statement( data_t *data,  const char *cmd );

void data_update( data_t *data );




