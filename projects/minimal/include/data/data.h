
/*
  rename measurement?

  localize data handling, and keep this structure opaque in app_t.

*/

#pragma once



#include <stdbool.h>
#include <mesch12b/matrix.h>   // MAT


typedef struct _mode_t _mode_t;

typedef struct data_t
{
  /*
  // TODO move to own file.
  // the MAT structures are annoying to deal with, cannot be easily opaquely prototyped.
  */

  uint32_t magic;

  // TODO reanme adc_valid . eg. same identifier / meaning as fpga code.
  // could also put flags/ for adc state in the status register. eg. the monitor pins.
  volatile bool  adc_measure_valid;

  bool adc_measure_valid_missed; // could be made a count


  unsigned verbose;     // treated as bool at the moment

  /////////////////
  unsigned model_cols; // to use.

  MAT *b;           // cal_b or model_b ?

  //////////////////

  // maybe change name  sa_buffer  acquis_buffer  meas_buffer.
  // change name sa_buffer
  MAT  *sa_buffer;

  // overflow index. rename _overflow, _wraparound
  unsigned sa_count_i;    /* has two needs.
                                   - for modulo indexing sa_buffer
                                    - determining when to stop.
                                    - actually we - may want to stamp - from the fpga .
                                    // wrap around can be handled. if == MAX( ) .
                            */


  double   hi;        // for az mode. the last hi measure TODO change name az_hi. and az_lo ?
  double   lo[2];   // for az mode. the last two lo signals.

} data_t;


#define DATA_MAGIC 123


void data_init ( data_t *);
void data_rdy_interupt( data_t *data);    // handler setup in app context.

void data_update(data_t *data, uint32_t spi );



// could move to data/cal.h
// can pass the yield function here, down from app,
// but note - the yield should not process adc raw data / counts.




void data_cal( data_t *data , uint32_t spi, _mode_t *mode,  volatile uint32_t *system_millis   /* void (*yield)( void * ) */ );



// consider rename. this is more model_encode_row_from_counts()) - according to the model.
// taking model as first arg


MAT * run_to_matrix( uint32_t clk_count_mux_neg, uint32_t clk_count_mux_pos, uint32_t clk_count_mux_rd, unsigned model, MAT * out);



