
/*
  test function, when first populating board, to test mcu/fpga spi comms
  note this checks the return reg_direct return value. so forms a self-test.

  - writes reg_direct, avoids mode. good for first test

*/

#include <stdio.h>
#include <assert.h>
#include <string.h>         // strcmp


#include <lib2/util.h>     // msleep()
#include <lib2/format.h>   // format_bits()



#include <app.h>
#include <peripheral/spi-ice40.h>
#include <ice40-reg.h>





static void test (app_t *app)
{

  spi_mux_ice40( app->spi );

  for(unsigned i = 0; i < 50; ++i )  {

    static uint32_t counter = 0;
    ++counter;
    uint32_t magic = counter  ^ (counter >> 1);
    /*
    static uint32_t magic = 0;
    ++magic;
    */

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

    msleep( 100,  &app->system_millis);
  }
}



bool app_test02( app_t *app , const char *cmd)
{
  assert(app);
  assert(cmd);


  if( strcmp(cmd, "test02") == 0) {

    printf("here\n");
    test( app );

    return 1;
  }

  return 0;
}


