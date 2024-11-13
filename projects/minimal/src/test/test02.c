
/*
  just test 4094, by bouncing relay K407 a few times, to audibly check 4094 comms.

  could also pass an argument for which relay...
  no. can test easily with 'set k407 1' etc

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

  for(unsigned i = 0; i < 6; ++i ) {

    flip = ! flip;
    // mode.first.K407 =  flip ? 0b01 : 0b10;
    // mode.first.K703 =  flip ? 0b01 : 0b10;
    mode.first.K403 =  flip ? 0b01 : 0b10;
    spi_mode_transition_state( app->spi_u102, app->spi_4094, app->spi_ad5446, &mode, &app->system_millis);
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
  // JA spi_4094_reg_write_n(app->spi, (uint8_t *)& magic , 1 );
  spi_4094_reg_write_n(app->spi, (uint8_t *)& mode, sizeof(mode) );

  // sleep 10ms.
  msleep(10, &app->system_millis);

  // now clear the relays
  mode.K701 = 0b00;
  mode.K404 = 0b00;
  mode.K407 = 0b00;
  spi_4094_reg_write_n(app->spi, (uint8_t *)& mode, sizeof(mode) );


  /* EXTR. IMPORTANT. must call spi_mux_ice40 again
        - to prevent spi emission on 4094 spi clk,data lines.
        - when reading the adc counts
  */
  spi_mux_ice40(app->spi);

}
#endif





