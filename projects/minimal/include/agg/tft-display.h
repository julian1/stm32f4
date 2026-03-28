
#pragma once


#ifdef __cplusplus
extern "C" {
#endif


/*
  needed as interface between c++ and c. code

  most agg test code, wants to be called on app_update().
  but real display on app_update_data()

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

  // rename *tick_up;   could be dedicated.  tft_display_tick_up. - only for performance test
  volatile uint32_t *system_millis;

  /* if manage the update field here if want, instead of app_t.
    this way, the repl can be localized. and typed on tft_display_t rather than app_t
  */
  /*
    this interface is common for tft, vfd, and display test code.
    and even the repl output. and app.
    could factor into a separate interface. but not clear how useful
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
// these are only called from tft_display_repl_statement()
void tft_test2( tft_display_t *tft_display);
void tft_test3( tft_display_t *tft_display);





#ifdef __cplusplus
}
#endif



