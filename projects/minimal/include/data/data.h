
/*
  low-level data update

*/

#pragma once


#include <stdbool.h>


typedef struct cal_t cal_t;
typedef struct spi_t spi_t;


// consider move to data.c
#define DATA_MAGIC 123


typedef struct data_t
{
  uint32_t magic;

  cal_t *cal;
  spi_t *spi ;

  // first reading
  bool  first;


  // persist...  for AZ. from last reading
  uint32_t clk_count_refmux_pos_lo;
  uint32_t clk_count_refmux_neg_lo;

  ///////////////////////

  // for other modules
  bool   valid;
  // reading
  double value;

  bool show_counts;
  bool show_stats;
  bool show_extra;


} data_t;




// data_t * data_create( cal_t * cal, spi_t *spi);
// reset or init...
void data_reset( data_t *, cal_t * cal, spi_t *spi);


bool data_repl_statement( data_t *data,  const char *cmd );

void data_update( data_t *data );




