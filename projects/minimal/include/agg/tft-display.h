
#pragma once


#ifdef __cplusplus
extern "C" {
#endif


/*
  not sure this sturct is worthwhile. but simplifies all argument passing..

  interface between c++ and c. for
  and most agg code, want to be called on every app_update().

  struct - simplifies the calling interface for test code.
  although not sure if really needed

  ---
  update() should be called from app_update() eg. main loop.
  -----------

*/


#include <stdint.h>
#include <stdbool.h>



typedef struct tft_t tft_t;
typedef struct data_t data_t;



#define TFT_DISPLAY_MAGIC 1827218

typedef struct tft_display_t tft_display_t;

struct tft_display_t
{

  uint32_t  magic;

  tft_t     *tft;
  int       page;

  // int count;

  // volatile int32_t *tick_up;   only used for performance test
  volatile uint32_t *system_millis;

  /* if manage the update field here if want, instead of app_t.
    this way, the repl can be localized. and typed on tft_display_t rather than app_t
  */
  /*
    common interface for tft, vfd, and display test code.
    and even the repl output.
    could factor to separate interface. but not clear how useful
  */
  void (*update)( tft_display_t *);
  void (*update_data)( tft_display_t *, data_t *data);
  void (*update_500ms)( tft_display_t *);

};


// accessors
void tft_display_update( tft_display_t *tft_display);
void tft_display_update_data( tft_display_t *tft_display, data_t *data);
void tft_display_update_500ms( tft_display_t *tft_display);


void tft_display_init( tft_display_t *tft_display,  tft_t *tft, volatile uint32_t *system_millis);

bool tft_display_repl_statement( tft_display_t *,  const char *cmd);


// need to communicate between agg files
void tft_test2( tft_display_t *tft_display);
void tft_test3( tft_display_t *tft_display);





#ifdef __cplusplus
}
#endif



