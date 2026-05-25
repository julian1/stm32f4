
#pragma once


#include <stdbool.h>

#include <device/spi-fpga0-reg.h>     // for reg_sr_t



typedef struct cal_t cal_t;
typedef struct spi_t spi_t;
typedef struct range_t range_t;
typedef struct data_t data_t;
typedef struct ranging_t ranging_t;
typedef struct _mode_t _mode_t;




// consider move to decode.c
#define DECODE_MAGIC 123


/*
  TODO
  review.
  decode could just about be typed on app....
  instead than passing all this stuff

  only state. are the debug/print control stuff.

*/

typedef struct decode_t
{
  uint32_t    magic;

  // should all be const
  spi_t             *spi ;
  const cal_t       *cal;

  // needed to decode
  // but issue is that the mode - can be out of sequence.
  const _mode_t    *mode;

  // only for printing/formatting.
  const ranging_t   *ranging;   // why?

  uint32_t          *line_freq;


  /*
    these fields are the important persistent state
  */
/*
  // persist...  for AZ. from last reading
  uint32_t adc_clk_count_refmux_pos_hi;
  uint32_t adc_clk_count_refmux_neg_hi;
*/
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
  uint32_t        *line_freq
);


bool decode_repl_statement( decode_t *decode,  const char *cmd);

void decode_update_data( decode_t *decode, data_t *data);




