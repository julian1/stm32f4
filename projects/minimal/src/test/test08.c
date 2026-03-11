
/*
  test adc ref mux current switching and integrator.
  just a mode on the fpga.
  so this is simple enough that does not need to be a separate test, but good to make express.

*/


#include <stdio.h>
#include <assert.h>
#include <string.h>   // strcmp


#include <device/spi-fpga0-reg.h>  // modes

#include <mode.h>

#include <app.h>
#include <test/test.h>




bool app_test08( app_t *app , const char *cmd)
{
  assert(app);
  assert(cmd);

  if( strcmp(cmd, "test08") == 0) {

    printf("test adc refmux switching\n");

    // app_repl_statements(app, "reset;   set mode 5;" );  should work???

    _mode_t *mode = app->mode;
    mode_reset( mode);


    cr_mode_set( &mode->reg_cr, MODE_ADC_MUX_REF_TEST);

    app_transition_state( app);

    printf("sleep 5s\n");  // really need the yield would be quite nice here.
    app_msleep( app, 5 * 1000);


    // could reset here.

    return 1;
  }


  return 0;
}


