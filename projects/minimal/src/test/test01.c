
/*
  more an example of how to use embedded repl.
  need to call sleep though. sleep .

*/

#include <stdio.h>
#include <assert.h>
#include <string.h>   // strcmp

#include <app.h>
// #include <lib2/util.h>         // msleep()



bool app_test01( app_t *app , const char *cmd)
{
  assert(app);
  assert(cmd);


  if( strcmp(cmd, "test01") == 0) {

    app_repl_statements(app, "set k405 1; set k406 0; set k407 0\n" );
    return 1;
  }

  return 0;
}


