
/*
  decode counts, use cal to calculate reading, and assign range

*/




#include <stdio.h>
#include <assert.h>
// #include <stdlib.h>     // malloc
#include <string.h>     // memcpy




#include <peripheral/spi-ice40.h>
#include <device/spi-fpga0-reg.h>   // for seq mode


#include <lib2/util.h>      // UNUSED
#include <lib2/format.h>    // format_float


#include <data/cal.h>
#include <data/data.h>
#include <data/decode.h>
#include <data/range.h>





void decode_init(

  decode_t  *decode,
  spi_t     *spi,
  cal_t     *cal,
  range_t   *ranges,
  unsigned  *range_idx// ,
  // data_t *  data
)
{
  // called once at initialization

  // decode_t *decode = malloc( sizeof(decode_t));
  assert( decode);
  assert( ranges);
  assert( range_idx);

  memset( decode, 0, sizeof( decode_t));
  decode->magic = DECODE_MAGIC;

  // decode->line_freq = 50;


  decode->spi       = spi;
  decode->cal       = cal;
  decode->ranges    = ranges;
  decode->range_idx = range_idx;

  // decode->data      = data;

  // default
  decode->show_counts = true;
  decode->show_reading = true;
}



/*
  EXTR.
  can pass data_t to the update() function.
  rather than static injecting on construction.
*/

void decode_update( decode_t *decode,  data_t *data)
{
  assert( decode);
  assert( decode->magic == DECODE_MAGIC);

  cal_t *cal = decode->cal;
  assert( cal && cal->magic == CAL_MAGIC );

  spi_t *spi = decode->spi;


  assert( data && data->magic == DATA_MAGIC );



  char buf[100 + 1];


  uint32_t status_          = spi_ice40_reg_read32( spi, REG_STATUS );

   _Static_assert(sizeof(data->status) == sizeof(status_), "bad typedef size");
  memcpy( &data->status, &status_,  sizeof( status_));

  // record for current part of reading
  // cal w. needs this data
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


  if(decode->show_counts) {
    printf( "first=%u idx=%u seq_n=%u, ", data->status.first, data->status.sample_idx, data->status.sample_seq_n);
    printf( "counts pos %7lu neg %7lu sig %7lu, ", data->clk_count_refmux_pos, data->clk_count_refmux_neg, data->clk_count_sigmux);
    // printf( "ratio %.2f, ", ratio);
  }



  if( data->status.sample_idx == 0) {

    // lo - record LO counts
    decode->clk_count_refmux_pos_lo = data->clk_count_refmux_pos;
    decode->clk_count_refmux_neg_lo = data->clk_count_refmux_neg;

    data->valid = false;
  }

  else if ( data->status.sample_idx == 1) {

    // hi
    data->count_sum =
          ((double) data->clk_count_refmux_pos      - (cal->w * data->clk_count_refmux_neg))
        - ((double) decode->clk_count_refmux_pos_lo - (cal->w * decode->clk_count_refmux_neg_lo));


    if(decode->show_counts)
      printf("sum %.2f, ", data->count_sum);

    // normalized count
    data->count_sum_norm = data->count_sum  / data->clk_count_sigmux;


    range_t *range = &decode->ranges[ *decode->range_idx ];
    assert( range);
    data->range  = range;


    // calculate reading for current range
    data->reading = range->range_reading( range, cal, data->count_sum_norm);
    data->valid     = true;

    if(decode->show_reading) {

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







bool decode_repl_statement( decode_t *decode,  const char *cmd)
{
  assert( decode);
  assert( decode->magic == DECODE_MAGIC);


  if(strcmp(cmd, "decode show") == 0
    || strcmp(cmd, "decode show") == 0)
    decode->show_counts = true;

  else if(strcmp(cmd, "decode unshow") == 0
    || strcmp(cmd, "decode unshow") == 0)
    decode->show_counts = false;

  else
    return 0;

  return 1;
}



#if 0

mar 12. 2026. move this into a test. sampling lo 10.

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


