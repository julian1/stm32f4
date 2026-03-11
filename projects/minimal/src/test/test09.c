
/*
  test sa/adc.


  - Note. we can redirect the data ready interrupt here too.

    adc test is useful when populating/testing adc coponents.
    and when no calibration is available to interpret adc counts.
*/


#include <stdio.h>
#include <assert.h>
#include <string.h>   // strcmp


#include <device/spi-fpga0-reg.h>  // modes
#include <peripheral/gpio.h>        // trigger


#include <mode.h>

#include <app.h>
#include <test/test.h>




bool app_test09( app_t *app , const char *cmd)
{
  assert(app);
  assert(cmd);



  if( strcmp(cmd, "test09") == 0) {

    printf("test sa/adc \n");

    // this is using the current mode...
    // would be better if could apply repl to a custom mode.

    // should work???
    // thiis could be setup externally . and doesn't really matter
    app_repl_statements(app, "reset; dcv-source lts 1;  dcv-source chan 1 ; " ); // nplc.


    _mode_t *mode = app->mode;
    mode_reset( mode);


    // ordinary adc/sequence acquision
    cr_mode_set( &mode->reg_cr, MODE_SA_ADC);

/*
    mode->sa.p_seq_n  = 2;
    mode->sa.p_seq0 = (PC01 << 4) | S1;                // dcv/ chan 1.
    mode->sa.p_seq1 = mode.sa.p_seq0 ;         // the same
*/

    sa_state_t *sa = &mode->sa;
    sa->p_seq_n = 2;
    sa->p_seq_elt[ 0].azmux = S1;
    sa->p_seq_elt[ 0].pc = 0b01;



    // spi_mode_transition_state( &app->devices, &mode, &app->system_millis);
    app_transition_state( app);


    // ok. so we need to encode the trigger.
    // app_trigger( app, 1);   // aug 2025.
    gpio_write( app->gpio_trigger, 1);


    // printf("sleep 5s\n");  // really need the yield would be quite nice here.
    // msleep(5 * 1000,  &app->system_millis);

    // keeps running - because using the current mode


    return 1;
  }


  return 0;
}


