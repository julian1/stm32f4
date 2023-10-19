

#include <stdio.h>      // printf, scanf
#include <string.h>     // strcmp, memset
#include <assert.h>

// lib
#include "util.h"       // msleep()



// local
#include "reg.h"
#include "mux.h"        // mux_ice40()
#include "spi-ice40.h"  // spi_ice40_reg_write32()


#include "app.h"

// modes of operation.
#include "mode.h"





bool test11( app_t *app , const char *cmd)
{
  assert(app);
  assert(cmd);
  assert(app->mode_initial);


    if(strcmp(cmd, "test11") == 0) {

      printf("test11 amplify LO to test Vos\n");

      app->test_in_progress = 0;

      // ease syntax
      Mode *mode = app->mode_current;

      *mode =  *app->mode_initial;

      // route ground through himux2.
      // could also try ref-lo.
      mode->reg_direct.himux2 = S4;
      mode->reg_direct.himux  = S2;

      mode->reg_direct.azmux  = SOFF;    // ignored in NO_AZ mode.


      mode->reg_mode = MODE_NO_AZ;

      // 100x gain amplifier
      mode->first. U506 =  W3;        // amp feedback should never be turned off.
      mode->second.U506 =  W3;

      // 10x
      // mode->first. U506 =  W2;        // amp feedback should never be turned off.
      // mode->second.U506 =  W2;



      do_4094_transition( app->spi, app->mode_current,  &app->system_millis );

    return 1;
    }

  return 0;
}



/*
  oct 20.
  with lsk389.
  measure 0.203V.  at 100x. == 2mV. Vos.
  measure 0.017202 mV.  at 10x. == 1.7mV. Vos.

  So. the amplifier lo has 0.2V offset. which explains some of the apparant misalignment of scope input.


*/
