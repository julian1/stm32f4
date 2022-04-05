

#pragma once


#include "cbuffer.h"
// #include "cal.h"





// so it doesn't matter where it goes
// general,mix start 0
#define REG_LED                 7
#define REG_TEST                8

// control/param vars
// we don't necessarily need to expose all these
// just set once in pattern controller.
// modulation control parameters, start 30.

#define REG_CLK_COUNT_RESET_N   10
#define REG_CLK_COUNT_FIX_N     11
#define REG_CLK_COUNT_VAR_N     13
#define REG_CLK_COUNT_APER_N_LO 15
#define REG_CLK_COUNT_APER_N_HI 16

#define REG_USE_SLOW_RUNDOWN    17
#define REG_HIMUX_SEL           18
#define REG_STATE               19
#define REG_RESET               20   // hold modulation in reset.


// meas/run vars
// these are all last
#define REG_COUNT_UP            30
#define REG_COUNT_DOWN          31
#define REG_COUNT_TRANS_UP      32
#define REG_COUNT_TRANS_DOWN    33
#define REG_COUNT_FIX_UP        34
#define REG_COUNT_FIX_DOWN      35
#define REG_CLK_COUNT_RUNDOWN   37


/*
// treat as param variable
#define REG_LAST_HIMUX_SEL           40 // what was being muxed for integration. sig, azero, acal .
#define REG_LAST_CLK_COUNT_FIX_N     41
#define REG_LAST_CLK_COUNT_VAR_N    42
// #define REG_LAST_CLK_COUNT_VAR_NEG_N 43
#define REG_LAST_CLK_COUNT_APER_N_LO 44
#define REG_LAST_CLK_COUNT_APER_N_HI 45
*/

#define REG_MEAS_COUNT          50



#define HIMUX_SEL_SIG_HI      0b1110
#define HIMUX_SEL_REF_HI      0b1101
#define HIMUX_SEL_REF_LO      0b1011
#define HIMUX_SEL_ANG         0b0111


//
#define STATE_RESET_START    0    // initial stat
#define STATE_RESET          1


// this isn't very good. because displayed nplc
// will be constructive.
uint32_t nplc_to_aper_n( double nplc );
double aper_n_to_nplc( uint32_t aper_n);
double aper_n_to_period( uint32_t aper_n);



// do it directly
// void ctrl_set_pattern( uint32_t spi, uint32_t pattern );
uint32_t ctrl_get_state( uint32_t spi );

void ctrl_set_aperture( uint32_t spi, uint32_t aperture);
uint32_t ctrl_get_aperture( uint32_t spi );


void ctrl_set_mux( uint32_t spi, uint32_t mux );
uint32_t ctrl_get_mux( uint32_t spi);


// current....
void ctrl_set_var_n( uint32_t spi, uint32_t val);
uint32_t ctrl_get_var_n( uint32_t spi );

void ctrl_set_fix_n( uint32_t spi, uint32_t val);
uint32_t ctrl_get_fix_n( uint32_t spi );




void ctrl_reset_enable( uint32_t spi);
void ctrl_reset_disable(uint32_t spi);


char * himux_sel_format( uint32_t mux ); // change name it is any himux.






struct Param
{
  /*
    must have params to compute the clk sums, in param_run_to_matrix_t
    therefore need to pass around, or else read off the mcu.
  */
  // the pattern controller may change on its own - so should read for *each* run.
  uint32_t clk_count_aper_n;   // aperture.
  uint32_t clk_count_fix_n;
  uint32_t clk_count_var_n;

  // for auto-zero
  uint32_t himux_sel;
};



typedef struct Param Param;








struct Run
{
  uint32_t count_up;
  uint32_t count_down;
  // we don't have to read some of these
  uint32_t count_trans_up;
  uint32_t count_trans_down;
  uint32_t count_fix_up;
  uint32_t count_fix_down;
  uint32_t clk_count_rundown;
  // rundown_dir.

  // really don't need to read/use this
  // uint32_t meas_count;

};



typedef struct Run  Run;



// sould pass stream

void ctrl_param_read( uint32_t spi, Param *param);
// void ctrl_param_read_last( uint32_t spi, Param *param);
void param_report( const Param *param /* , FILE *f*/ );  // rename param_report??

void ctrl_run_read( uint32_t spi, Run *run );
void run_report( const Run *run  /*, FILE *f*/);    // rename run_report
void run_report_brief( const Run *run );





typedef struct app_t app_t;


//

typedef struct Cal Cal;


#define CMD_BUF_SZ  100

struct app_t
{
  CBuf console_in;
  CBuf console_out;


  uint32_t led_tick_count;
  uint32_t led_blink_interval;

  uint32_t  spi;  // spi device. maybe rename ctrl_spi

  volatile bool data_ready ;


  /*
    the command processor (called in deep instack) can configure this. ok.
    continuation function.
    so we can set this anywhere (eg. in command processor). and control will pass.
    and we can test this.
    - allows a stack to run to completion even if early termination -  to clean up resources.
    ------------
    don't really need this. just use an exit_flag, to communicate;
  */
  void *continuation_ctx;
  void (*continuation_f)(void *);

  // TODO should be using CString for this ? i think.
  // need to initialize
  char  cmd_buf[CMD_BUF_SZ ];
  unsigned cmd_buf_i;


  // issue is using null to indcate present.
  Cal     *cal[10];

  unsigned cal_idx;    // current idx; // rename cal_idx.

  unsigned cal_model_xcols;  // regression model specification. 3 or 4.




  // need to intialize
  unsigned buffer_i;
  MAT     *buffer;


  // need to intialize
  unsigned stats_buffer_i;
  MAT     *stats_buffer;


  MAT     *last;

} ;


typedef struct app_t app_t;



// app_loop1
void app_loop1(app_t *app );
void app_loop2(app_t *app );
void app_loop3(app_t *app );
void app_loop4(app_t *app );


void app_loop22 ( app_t *app );

// MAT * param_run_to_matrix( const Param *param, const Run *run, MAT * out );   // rename param_param_run_to_matrix()

MAT * param_run_to_matrix( const Param *param, const Run *run, unsigned model, MAT * out );

MAT * m_calc_predicted( const MAT *b, const MAT *x, const MAT *aperture);

void app_update_console_cmd(app_t *app);
// void app_update_led(app_t *app);
void app_update( app_t * app );


void app_simple_sleep( app_t * app, uint32_t period );
void app_loop_dispatcher(app_t *app);

void app_spi1_interupt(app_t *app );

double app_simple_read( app_t *app);
void app_voltage_source_set( app_t *app, double value );



