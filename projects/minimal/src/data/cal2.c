
/*
  Feb 2026.

  Can we create a calibration with just a ratio term.

  - no scale, offset.   so cannot compute noise
  - assume that parasitic/ leakage etc. will be constant. across az.

  by just setting the current ratio do not even need a value.

*/

#include <stdio.h>
#include <assert.h>
#include <math.h>     // fabs


#include <peripheral/gpio.h>
#include <peripheral/spi-ice40.h>
#include <device/spi-fpga0-reg.h>

#include <lib2/util.h>    // yield_with_msleep

#include <mode.h>
#include <util.h> // nplc_to_aperture()

#include <data/data.h>

#include <data/matrix.h> // m_set_row()
#include <data/regression.h>




#include <devices.h>

  /*
      separate out the tick_millis . that always updates with += 1000;
      from the sleep. that can be set where used.

      rename sleep_with_yield()
      and call like this

      *sleep_millis = 1000;
      sleep_with_yield_with_msleep( sleep_millis, yield, yield_ctx);
  */

  /*
    EXTR.
    we simplify this a lot ...
    and avoid passing around system_millis.


    *sleep_millis = 1000;
    while( system_millis > 0) {
      yield( yield_ctx);
    }

  */


void data_cal2(

  data_t    *data ,
  devices_t *devices,


  _mode_t *mode,
  unsigned model_spec,

  // app stuff
  gpio_t      *gpio_trigger_internal,
  volatile uint32_t *system_millis,
  void (*yield)( void * ),
  void * yield_ctx
)
{
  assert(devices);
  assert(data);
  assert(data->magic == DATA_MAGIC) ;
  assert(mode);

  // UNUSED(gpio_trigger_internal);
  UNUSED(model_spec);
  // UNUSED(system_millis);
  // UNUSED(yield);
  // UNUSED(yield_ctx);


  // trig. off.
  //  call reset.

  printf("whoot cal2() \n");


  // we are not doing hi/lo herer
  double values[ 10 ];


 // let things settle from spi emi burst, and board DA settle, amp to come out of lockup.
  // equivalent to discarding values

  // trig off
  gpio_write( gpio_trigger_internal, 0 );

  printf("sleep\n");
  yield_with_msleep( 1 * 1000, system_millis, yield, yield_ctx);



  // have to set the mode. and the sequence inputs .

  // it would be really nice to be able to set the mode.  here.
  // perhaps just pass app...
  // *app->mode_current = *app->mode_initial;



  mode_sa_set(mode, "0" );      // special mode. where just samples zero.

  // should have an accessor ...
  // trig on
  gpio_write( gpio_trigger_internal, 1 );


  // take obs loop
  for(unsigned i = 0; i < ARRAY_SIZE(values); ++i)
  {

    printf("i %u\n", i );

    // setup adc nplc
    mode->adc.p_aperture = nplc_to_aperture( 10, data->line_freq );				// fix jul 2024.


    // wait for adc data, on interupt
    while( !data->adc_interupt_valid ) {
      if(yield)
        yield( yield_ctx);
    }
    data->adc_interupt_valid = false;


    spi_t *spi_fpga0  = devices->spi_fpga0;
    assert(spi_fpga0);


    // embed a 8 bit. counter ini the reg_status and use it for the measure.

    uint32_t status = spi_ice40_reg_read32( spi_fpga0, REG_STATUS );
    UNUSED(status);

    uint32_t clk_count_mux_reset  = spi_ice40_reg_read32( spi_fpga0, REG_ADC_CLK_COUNT_MUX_RESET);    // useful check.
    uint32_t clk_count_mux_neg    = spi_ice40_reg_read32( spi_fpga0, REG_ADC_CLK_COUNT_MUX_REF_NEG);
    uint32_t clk_count_mux_pos    = spi_ice40_reg_read32( spi_fpga0, REG_ADC_CLK_COUNT_MUX_REF_POS);
    uint32_t clk_count_mux_rd     = spi_ice40_reg_read32( spi_fpga0, REG_ADC_CLK_COUNT_MUX_REF_RD);   // check.
    uint32_t clk_count_mux_sig    = spi_ice40_reg_read32( spi_fpga0, REG_ADC_CLK_COUNT_MUX_SIG);

    printf("counts %6lu %lu %lu %lu %6lu",
      clk_count_mux_reset,
      clk_count_mux_sig,
      clk_count_mux_neg, clk_count_mux_pos, clk_count_mux_rd
    );
    printf("\n");


    double w =  (clk_count_mux_pos  +  clk_count_mux_rd)  /   (clk_count_mux_neg + clk_count_mux_rd) ;

    printf("w %f", w );
    values[ i ] = w;

    /*
      w is independent of aperture/nplc.  and independent of constant parasitics. and signal input.

      no.  it is still not quite right.
      because if signal input changes - this will change the currents pushed.
      except the    input circuit - can have a different offset.

      not sure.

    */

    // so we would just push into a column array.  then take some averages.
    // actually simpler way.  is to compute the average.

  }


}
