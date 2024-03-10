

#include <stdio.h>
#include <assert.h>
#include <string.h>   // strcmp

#include <app.h>
#include <mode.h>

#include <lib2/format.h>   // trim_whitespace()  format_bits()


#include <peripheral/spi-ice40.h>


#include <ice40-reg.h>


// #include <lib2/util.h>         // msleep()


/*
  blinking leds, and click relays.

  this is better expressed as a separate test. rather than handled/performed in the main loop.
  with conditional logic, that reg_mode == 0. etc.

*/




static void test (app_t *app)
{

  /*
      consider rename test_led_blink  and disable by default to aoid
      spurious spi transmissions during acquisition
      potentially move into /src/test
  */
  if(false &&  app->mode_current->reg_mode == 0 /*app->led_blink*/ ) {


    /* EXTR
        - avoid electrical/comms activity of a heart-beat/led blink, during sample acquisition.  only use as test.
    */
    spi_mux_ice40( app->spi );

/*
    static uint32_t counter = 0;
    ++counter;
    uint32_t magic = counter  ^ (counter >> 1 );
*/


    static uint32_t magic = 0;
    ++magic;

    // blink led... want option. so can write reg_direct
    // note - led will only, actually light if fpga in default mode. 0.
    spi_ice40_reg_write32( app->spi, REG_DIRECT, magic );

    // check the magic numger
    uint32_t ret = spi_ice40_reg_read32( app->spi, REG_DIRECT);
    if(ret != magic ) {
      // comms no good
      char buf[ 100] ;
      printf("comms failed, returned reg value %s\n",  str_format_bits(buf, 32, ret ));
    } else {
      // printf("comms ok\n");
    }
  }


#if 0
  if(app->test_relay_flip) {

    static bool flip = 0;
    flip = ! flip;


#if 0
      _mode_t mode;
      memset(&mode, 0, sizeof(mode));

      // mode.first.K701 =  flip ? 0b01 : 0b10;
      mode.first.K404 =  flip ? 0b01 : 0b10;
      mode.first.K407 =  flip ? 0b01 : 0b10;

      mode.second.U1003 = flip ? 0b1111 : 0b000;

      spi_mode_transition_state( app->spi, &mode, &app->system_millis);
#endif

#if 0
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
#endif

    }
#endif

}









bool app_test02( app_t *app , const char *cmd)
{
  assert(app);
  assert(cmd);


  if( strcmp(cmd, "test02") == 0) {

    test( app );

    return 1;
  }

  return 0;
}


