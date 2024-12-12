
/*
  same as led_dance() now?

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





bool app_test01( app_t *app , const char *cmd)
{
  assert(app);
  assert(app->magic == APP_MAGIC);
  assert(cmd);


  if( strcmp(cmd, "test01") == 0) {

    app_led_dance( app);

    return 1;
  }

  return 0;
}


