
/*
  replicate function app_cal(app ) .  from dmm


  can pass the yield function here, down from app,
  but note - the yield should not process adc raw data / counts.

  as a strategy - it is simpler/less state interaction - if this function is written to take over control/and block and manage everything relayed to cal.


*/

#include <stdio.h>
#include <assert.h>

// #include <lib2/util.h>      // UNUSED()



#include <ice40-reg.h>
#include <peripheral/spi-ice40.h>

#include <data/data.h>


// we will need to pass the spi also.

void data_cal( uint32_t spi, data_t *data /* void (*yield)( void * ) */ )
{

  printf("whoot data_cal\n");


  for(unsigned i = 0; i < 5; ++i ) {


    // block on interupt.
    while(! data->adc_measure_valid ) {
      // yield( yield_ctx);
    }
    data->adc_measure_valid = false;


    // embed a 8 bit. counter ini the reg_status and use it for the measure.
    // uint32_t status =            spi_ice40_reg_read32( app->spi, REG_STATUS );

    uint32_t clk_count_mux_reset  = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_RESET);   // time refmux is in reset. useful check. not adc initialization time.
    uint32_t clk_count_mux_neg    = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_NEG);
    uint32_t clk_count_mux_pos    = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_POS);
    uint32_t clk_count_mux_rd     = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_REFMUX_RD);
    uint32_t clk_count_mux_sig    = spi_ice40_reg_read32( spi, REG_ADC_CLK_COUNT_MUX_SIG);

  /*  - OK. it doesn't matter whether aperture is for one more extra clk cycle. or one less.  eg. the clk termination condition.
      instead what matters is that the count is recorded in the same way, as for the reference currents.
      eg. so should should always refer to the returned count value, not the aperture ctrl register.

      uint32_t clk_count_mux_sig = spi_ice40_reg_read32( app->spi, REG_ADC_P_APERTURE );
  */

    printf("counts %6lu %lu %lu %6lu %lu", clk_count_mux_reset, clk_count_mux_neg, clk_count_mux_pos, clk_count_mux_rd, clk_count_mux_sig);



  }

}

