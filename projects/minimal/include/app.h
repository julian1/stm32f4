
#pragma once

#include <lib2/cbuffer.h>
#include <lib2/cstring.h>



typedef struct _mode_t _mode_t;



typedef struct app_t
{


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

  ////

  const _mode_t *mode_initial;
  _mode_t *mode_current;


  uint32_t line_freq;



/*
  - yield function - can be used to process/service incomming raw data - eg. convert adc counts to a value.
      while control is blocking waiting for measurement data.
      without needing two stacks.  eg.

      while(!app->data_ready)
        app->yield( app->yield_ctx);


      // wait for obs etc.
      while( queue_count(app->measurements) < 10)
        app->yield( app->yield_ctx);


      the limitation of a single stack yield(), is no nested co-recursion.
      but thats ok, we only engage in one user task/sequence/ at a time

      actually we can block on more than two events.  we just need to write the poll loop for the tests, and call app->yield.
*/

  /*void (*yield)(void *);
  void *yield_ctx;
  */

} app_t;



void app_msleep_with_yield(app_t *app, uint32_t delay );


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


