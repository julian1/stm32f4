

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <test/support.h>


#include <peripheral/spi-ice40.h>






void spi_read_registers( spi_t *spi, data_t *data)
{

  uint32_t status_              = spi_ice40_reg_read32( spi, REG_STATUS );

   _Static_assert(sizeof(data->status) == sizeof(status_), "bad typedef size");
  memcpy( &data->status, &status_,  sizeof( status_));


  data->clk_count_refmux_pos = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_POS);
  data->clk_count_refmux_neg = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_NEG);
  data->clk_count_sigmux     = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_SIGMUX );

      // must be explicit.  because w is not set
  data->clk_count_aperture   = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_APERTURE);


}



void print_data( data_t * data)
{
  assert( data);


  // printf( "first=%u idx=%u seq_n=%u, ", data->status.first, data->status.sample_idx, data->status.sample_seq_n);

  reg_sr_t  status = data->status;

  printf( "{first=%u idx=%u seq_n=%u}, ",
    status.sample.first,
    status.sample.idx,
    status.sample.seq_n
  );


  printf( "counts pos %7lu neg %7lu sig %7lu, ", data->clk_count_refmux_pos, data->clk_count_refmux_neg, data->clk_count_sigmux);
}


