

#include <stdio.h>
#include <assert.h>


#include <app.h>
#include <mode.h>
// #include <lib2/util.h>    // UNUSE

#include <data/cal.h>




void app_cal_01( app_t *app)
{

  printf("cal01\n");

  mode_reset( app->mode);

  // except we want the LTS. 10. input range.

  // TODO - want a function with separate args
  // should be called switch range.
  bool ret = app_repl_range( app, "LTS 10");
  assert(ret);


  // set the LTS. to 1V.

}

