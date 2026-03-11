/*
  consider rename this file. perhaps to measurement.
  and move the more cal specific functions to cal. perhaps.

*/
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>     // malloc
#include <string.h>     // memcpy




#include <peripheral/spi-ice40.h>
#include <device/spi-fpga0-reg.h>   // for seq mode


#include <lib2/util.h>      // UNUSED
#include <lib2/format.h>    // format_float


#include <data/cal.h>
#include <data/data.h>
#include <data/range.h>





void data_init(
  data_t    *data,
  spi_t     *spi,

  // note that we have not injected cal here.
  // for a default value.
  // double    *cal_w,
  cal_t     *cal,
  range_t   *ranges,
  unsigned *range_idx
)
{
  // called once at initialization

  // data_t *data = malloc( sizeof(data_t));
  assert(data);
  assert(ranges);
  assert(range_idx);


  memset( data, 0, sizeof( data_t));
  data->magic = DATA_MAGIC;

  // data->line_freq = 50;


  data->spi       = spi;
  data->cal       = cal;
  data->ranges    = ranges;
  data->range_idx = range_idx;


  // default
  data->show_counts = true;
  data->show_reading = true;
}




void data_update( data_t *data )
{
  assert( data);
  assert( data->magic == DATA_MAGIC);

  cal_t *cal = data->cal;
  assert( cal && cal->magic == CAL_MAGIC );
  // double cal_w = *data->cal_w;

  range_t *range = &data->ranges[ *data->range_idx ];
  assert(range);



  char buf[100 + 1];

  // could actually pass this dependency - in the update_call().  since this is only time it is needed
  spi_t *spi = data->spi;

  uint32_t status_          = spi_ice40_reg_read32( spi, REG_STATUS );

   _Static_assert(sizeof(data->status) == sizeof(status_), "bad typedef size");
  memcpy( &data->status, &status_,  sizeof( status_));

  data->clk_count_refmux_pos = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_POS);
  data->clk_count_refmux_neg = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_NEG);
  data->clk_count_sigmux     = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_SIGMUX );

  // useful for bounds - and to correct asymetry
  double ratio = (data->clk_count_refmux_pos >= data->clk_count_refmux_neg)
      ?  (double) data->clk_count_refmux_pos / data->clk_count_refmux_neg
      :  - (double) data->clk_count_refmux_neg / data->clk_count_refmux_pos ;
  UNUSED(ratio);


  if( data->status.first) {

    // printf("\n");
  }


  if(data->show_counts) {
    printf( "first=%u idx=%u seq_n=%u, ", data->status.first, data->status.sample_idx, data->status.sample_seq_n);
    printf( "counts pos %7lu neg %7lu sig %7lu, ", data->clk_count_refmux_pos, data->clk_count_refmux_neg, data->clk_count_sigmux);
    // printf( "ratio %.2f, ", ratio);
  }



  if( data->status.sample_idx == 0) {

    // lo - record counts
    data->clk_count_refmux_pos_lo = data->clk_count_refmux_pos;
    data->clk_count_refmux_neg_lo = data->clk_count_refmux_neg;

    data->valid = false;
  }

  else if ( data->status.sample_idx == 1) {

    // hi
    data->count_sum =
          ((double) data->clk_count_refmux_pos    - (cal->w * data->clk_count_refmux_neg))
        - ((double) data->clk_count_refmux_pos_lo - (cal->w * data->clk_count_refmux_neg_lo));


    if(data->show_counts)
      printf("sum %.2f, ", data->count_sum);

    // normalized count
    data->count_norm = data->count_sum  / data->clk_count_sigmux;

    // calculate reading for current range
    data->reading = range->range_reading( range, cal, data->count_norm);
    data->valid     = true;

    if(data->show_reading) {

      printf( "%s-%s, ", range->name, range->arg );
      printf( "read %s", str_format_float_with_commas(buf, 100, 8, data->reading ));
      printf( "%s, ", range->unit );
      // printf( "%s, ", range ? range->unit : ""  );
    }

  }
  else {

    assert(0);

  }

}







bool data_repl_statement( data_t *data,  const char *cmd)
{
  assert(data);
  assert(data->magic == DATA_MAGIC);

  UNUSED(cmd);


  if(strcmp(cmd, "data counts show") == 0
    || strcmp(cmd, "data count show") == 0)
    data->show_counts = true;

  else if(strcmp(cmd, "data counts unshow") == 0
    || strcmp(cmd, "data count unshow") == 0)
    data->show_counts = false;

  else
    return 0;

  return 1;
}



