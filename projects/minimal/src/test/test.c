
/*
  having the test repl handling here is better than
  in app.c
  not sure if/how to combine with test/support.h.
  though.

*/

#include <assert.h>


#include <app.h>
#include <test/test.h>


bool app_test_repl_statement( app_t *app,  const char *cmd)
{
  assert(app && app->magic == APP_MAGIC);


  if( false) ;

  else if( app_test01( app, cmd)) { }
  else if( app_test02( app, cmd)) { }

  else if( app_test08( app, cmd)) { }
  else if( app_test09( app, cmd)) { }

  else if( app_test10( app, cmd)) { }
  else if( app_test11( app, cmd)) { }

  else if( app_test12( app, cmd)) { }
  else if( app_test14( app, cmd)) { }
  else if( app_test15( app, cmd)) { }

#if 0

  else if( app_test20( app, cmd, (void (*)(void *))app_update_simple_with_data, app )) { }
  else if( app_test40( app, cmd, (void (*)(void *))app_update_simple_with_data, app )) { }
  else if( app_test41( app, cmd, (void (*)(void *))app_update_simple_with_data, app )) { }
  else if( app_test42( app, cmd, (void (*)(void *))app_update_simple_with_data, app )) { }
#endif


  else if( app_test50( app, cmd)) { }
  else if( app_test51( app, cmd)) { }
  else if( app_test52( app, cmd)) { }


  else
    return 0;

  return 1;
}


