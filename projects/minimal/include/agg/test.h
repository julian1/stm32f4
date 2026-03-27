
#pragma once


#ifdef __cplusplus
extern "C" {
#endif


/*
  not sure this sturct is worthwhile. but simplifies all argument passing..

  need interface between c++ and c. for
  and most agg code, want to be called on every app_update().

  struct - simplifies the calling interface for test code.
  although not sure if really needed

  ---
  update() should be called from app_update() eg. main loop.
*/


#include <stdint.h>
#include <stdbool.h>



typedef struct tft_t tft_t;



#define AGG_TEST_MAGIC 1827218

typedef struct agg_test_t agg_test_t;

struct agg_test_t
{

  uint32_t  magic;

  tft_t     *tft;
  int       page;

  // int count;

  // volatile int32_t *millis_yield_countdown;    // make count down
  volatile uint32_t *system_millis;

  /* if manage the update field here if want, instead of app_t.
    this way, the repl can be localized. and typed on agg_test_t rather than app_t
  */
  void (*update)( agg_test_t *);

};


void agg_test_init( agg_test_t *agg_test,  tft_t *tft, volatile uint32_t *system_millis);
void agg_test_update( agg_test_t *agg_test);
bool agg_test_repl_statement( agg_test_t *,  const char *cmd);


void agg_test2( agg_test_t *agg_test);
void agg_test3( agg_test_t *agg_test);




#ifdef __cplusplus
}
#endif



