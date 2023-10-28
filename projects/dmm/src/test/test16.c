
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

/*
  Ok, we want to be able to set the input source. and then the amplifier gain.

*/


// prefix app_test16()
bool test16( app_t *app , const char *cmd/*,  const Mode *mode_initial*/)
{
  assert(app);
  assert(cmd);
  assert(app->mode_initial);

//    uint32_t u1;  // rename
//    int32_t i0;



  return 0;
}
