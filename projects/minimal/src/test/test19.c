
/*
  test adc ref mux current switching and integrator.
  just a mode on the fpga.
  so this is simple enough that does not need to be a separate test, but good to make express.

*/


#include <stdio.h>
#include <assert.h>
#include <string.h>   // strcmp


#include <mode.h>
#include <app.h>
#include <ice40-reg.h>    // MODE
#include <lib2/util.h>    // msleep()


bool app_test19( app_t *app , const char *cmd)
{
  assert(app);
  assert(cmd);
  assert(app->mode_initial);

  if( strcmp(cmd, "test19") == 0) {

    printf("test adc refmux switching\n");

    _mode_t mode = *app->mode_current;

    mode.reg_mode =  MODE_ADC_REFMUX_TEST;
    spi_mode_transition_state( app->spi, &mode, &app->system_millis);

    printf("sleep 5s\n");  // really need the yield would be quite nice here.
    msleep(60 * 1000,  &app->system_millis);


    return 1;
  }


  return 0;
}


