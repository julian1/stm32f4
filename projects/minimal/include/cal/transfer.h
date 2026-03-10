

#pragma once

// consider renam app_transfer.h.

#include <stdbool.h>


typedef struct app_t app_t;
typedef struct cal_t cal_t;

// put all these in a separate header???



/*
  use reasonable names
  we can always put these functions in an array
  and call by index
  ----

  EXTR> may want to pass a context.
    to mangge setting nplc. number of obs. delay. etc.
*/


typedef struct transfer_t
{
  // magic
  // const char *name;

  // shoudl support return false/bool on error
  // consider rename step_reference(). or reference_setup() target_setup() etc.
  void (*step1)( app_t *app);   // reference/transfer/source
  void (*step2)( app_t *app);   // target
  void (*cal_set_value)( cal_t *cal, double mean0, double mean1);

} transfer_t;

void app_transfer( app_t *app, transfer_t *transfer);


// we want a repl here.
bool app_transfer_repl_statement( app_t *app, const char *cmd);

// void app_cal_setup( app_t *app);
// void app_cal_finish( app_t *app);

void app_cal_w( app_t *app);
void app_cal_b( app_t *app);
void app_cal_b10( app_t *app);
void app_cal_b100( app_t *app);
void app_cal_b1000( app_t *app);

void app_cal_div100( app_t *app);
void app_cal_div1000( app_t *app);


void app_cal_all( app_t *app);


void app_fill_buffer( app_t *app, double *values, size_t n);
void app_fill_buffer1( app_t *app, double *pos_values, double *neg_values, size_t n);



