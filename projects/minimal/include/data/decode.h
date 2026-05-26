
#pragma once


#include <stdbool.h>



typedef struct cal_t cal_t;
typedef struct spi_t spi_t;
typedef struct range_t range_t;
typedef struct data_t data_t;
typedef struct _mode_t _mode_t;
typedef struct ranging_t ranging_t ;
typedef struct environment_t environment_t;




// consider move to decode.c
#define DECODE_MAGIC 123


/*
  TODO
  review.
  decode could just about be typed on app....
  instead than passing all this stuff
  only state. are the debug/print control stuff.

  we used to type on app_t.
  then moved.
  but this module does less now the decode strategy is stored on/against the mode.

  the only local state. is related to the show flags.

*/

typedef struct decode_t
{
  uint32_t    magic;

  // should all be const
  spi_t             *spi ;
  const cal_t       *cal;

  // needed to decode
  // but potential issue with state synchronization, of mode is being updated
  const _mode_t    *mode;

  // only for printing/formatting.
  const ranging_t   *ranging;

  const environment_t *environment;

  ///////////////////////

  bool show_counts;
  bool show_reading;
  // bool show_sum;
  // bool show_ratio;


} decode_t;



void decode_init(
  decode_t        *decode,
  spi_t           *spi,
  const cal_t     *cal,
  const _mode_t   *mode,

  const ranging_t *ranging,
  const environment_t *environment
);


bool decode_repl_statement( decode_t *decode,  const char *cmd);

void decode_update_data( decode_t *decode, data_t *data);




