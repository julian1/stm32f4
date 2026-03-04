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


#include <data/data.h>
#include <data/range.h>




void data_init(
  data_t    *data,
  spi_t     *spi,
  double    *cal_w,
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


  // data->cal = cal;
  data->spi = spi;

  data->cal_w     = cal_w;
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

  assert( data->cal_w);
  double cal_w = *data->cal_w;

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


  if( data->status.first) {

    // printf("\n");
  }


  if(data->show_counts) {
    printf( "first=%u idx=%u seq_n=%u, ", data->status.first, data->status.sample_idx, data->status.sample_seq_n);
    printf( "counts pos %7lu neg %7lu sig %7lu, ", data->clk_count_refmux_pos, data->clk_count_refmux_neg, data->clk_count_sigmux);
  }



  if( data->status.sample_idx == 0) {

    // lo - record counts
    data->clk_count_refmux_pos_lo = data->clk_count_refmux_pos;
    data->clk_count_refmux_neg_lo = data->clk_count_refmux_neg;

    data->valid = false;
  }

  else if ( data->status.sample_idx == 1) {

    // hi
    data->clk_count_sum =
          ((double) data->clk_count_refmux_pos    - (cal_w * data->clk_count_refmux_neg))
        - ((double) data->clk_count_refmux_pos_lo - (cal_w * data->clk_count_refmux_neg_lo));


    if(data->show_counts)
      printf("sum %.2f, ", data->clk_count_sum);

    data->value     = data->clk_count_sum  / data->clk_count_sigmux;
    data->reading   = data->value  * range->b + range->a;
    data->valid     = true;

    if(data->show_reading) {

      printf( "%s, ", range->name );
      printf( "read %s", str_format_float_with_commas(buf, 100, 8, data->reading ));
      printf( "%s, ", range->unit );
    }

  }
  else
    assert(0);


}







bool data_repl_statement( data_t *data,  const char *cmd)
{
  assert(data);
  assert(data->magic == DATA_MAGIC);

  UNUSED(cmd);


  if(strcmp(cmd, "data counts show") == 0)
    data->show_counts = true;

  else if(strcmp(cmd, "data counts unshow") == 0)
    data->show_counts = false;

  else
    return 0;

  return 1;
}







