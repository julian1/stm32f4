
/*
  test sa/adc.


  - Note. we can redirect the data ready interrupt here too.

    adc test is useful when populating/testing adc coponents.
    and when no calibration is available to interpret adc counts.
*/


#include <stdio.h>
#include <assert.h>
#include <string.h>   // strcmp


#include <mode.h>
#include <app.h>
#include <lib2/util.h>    // msleep()

#include <device/fpga0_reg.h>  // modes


bool app_test09( app_t *app , const char *cmd)
{
  assert(app);
  assert(cmd);
  assert(app->mode_initial);

  if( strcmp(cmd, "test09") == 0) {

    printf("test sa/adc \n");

    // this is using the current mode...
    // would be better if could apply repl to a custom mode.

    // should work???
    // thiis could be setup externally . and doesn't really matter
    app_repl_statements(app, "reset; dcv-source lts 1;  dcv-source chan 1 ; " ); // nplc.


   _mode_t *mode = app->mode_current;


    // set up sequence acquision
    mode->reg_mode = MODE_SA_ADC;       // mode 7
    mode->sa.reg_sa_p_seq_n  = 2;
    mode->sa.reg_sa_p_seq0 = (PC01 << 4) | S1;                // dcv/ chan 1.
    mode->sa.reg_sa_p_seq1 = mode->sa.reg_sa_p_seq0 ;         // the same

    // ok. so we need to encode the trigger.
    mode->sa.reg_sa_p_trig = 1;


    spi_mode_transition_state( (spi_t *) app->spi_fpga0, app->spi_4094, app->spi_mdac0, mode, &app->system_millis);

    // printf("sleep 5s\n");  // really need the yield would be quite nice here.
    // msleep(5 * 1000,  &app->system_millis);

    // keeps running - because using the current mode


    return 1;
  }


  return 0;
}


