

#include <stdio.h>
#include <assert.h>


#include <app.h>
#include <mode.h>
// #include <lib2/util.h>    // UNUSE

// #include <data/cal.h>




void app_cal_01( app_t *app)
{

  printf("cal01\n");

  mode_reset( app->mode);

  // set LTS. 10. input range.
  app_switch_range1( app, "LTS", "10");


  // set the LTS source. to 1V.
  mode_lts_set( app->mode, 1.0 );




}

