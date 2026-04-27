
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
typedef struct buffer_t buffer_t;



#define TFT_DISPLAY_MAGIC 1827218

typedef struct display_tft_t display_tft_t;

struct display_tft_t
{

  uint32_t  magic;

  tft_t     *tft;
  buffer_t  *buffer;

  int       page;

  // TODO
  bool      page_ready; // for flipping

  // int count;

  // only to time rendering
  // use a dedicated up counter
  volatile uint32_t *system_millis;

  /* if manage the update field here if want, instead of app_t.
    this way, the repl can be localized. and typed on display_tft_t rather than app_t
  */
  /*
    this interface is common for tft, vfd, and display test code.
    and even the repl output. and app.
    could factor into a separate interface. but not clear how useful
  */
  void (*update_data)( display_tft_t *, const data_t *data);

  // no reason to make thi
  // void (*update)( display_tft_t *);
  // void (*update_500ms)( display_tft_t *);

};


// accessors
void display_tft_update_data( display_tft_t *display_tft, const data_t *data);
// no reason to make polymorphic unless clear we need
void display_tft_update( display_tft_t *display_tft);
void display_tft_update_500ms( display_tft_t *display_tft);


void display_tft_init( display_tft_t *display_tft,  tft_t *tft, buffer_t *buffer, volatile uint32_t *system_millis);



bool display_tft_repl_statement( display_tft_t *,  const char *cmd);


// need to communicate between agg files
// these are only called from display_tft_repl_statement()
void tft_test2( display_tft_t *display_tft);
void tft_test3( display_tft_t *display_tft);
void tft_test4( display_tft_t *display_tft);
void tft_test5( display_tft_t *display_tft);
void tft_test6( display_tft_t *display_tft);





#ifdef __cplusplus
}
#endif



