
/*
  simplest possible test.
  test 4094, by bouncing relay K403 a few times,
  -----

  TODO consider changing to test all relays . eg just click each one.

*/

#include <stdio.h>
#include <assert.h>
#include <string.h>   // strcmp


#include <lib2/util.h>      // msleep()
#include <lib2/format.h>   // format_bits()



#include <app.h>
#include <mode.h>
#include <app.h>



static void test (app_t *app)
{

  _mode_t mode = * app->mode_initial;
  bool flip = 0;

  for(unsigned i = 0; i < 4; ++i ) {

    flip = ! flip;
    mode.first.K403 =  flip ? 0b01 : 0b10;
    spi_mode_transition_state(  (spi_t *)app->spi_fpga0, app->spi_4094, app->spi_mdac0, &mode, &app->system_millis);
    msleep( 300, &app->system_millis);
  }


  for(unsigned i = 0; i < 4; ++i ) {

    flip = ! flip;
    mode.first.K703 =  flip ? 0b01 : 0b10;
    spi_mode_transition_state(  (spi_t *)app->spi_fpga0, app->spi_4094, app->spi_mdac0, &mode, &app->system_millis);
    msleep( 300, &app->system_millis);
  }

}



bool app_test02( app_t *app , const char *cmd)
{
  assert(app);
  assert(app->magic == APP_MAGIC);
  assert(cmd);

  if( strcmp(cmd, "test02") == 0) {
    test( app );
    return 1;
  }

  return 0;
}



#if 0
  // done directly.

   // click the relays, and analog switch.
  _4094_state_t mode;
  memset(&mode, 0, sizeof(mode));

  mode.K701 =  flip ? 0b01 : 0b10;
  mode.K404 =  flip ? 0b01 : 0b10;
  mode.K407 =  flip ? 0b01 : 0b10;
  mode.U1003 = flip ? 0b1111 : 0b000;

  // make sure we are muxing spi,
  spi_mux_4094( app->spi );

  // can probe 4094 signals - by connecting scope to 4094 extension header pins.
  // write single byte - should be enough to flip a relay.
  // JA spi_4094_write_n(app->spi, (uint8_t *)& magic , 1 );
  spi_4094_write_n(app->spi, (uint8_t *)& mode, sizeof(mode) );

  // sleep 10ms.
  msleep(10, &app->system_millis);

  // now clear the relays
  mode.K701 = 0b00;
  mode.K404 = 0b00;
  mode.K407 = 0b00;
  spi_4094_write_n(app->spi, (uint8_t *)& mode, sizeof(mode) );


  /* EXTR. IMPORTANT. must call spi_mux_ice40 again
        - to prevent spi emission on 4094 spi clk,data lines.
        - when reading the adc counts
  */
  spi_mux_ice40(app->spi);

}
#endif





