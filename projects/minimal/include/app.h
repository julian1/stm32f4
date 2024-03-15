
#pragma once

#include <lib2/cbuffer.h>
#include <lib2/cstring.h>


/*
  there are two main cases to yield.
    - for time,delay.
    - until we have enough measurements in buffer.

    - both cases - should be handled with functions with 'app_yield_wait_dealay( app, mdelay )' ,  'app_yield_wait_measurements( app, 10 ) like naming .
    - don't even really need the indirection of function pointer and context.

  ---
  - yield function - can be used to process/service incomming raw data - eg. convert adc counts to a value.
      while control is blocking waiting for measurement data.
      without needing two stacks.  eg.

      while(!app->data_ready)
        app->yield( app->yield_ctx);


      // wait for obs etc.
      while( queue_count(app->measurements) < 10)
        app->yield( app->yield_ctx);

      // sleep
      system_millis = 0;
      while( system_millis < 500 )
        app->yield( app->yield_ctx);

      etc


      the limitation of a single stack yield(), is no nested co-recursion.
      but thats ok, we only engage in one user task/sequence/ at a time

      actually we can block on more than two events.  we just need to write the poll loop for the tests, and call app->yield.
      ----
      uses -
        1. for blocking when need to wait on some condition eg. measurement data
        2. long running functions, to keep processing.
*/


typedef struct _mode_t _mode_t;

typedef struct data_t data_t;





typedef struct app_t
{
  uint32_t magic;


  // remove. should be able to query the led state  to invert it...
  // no. it's ok. led follows the led_state, more than one thing follows.
  bool led_state ;     // for mcu. maybe change name to distinguish

  uint32_t soft_500ms;

  // updated on interupt. should probably be declared volatile.
  // but functions that use can also declare volatile
  volatile uint32_t system_millis;

  cbuf_t  console_in;
  cbuf_t  console_out;

  cstring_t     command;


  uint32_t  spi;


  const _mode_t *mode_initial;

  _mode_t *mode_current;


  // static input property of the environment
  // does not really belon in mode. mode has values dependent.
  uint32_t line_freq;

/*
  // yield abstracts the function, and allows substitution, but may not be needed.
  // eg. versus just calling app_update(app); or  app_yield( app) etc.
  void (*yield)(void *);
  void *yield_ctx;
  
  not quite right. because anything running at app context - can determine the yield.
    anything below - eg. peripheral,data,  should not have any knowledge of app structure.
  
  eg. the yield func, is a dependency, and should be passed explicitly to whatever is calling it.
*/


  data_t  *data;

} app_t;



#define APP_MAGIC   456



void app_init_buffers(app_t *app);
void app_update(app_t *app);
void app_loop(app_t *app);
void app_systick_interupt(app_t *app);



/*

void app_yield_with_delay(app_t *app, uint32_t delay );
void app_yield(app_t *app);

*/



void app_repl_statement(app_t *app,  const char *cmd);
void app_repl_statements(app_t *app,  const char *s);

/*
  IMPORTANT - none of these take arguments anymore,
  the precondition state, should be setup in other commands.
  so we could remove the cmd argument.  and do the test in the repl command.
  but leave for moment, in case want to be able to pass flags.
*/

bool app_test01( app_t *app , const char *cmd);
bool app_test02( app_t *app , const char *cmd);
bool app_test03( app_t *app , const char *cmd);
bool app_test05( app_t *app , const char *cmd);
bool app_test14( app_t *app , const char *cmd);
bool app_test15( app_t *app , const char *cmd);


