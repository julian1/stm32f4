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

    // TODO consider better names here?
    // data->count_sum
    // data->adjusted_sum
    // or count_norm.  for normalized.
    data->count_norm = data->count_sum  / data->clk_count_sigmux;

/*
    consider no range or no range->cal. function available consider a default.
    except a range is always available here
   //  data->reading   = data->value  * cal->b;
*/

    data->reading = range->range_cal_convert( range, cal, data->count_norm );
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


  if(strcmp(cmd, "data counts show") == 0)
    data->show_counts = true;

  else if(strcmp(cmd, "data counts unshow") == 0)
    data->show_counts = false;

  else
    return 0;

  return 1;
}







