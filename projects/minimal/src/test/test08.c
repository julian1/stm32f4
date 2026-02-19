
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

#include <device/spi-fpga0-reg.h>  // modes


bool app_test08( app_t *app , const char *cmd)
{
  assert(app);
  assert(cmd);

  if( strcmp(cmd, "test08") == 0) {

    printf("test adc refmux switching\n");

    // app_repl_statements(app, "reset;   set mode 5;" );  should work???

    _mode_t *mode = app->mode;
    mode_reset( mode);


    // mode.reg_mode =  MODE_ADC_MUX_REF_TEST;
    mode_reg_cr_set( mode, MODE_ADC_MUX_REF_TEST);

    app_transition_state( app);

    printf("sleep 5s\n");  // really need the yield would be quite nice here.
    msleep(5 * 1000,  &app->system_millis);


    // could reset here.

    return 1;
  }


  return 0;
}


