
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

  // static input property of the environment
  // does not really belon in mode. mode has values dependent.
  uint32_t line_freq;


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


void data_update_new_reading(data_t *data, uint32_t spi, bool verbose);


// could create and add to data/cal.h

void data_cal(
    data_t *data ,
    uint32_t spi,
    _mode_t *mode,
    volatile uint32_t *system_millis,
    void (*yield)( void * ),
    void * yield_ctx
);

// consider rename. this is more model_encode_row_from_counts()) - according to the model.
// taking model as first arg

/*
  rename name row_to_matrix() to   model_adc_counts_to_m() or similar.
  and make the model the first arg.

*/

MAT * run_to_matrix( uint32_t clk_count_mux_neg, uint32_t clk_count_mux_pos, uint32_t clk_count_mux_rd, unsigned model, MAT * out);

MAT * m_calc_predicted( const MAT *b, const MAT *x, const MAT *aperture);


bool data_flash_repl_statement( data_t *data, const char *cmd);






