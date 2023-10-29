/*
  - long running process.
  - just take control of the main loop.
  as alternative to function yielding.
  ------
  Advantage.   for long running storage for loop - we can isoalte the memory and don't even need to pass it.
*/


#include <libopencm3/stm32/gpio.h>    // led

#include <stdio.h>    // printf, scanf
#include <string.h>   // strcmp, memset
#include <assert.h>



#include "app.h"
#include "util.h"
#include "mux.h"
#include "reg.h"
#include "mode.h"   // we need to set up 4094 state *and* reg_direct


#include "spi-ice40.h"

// loop3 structure. should probably be in own header file.




void app_loop3( app_t *app )
{
  UNUSED(app);

  // Loop3 loop3 ;
  // memset(&loop3, 0, sizeof(loop3));




  // this overrides nplc/aperture.
  *app->mode_current = *app->mode_initial;
  //  alias to ease syntax
  Mode *mode = app->mode_current;
  // ampliier 1x. default?
  // input relays are all off

  mode->reg_mode = MODE_NO_AZ;

  mode->reg_direct.sig_pc_sw_ctl  = SW_PC_SIGNAL;   // pc switch muxes signal.
  mode->reg_direct.azmux          = S1;             // azmux muxes pc-out // EXTR. review.  not muxing ref-lo here.
  mode->reg_direct.himux2 = S5 ;    // reg-hi.
  mode->reg_direct.himux  = S2 ;    // himux2

  mode->reg_aperture = nplc_to_aper_n( 10, app->lfreq );    // dynamic. problem. maybe 50,60Hz. or other.

  // do the state transition
  app_transition_state( app->spi, mode,  &app->system_millis );



  // MAT *row = NULL;


  // this would be in the  loop when changing parameters
  // don't think this is quite right.
  {
  spi_ice40_reg_write32(app->spi, REG_SA_ARM_TRIGGER, 0 );      // arm to halt.
  printf("arm and block\n");
  while( !  (spi_ice40_reg_read32(app->spi, REG_STATUS) & (1<<8) )) ;  // wait for adc to be ready/valid.
  printf("adc measure valid/ done\n");
  app->adc_drdy = false;
  printf("trigger/restart\n");
  spi_ice40_reg_write32(app->spi, REG_SA_ARM_TRIGGER, 1 );    // trigger. signal acquisition
  }

  // ok. we need to be able to manipulate nplc.


  for(unsigned i = 0; i < 5; ++i ) {

    // block on interupt.
    while(! app->adc_drdy );
    app->adc_drdy = false;


    // we can construct our matrix directly. without needing Run.
    // we don't care
    uint32_t clk_count_mux_neg = spi_ice40_reg_read32( app->spi, REG_ADC_CLK_COUNT_MUX_NEG);
    uint32_t clk_count_mux_pos = spi_ice40_reg_read32( app->spi, REG_ADC_CLK_COUNT_MUX_POS);
    uint32_t clk_count_mux_rd  = spi_ice40_reg_read32( app->spi, REG_ADC_CLK_COUNT_MUX_RD);

    printf("loop3 data  %lu %lu %lu\n", clk_count_mux_neg, clk_count_mux_pos, clk_count_mux_rd);

    // if have count of data then we are done.

  }

  // leave running as is
  // turn off. -
  spi_ice40_reg_write32(app->spi, REG_SA_ARM_TRIGGER, 0 );      // arm to halt.

  return ;
}








  // need to support fpga core reset. to ensure parameters are being respected.
  // or else sleeping.

  /*
    - Ok. am not sure we really need any of this.
    - we just pause, until we get a trigger.

    - DURING RECONFIGURATION.
        - just reconfigure. and wait for the adc to signal.
        - or a separate reset. flag but *not* a edge triggered halt.

        - Or we just need a valid clear.

  */

  // we do want the arm, and trigger. for external triggering etc.
  // but this is different to 'tarm'.
  // rather than call it arm. might be better to call wait.


  // the lo can/should be taken.  probably by changing the az-mux.
  // or even use the az mode. to take ref-hi, and ref-lo.
/*

  uint32_t spi = app->spi;

  // now do fpga state
  mux_ice40(spi);

  // set mode
  spi_ice40_reg_write32(spi, REG_MODE, MODE_NO_AZ );


  F f;
  memset(&f, 0, sizeof(f));

  // so need to set up the muxes - for the sample
  f.led0 = app->led_state;

  mux_ice40(app->spi);
  spi_ice40_reg_write_n(app->spi, REG_DIRECT, &f, sizeof(f) );


  // spi_ice40_reg_write_n(spi, REG_DIRECT,  &mode->reg_direct,  sizeof( mode->reg_direct) );

  // write the aperture. REG_ADC_P_APERTURE
  //  spi_ice40_reg_write32(spi, REG_CLK_SAMPLE_DURATION, mode->reg_aperture );
*/



#if 0
static void loop3_update_soft_500ms(app_t *app)
{
  assert(app);

  app->led_state = ! app->led_state;

  // blink mcu led
  // be explicit. don't hide top-level state.
  if(app->led_state)
    gpio_clear( app->led_port, app->led_out);
  else
    gpio_set(   app->led_port, app->led_out);


  // hearbeat and led flash
  mux_ice40(app->spi);

  // use a magic number to blink the led. and test comms
  uint32_t magic = app->led_state ? 0b010101 : 0b101010 ;
  spi_ice40_reg_write32( app->spi, REG_LED, magic);
  uint32_t ret = spi_ice40_reg_read32( app->spi, REG_LED);
  if(ret != magic ) {

    printf("comms bads\n");
/*
    // comms no good
    char buf[ 100] ;
    printf("no comms, wait for ice40 v %s\n",  format_bits(buf, 32, ret ));
    app->comms_ok = false;
*/
    // REVIEW
    // return
  }

}
#endif


/*

typedef struct Loop3  {

  uint32_t  whoot;

} Loop3 ;
*/
