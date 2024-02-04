
#pragma once

// #include <lib2/usart.h>
// #include <lib2/streams.h>
// #include <lib2/util.h>   // msleep()
#include <lib2/cbuffer.h>
#include <lib2/cstring.h>
// #include <lib2/format.h>   // trim_whitespace()




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

  bool led_blink;

} app_t;


