
#pragma once

#include <lib2/cbuffer.h>
#include <lib2/cstring.h>



typedef struct _mode_t _mode_t;



typedef struct app_t
{


  // remove. should be able to query the led state  to invert it...
  // no. it's ok. led follows the led_state
  bool led_state ;     // for mcu. maybe change name to distinguish

  uint32_t soft_500ms;

  // updated on interupt. should probably be declared volatile.
  // but functions that use can also declare volatile
  volatile uint32_t system_millis;

  cbuf_t  console_in;
  cbuf_t  console_out;

  cstring_t     command;


  uint32_t  spi;

  ////////
/*
  bool led_blink;
  bool test_relay_flip;
*/

  const _mode_t *mode_initial;
  _mode_t *mode_current;


  uint32_t line_freq;


} app_t;


void app_repl_statements(app_t *app,  const char *stmts);



bool app_test05( app_t *app , const char *cmd);
bool app_test14( app_t *app , const char *cmd);

