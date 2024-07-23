
/*
  we need this repeated - whereby the adc actually verifies the value.

*/

#include <stdio.h>
#include <assert.h>
#include <string.h>   // strcmp

#include <lib2/util.h>      // msleep()

#include <app.h>



static void test (app_t *app)
{
  unsigned pause = 1000; // in milli

  /*
    would be better to construct as loop and array.
    could sprintf to format the double value to use.
    and for
  */

  app_repl_statements(app, "dcv-source 10\n" );
  msleep( pause, &app->system_millis);

  app_repl_statements(app, "dcv-source 1\n" );
  msleep( pause, &app->system_millis);

  app_repl_statements(app, "dcv-source 0.1\n" );
  msleep( pause, &app->system_millis);

  app_repl_statements(app, "dcv-source 0.01\n" );     // resistor not fitted, but can still run the test.
  msleep( pause, &app->system_millis);

}



bool app_test10( app_t *app , const char *cmd)
{
  assert(app);
  assert(app->magic == APP_MAGIC);
  assert(cmd);

  if( strcmp(cmd, "test10") == 0) {
    test( app );
    return 1;
  }

  return 0;
}






