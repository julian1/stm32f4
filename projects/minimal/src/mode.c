

#include <assert.h>
#include <stdio.h>



#include <ice40-reg.h>

#include <spi-ice40.h>
#include <spi-4094.h>
#include <spi-ad5446.h>
#include <spi-dac8811.h>



#include <lib2/util.h>      // msleep
#include <lib2/format.h>   // str_format_bits
#include <mode.h>




static void state_format ( uint8_t *state, size_t n)
{
  assert(state);

  char buf[100];
  for(unsigned i = 0; i < n; ++i ) {
    printf("v %s\n",  str_format_bits(buf, 8, state[ i ]  ));
  }
}



void spi_mode_transition_state( uint32_t spi, const Mode *mode, volatile uint32_t *system_millis  /*, uint32_t update_flags */ )
{
  assert(mode);

  // printf("4094 size %u\n", sizeof(_4094_state_t));
  assert( sizeof(_4094_state_t) == 7 );

  // mux spi to 4094. change mcu spi params, and set spi device to 4094
  spi_mux_4094 ( spi);

/*
  printf("-----------\n");
  printf("write first state\n");
  state_format (  (void *) &mode->first, sizeof(mode->first) );
*/

  // and write device
  spi_4094_reg_write_n(spi, (void *) &mode->first, sizeof( mode->first ) );

  // sleep 10ms
  msleep(10, system_millis);

/*
  // and format
  // printf("write second state\n");
  state_format ( (void *) &mode->second, sizeof(mode->second) );
*/

  // and write device
  spi_4094_reg_write_n(spi, (void *) &mode->second, sizeof(mode->second) );

  /////////////////////////////

  // now write dac state

   // spi_mux_dac8811(app->spi);
  spi_mux_ad5446( spi );

  // for dac881 1eg. 0=-0V out.   0xffff = -7V out. nice.
  // spi_dac8811_write16( app->spi, mode->dac_val );

  // for ad5444  14bit max is 0x3fff.
  spi_ad5446_write16( spi, mode->dac_val );




  /////////////////////////////

  // now do fpga state
  spi_mux_ice40(spi);


  assert( sizeof(reg_direct_t) == 4);
  spi_ice40_reg_write_n(spi, REG_DIRECT,  &mode->reg_direct,  sizeof( mode->reg_direct) );

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