#if 0

nose seems lower in the morning. 6.50am.
nplc 10.

> first=1 idx=0 seq_n=2, counts pos 1975984 neg 2025255 sig 4000001,
first=0 idx=1 seq_n=2, counts pos 1976354 neg 2025634 sig 4000001, sum 0.23, LO-10, read -0.000,000,82V, (0, 0), mean   -0.000,000,82V, stddev 0.000,000,00V,
first=0 idx=0 seq_n=2, counts pos 1975946 neg 2025216 sig 4000001,
first=0 idx=1 seq_n=2, counts pos 1976352 neg 2025632 sig 4000001, sum 0.13, LO-10, read -0.000,000,46V, (1, 1), mean   -0.000,000,64V, stddev 0.000,000,18V,
first=0 idx=0 seq_n=2, counts pos 1976348 neg 2025628 sig 4000001,
first=0 idx=1 seq_n=2, counts pos 1976354 neg 2025634 sig 4000001, sum 0.15, LO-10, read -0.000,000,52V, (2, 2), mean   -0.000,000,60V, stddev 0.000,000,16V,
first=0 idx=0 seq_n=2, counts pos 1976348 neg 2025628 sig 4000001,
first=0 idx=1 seq_n=2, counts pos 1976316 neg 2025595 sig 4000001, sum 0.20, LO-10, read -0.000,000,70V, (3, 3), mean   -0.000,000,63V, stddev 0.000,000,14V,
first=0 idx=0 seq_n=2, counts pos 1976347 neg 2025627 sig 4000001,
first=0 idx=1 seq_n=2, counts pos 1976316 neg 2025595 sig 4000001, sum 0.22, LO-10, read -0.000,000,79V, (4, 4), mean   -0.000,000,66V, stddev 0.000,000,14V,
first=0 idx=0 seq_n=2, counts pos 1976349 neg 2025629 sig 4000001,
first=0 idx=1 seq_n=2, counts pos 1976318 neg 2025597 sig 4000001, sum 0.22, LO-10, read -0.000,000,79V, (5, 5), mean   -0.000,000,68V, stddev 0.000,000,14V,
first=0 idx=0 seq_n=2, counts pos 1976351 neg 2025631 sig 4000001,
first=0 idx=1 seq_n=2, counts pos 1976354 neg 2025634 sig 4000001, sum 0.07, LO-10, read -0.000,000,26V, (6, 6), mean   -0.000,000,62V, stddev 0.000,000,20V,
first=0 idx=0 seq_n=2, counts pos 1975945 neg 2025215 sig 4000001,
first=0 idx=1 seq_n=2, counts pos 1976315 neg 2025594 sig 4000001, sum 0.23, LO-10, read -0.000,000,82V, (7, 7), mean   -0.000,000,65V, stddev 0.000,000,19V,
first=0 idx=0 seq_n=2, counts pos 1976346 neg 2025626 sig 4000001,
first=0 idx=1 seq_n=2, counts pos 1976353 neg 2025633 sig 4000001, sum 0.17, LO-10, read -0.000,000,61V, (8, 8), mean   -0.000,000,64V, stddev 0.000,000,18V,
first=0 idx=0 seq_n=2, counts pos 1976348 neg 2025628 sig 4000001,
first=0 idx=1 seq_n=2, counts pos 1976354 neg 2025634 sig 4000001, sum 0.15, LO-10, read -0.000,000,52V, (9, 9), mean   -0.000,000,63V, stddev 0.000,000,18V,
first=0 idx=0 seq_n=2, counts pos 1975945 neg 2025215 sig 4000001,
first=0 idx=1 seq_n=2, counts pos 1976353 neg 2025633 sig 4000001, sum 0.18, LO-10, read -0.000,000,64V, (0, 10), mean   -0.000,000,61V, stddev 0.000,000,17V,
first=0 idx=0 seq_n=2, counts pos 1976348 neg 2025628 sig 4000001,
first=0 idx=1 seq_n=2, counts pos 1976317 neg 2025596 sig 4000001, sum 0.22, LO-10, read -0.000,000,79V, (1, 10), mean   -0.000,000,65V, stddev 0.000,000,17V,
#endif


