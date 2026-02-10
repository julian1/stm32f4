
/*
  rename measurement?

  localize data handling, and keep this structure opaque in app_t.

*/

#pragma once



#include <stdbool.h>
#include <mesch12b/matrix.h>   // MAT


typedef struct _mode_t _mode_t;
typedef struct data_t data_t;
typedef struct devices_t devices_t;
typedef struct gpio_t gpio_t;





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


  // TODO rename to adc_interupt.
  // consider moving to app.

  // TODO reanme adc_valid . eg. same identifier / meaning as fpga code.
  // could also put flags/ for adc state in the status register. eg. the monitor pins.
  volatile bool  adc_interupt_valid;

  bool adc_interupt_valid_missed; // could be made a count

  // top level concept.
  // unsigned verbose;     // treated as bool at the moment

  /////////////////

  unsigned model_id_to_load;
  /*
    perhaps remove. since only used to produce the cal.
    so just pass as arg when cal process is called
    - except can be used to encode model representatino if have two models with same number of coefficients
  */
  unsigned model_id;
  unsigned model_spec; // to use.

  MAT *model_b;           // consider rename cal_b or model_b ?

  double model_sigma_div_aperture;    // stderr() of the regression..  maybe rename model_stderr change name model_sigma_div_aperture?

  //////////////////

#if 0
  /*
  - readings stored by sequence_idx.  which works very well for calculating output for the different modes.
  */
  // better name,  data/ vals? reading
  double reading[  4 ] ;
  double reading_last[  4 ] ;

#endif

  /////////////////

  /* the final result
    issue whether we should store this here.
    or else call a continuation.
  ------
    if we maintain the stats buffer here, then we should include this value.
  */



  double (*handler_computed_val)( void *ctx, double val, uint32_t status);
  void    *ctx_computed_val;




  double computed_val ;
  // used to communicate between adc reading and display of adc data.
  uint32_t adc_status;
  uint32_t adc_clk_count_mux_sig;


  /*
      Should we use a buffer here.   A simple double float array might be easier.
      And we can compute means(), stddev() on it ok.

      - The thing we do with truncate_rows(),  rather than setting an index. is it adding complexity?
      When we want to substitute - the buffer.

      - could also just have a function pointer, ctx.    new_measurement ( fp, ctx. double val )
  */

  // data buffer
  MAT  *buffer;

  // MAT  *buffer2; second buffer.

  //  name readinx_idx
  uint32_t buffer_idx;    /* has two needs.
                                   - for modulo indexing sa_buffer
                                    - determining when to stop.
                                    - actually we - may want to stamp - from the fpga .
                                    // wrap around can be handled. if == MAX( ) .
                            */


  bool show_counts;
  bool show_stats;
  bool show_extra;


} data_t;


#define DATA_MAGIC 123


void data_init ( data_t *);

// void data_reading_reset( data_t *data );
void data_reset( data_t * data );


// void data_rdy_interupt( data_t *data);    // handler setup in app context.


typedef struct interrupt_t  interrupt_t;
void data_rdy_interupt( data_t *data, interrupt_t *);    // handler setup in app context.

void data_rdy_clear( data_t *data);


// better name process reading.
// void data_update_new_reading(data_t *data, uint32_t spi);


typedef struct spi_t spi_t ;

void data_update_new_reading2(data_t *data, spi_t *spi_fpga0/*, bool verbose*/);


void data_cal(

    data_t    *data,
    devices_t *devices,
    _mode_t   *mode,
    unsigned  model_spec,

    // app stuff
    gpio_t    *gpio_trigger_internal,
    volatile uint32_t *system_millis,
    void      (*yield)( void * ),
    void      *yield_ctx
);


void data_cal2(

    data_t    *data,
    devices_t *devices,
    _mode_t   *mode,
    unsigned  model_spec,

    // app stuff
    gpio_t    *gpio_trigger_internal,
    volatile uint32_t *system_millis,
    void      (*yield)( void * ),
    void      *yield_ctx
);




// consider rename. this is more model_encode_row_from_counts()) - according to the model.
// taking model as first arg

/*
  rename name row_to_matrix() to   model_adc_counts_to_m() or similar.
  and make the model the first arg.

*/

unsigned model_spec_cols( unsigned model_spec );

MAT * run_to_matrix( uint32_t clk_count_mux_neg, uint32_t clk_count_mux_pos, uint32_t clk_count_mux_both, unsigned model, MAT * out);

MAT * m_calc_predicted( const MAT *b, const MAT *x, const MAT *aperture);


bool data_flash_repl_statement( data_t *data, const char *cmd);


bool data_repl_statement( data_t *data,  const char *cmd );

// void data_print_slope_b_detail( unsigned aperture, double slope_b );

void data_cal_show( data_t *data );

#if 0
char * seq_mode_str( uint8_t sample_seq_mode, char *buf, size_t n  );

#endif


// handler/catcher.
// double data_sa_simple_computed_val( void *ctx, double val, uint32_t status);

// double data_sa_simple_computed_val( void *ctx, double val, reg_sr_t status);


