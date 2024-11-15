
/*
  blink the fpga leds in a nice pattern.
  using direct mode.
  shouldn't need fpga clk.

  use when first populating board, to test mcu/fpga and spi comms
  also checks the reg_direct value.

  note power consupmption - mcu 21mA. fpga 19mA.  with cmos oscillator running.
  but needs more on first power up- else slow cap charge, and won't start.
*/

#include <stdio.h>
#include <assert.h>
#include <string.h>         // strcmp


#include <lib2/util.h>     // msleep()
#include <lib2/format.h>   // format_bits()



#include <app.h>
#include <peripheral/spi-ice40.h>
#include <device/reg_u102.h>





static void test (app_t *app)
{
  assert( 0);
  // spi_mux_ice40( app->spi_u102 );

  // put direct mode
  spi_ice40_reg_write32( app->spi_u102, REG_MODE , MODE_DIRECT );



  for(unsigned i = 0; i < 31; ++i )  {

    static uint32_t counter = 0;
    ++counter;
    uint32_t magic = counter  ^ (counter >> 1);
    /*
    static uint32_t magic = 0;
    ++magic;
    */

    // blink led... want option. so can write reg_direct
    // note - led will only, actually light if fpga in default mode. 0.
    spi_ice40_reg_write32( app->spi_u102, REG_DIRECT, magic );

    // check the magic numger
    uint32_t ret = spi_ice40_reg_read32( app->spi_u102, REG_DIRECT);
    if(ret != magic ) {
      // comms no good
      char buf[ 100] ;
      printf("comms failed, returned reg value %s\n",  str_format_bits(buf, 32, ret ));
    } else {
      // printf("comms ok\n");
    }

    msleep( 100,  &app->system_millis);
  }
}



bool app_test01( app_t *app , const char *cmd)
{
  assert(app);
  assert(app->magic == APP_MAGIC);
  assert(cmd);



  if( strcmp(cmd, "test01") == 0) {
    test( app );
    return 1;
  }

  return 0;
}


