
/*
  test non-isoalted dac output voltages.

  seems too simple/redundant.
  but good to quickly verify/lockdown behavior - after populating components.
  -
  should duplicate test and use adc to verify  value is ok/within bounds.
  ----

*/

#include <stdio.h>
#include <assert.h>
#include <string.h>   // strcmp
// #include <math.h>   // floor()

#include <lib2/util.h>      // msleep(), ARRAY_SIZE

#include <app.h>



static void test (app_t *app)     // should be passing the continuation.
{
  // eg. max 0x3fff for ad544

  /*
    TODO - need to handle negative values. eg. produce positive outputs
    decide where/how to do the conversion from floating/continuous value to device dependent dac value.

  */

  for(unsigned i = 0; i <= 10; ++i)  {

    uint32_t val  = i * 0x3fff / 10 ;
    printf( "val %lx (%lu)\n", val , val );


    char buf[100];
    snprintf( buf, 100, "dcv-source dac %lu\n", val );
    app_repl_statements( app, buf);

    msleep( 500, &app->system_millis);   // sleep 1s.
  }
}



bool app_test11( app_t *app , const char *cmd)
{
  assert(app);
  assert(app->magic == APP_MAGIC);
  assert(cmd);

  if( strcmp(cmd, "test11") == 0) {
    test( app );
    return 1;
  }

  return 0;
}






