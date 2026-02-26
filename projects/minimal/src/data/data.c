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


#include <util.h>     // aper_n_to_period
#include <lib2/util.h>      // ARRAY_SIZE
#include <lib2/format.h>  // format_float


#include <data/cal.h>
#include <data/ranges.h>
#include <data/data.h>




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

  uint32_t status_ = spi_ice40_reg_read32( spi, REG_STATUS );
  reg_sr_t  status;
   _Static_assert(sizeof(status) == sizeof(status_), "bad typedef size");
  memcpy( &status, &status_,  sizeof( status_));

  uint32_t clk_count_refmux_pos = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_POS);
  uint32_t clk_count_refmux_neg = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_NEG);
  uint32_t clk_count_sigmux     = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_SIGMUX );


  data->first =  status.first;
  if(data->first ) {
    printf("\n");
  }

  printf( "first=%u  idx=%u seq_n=%u, ", status.first, status.sample_idx, status.sample_seq_n);
  printf( "counts pos %7lu neg %7lu, sig %7lu, ", clk_count_refmux_pos, clk_count_refmux_neg, clk_count_sigmux);



  if(status.sample_idx == 0) {

    // lo - record counts
    data->clk_count_refmux_pos_lo = clk_count_refmux_pos;
    data->clk_count_refmux_neg_lo = clk_count_refmux_neg;

    data->valid = false;
  }

  else if (status.sample_idx == 1) {

    // hi
    double v = ((double) clk_count_refmux_pos          - (cal_w * clk_count_refmux_neg))
            - ( (double) data->clk_count_refmux_pos_lo - (cal_w * data->clk_count_refmux_neg_lo));

    printf("v %f, ", v );

    data->value = (v / clk_count_sigmux ) * range->b + range->a;
    data->valid = true;

    printf( "v2 %s, ", str_format_float_with_commas(buf, 100, 8, data->value));

    printf( "%s %s, ", range->name, range->unit );

  }
  else
    assert(0);


}







bool data_repl_statement( data_t *data,  const char *cmd )
{
  assert(data);
  assert(data->magic == DATA_MAGIC);


  // could be called, 'buffer show stats', 'buffer show extra' etc.

  if(strcmp(cmd, "data show counts") == 0)
    data->show_counts = 1;

  else if(strcmp(cmd, "data show extra") == 0)
    data->show_extra = 1;

  else if(strcmp(cmd, "data show stats") == 0)
    data->show_stats = 1;

#if 0
  else if(strcmp(cmd, "data cal show") == 0) {

    data_cal_show( data );

  }
#endif

  else
    return 0;

  return 1;
}







