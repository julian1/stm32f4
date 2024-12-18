
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
#include <lib2/util.h>    // msleep()

#include <device/fpga0_reg.h>  // modes


bool app_test08( app_t *app , const char *cmd)
{
  assert(app);
  assert(cmd);
  assert(app->mode_initial);

  if( strcmp(cmd, "test08") == 0) {

    printf("test adc refmux switching\n");

    // app_repl_statements(app, "reset;   set mode 5;" );  should work???

#if 1

    _mode_t mode = *app->mode_current;

    mode.reg_mode =  MODE_ADC_REFMUX_TEST;
    spi_mode_transition_state( (spi_t *) app->spi_fpga0, app->spi_4094, app->spi_mdac0, &mode, &app->system_millis);
#endif

    printf("sleep 5s\n");  // really need the yield would be quite nice here.
    msleep(5 * 1000,  &app->system_millis);



    return 1;
  }


  return 0;
}


