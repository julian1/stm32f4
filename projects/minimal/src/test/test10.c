
/*
  should duplicate test and use adc to verify  value is ok/within bounds.

  perhaps consolidate tests here.
  for ref and dac?
	-------

  useful to test dcv-source lts, amplifier, adc. - and full chain for a few voltages.


*/

#include <stdio.h>
#include <assert.h>
#include <string.h>   // strcmp

#include <lib2/util.h>      // msleep(), ARRAY_SIZE

#include <app.h>



static void test (app_t *app)     // should be passing the continuation.
{

  // test voltages
  double fa[]  = { 10, 1, 0.1, 0.01, 0,  -0.01, -0.1, -1, -10, 0  } ;

  for(unsigned i = 0; i < ARRAY_SIZE(fa); ++i)  {

    char buf[100];
    snprintf( buf, 100, "dcv-source lts %lf\n", fa[i]);
    app_repl_statements( app, buf);

    msleep( 1000, &app->system_millis);   // sleep 1s.
  }
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






