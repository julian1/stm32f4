
/*
  replicate function app_cal(app ) .  from dmm


  can pass the yield function here, down from app,
  but note - the yield should not process adc raw data / counts.

  as a strategy - it is simpler/less state interaction - if this function is written to take over control/and block and manage everything relayed to cal.
  ------

  OK. issue is that cal has to set up the mode. to control ref voltages etc.
  BUT. perhaps we can handle from here?


*/

#include <stdio.h>
#include <assert.h>


#include <ice40-reg.h>
#include <peripheral/spi-ice40.h>


#include <mode.h>
#include <util.h> // nplc_to_aperture()

#include <data/data.h>
#include <data/data.h>


// we will need to pass the spi also.


/*
    could be used in app also.
*/

/* / we need system mills
  HMMMM.  there's quite a bit of data we need.
  where are we going to store the cal?
  in data.

  perhaps line_freq should go in data...
*/


void data_cal( data_t *data , uint32_t spi, _mode_t *mode,  volatile uint32_t *system_millis   /* void (*yield)( void * ) */ )
{
  assert(data);
  assert(data->magic == DATA_MAGIC) ;
  assert(mode);




  printf("whoot cal() \n");



  mode_set_dcv_source( mode, 10 );

  mode->reg_mode = MODE_SA_ADC;       // set mode adc.
  mode->trigger_source_internal = 1;  // turn on adc  // perhapkkkkk

  mode->adc.reg_adc_p_aperture = nplc_to_aperture( 10, data->line_freq );;    // set aperture
 


  printf("spi_mode_transition_state()\n");
  spi_mode_transition_state( spi, mode, system_millis);




  uint32_t model_cols = 3;

  // unsigned row_idx = 0;
  MAT *row = NULL;


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

    printf("\n");

    // consider rename. this is more model_encode_row_from_counts()) - according to the model.
    // taking model as first arg
    row = run_to_matrix( clk_count_mux_neg, clk_count_mux_pos, clk_count_mux_rd, /*app->*/model_cols, row);



  }

}

