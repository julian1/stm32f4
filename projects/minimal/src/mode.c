

#include <assert.h>




#include <spi-ice40.h>
#include <spi-4094.h>

#include <lib2/util.h>   // msleep
#include <mode.h>


void mode_transition_state( unsigned spi, const Mode *mode, volatile uint32_t *system_millis)
{
  // could name spi_mode_transition_state
  assert(mode);
  // assert( sizeof(_4094_state_t) == 10 );

  // should we be passing as a separate argument


  // mux spi to 4094. change mcu spi params, and set spi device to 4094
  spi_mux_4094 ( spi);


  // printf("-----------\n");

  // printf("app_transition_state write first state\n");
  // state_format (  (void *) &mode->first, sizeof(X) );

  // and write device
  spi_4094_reg_write_n(spi, (void *) &mode->first, sizeof( mode->first ) );

  // sleep 10ms
  msleep(10, system_millis);


  // and format
  // printf("app_transition_state write second state\n");
  // state_format ( (void *) &mode->second, sizeof(X) );

  // and write device
  spi_4094_reg_write_n(spi, (void *) &mode->second, sizeof(mode->second) );

  /////////////////////////////

  // now do fpga state
  spi_mux_ice40(spi);

/*
  // set mode
  spi_ice40_reg_write32(spi, REG_MODE, mode->reg_mode );

  spi_ice40_reg_write_n(spi, REG_DIRECT,  &mode->reg_direct,  sizeof( mode->reg_direct) );

  spi_ice40_reg_write32(spi, REG_ADC_P_APERTURE,          mode->reg_adc_p_aperture );
  spi_ice40_reg_write32(spi, REG_ADC_P_CLK_COUNT_RESET,   mode->reg_adc_p_reset );

  spi_ice40_reg_write32(spi, REG_SA_P_CLK_COUNT_PRECHARGE, mode->reg_sa_p_clk_count_precharge );

  // can add the reg reset here.

*/
}



