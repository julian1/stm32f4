

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
#define REG_COUNT_FLIP          36

#define REG_CLK_COUNT_RUNDOWN   37


#define REG_CLK_COUNT_MUX_NEG   40
#define REG_CLK_COUNT_MUX_POS   41
#define REG_CLK_COUNT_MUX_RD    42


#define REG_USE_FAST_RUNDOWN    43



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



void ctrl_set_fast_rundown( uint32_t spi, uint32_t val);
uint32_t ctrl_get_fast_rundown( uint32_t spi );





void ctrl_reset_enable( uint32_t spi);
void ctrl_reset_disable(uint32_t spi);


char * himux_sel_format( uint32_t mux ); // change name it is any himux.






struct Param
{
  /*
    - state that controls modulatoin on the fpga.
    - TODO rename ModulationParam? perhaps. or just Mod.
    - not clear that aperture should be here. but useful to indicate what was used during calibration.
    - can/should add the two reset periods
  */
  // may
  uint32_t clk_count_aper_n;   // aperture. really not sure this should be here.
                                // perhaps useful to indicate aperture was used for calibration.

  uint32_t clk_count_fix_n;
  uint32_t clk_count_var_n;

  // control over reset periods would be useful. if increase reading per second. on sub 1NPLC.
  // but not sure should go here.


  //
  // uint32_t himux_sel;
  // unused. but avoid disturbing serialization
  uint32_t old_serialization ;
};



typedef struct Param Param;

void ctrl_param_read( uint32_t spi, Param *param);
void ctrl_param_write( uint32_t spi, Param *param);
bool param_equal( Param *param_a , Param *param_b);

// void ctrl_param_read_last( uint32_t spi, Param *param);
void param_show( const Param *param /* , FILE *f*/ );  // rename param_show??








struct Run
{
  /*
    we could avoid reading. perhaps with verbosity flag.

  */
  uint32_t count_var_up;
  uint32_t count_var_down;

  uint32_t count_fix_up;
  uint32_t count_fix_down;

  uint32_t count_pos_trans;   // pos switch on transitions.
  uint32_t count_neg_trans;

  uint32_t count_flip;


  // TODO REMOVE
  // run->clk_count_rundown  = spi_ice40_reg_read(spi, REG_CLK_COUNT_RUNDOWN );
  /// uint32_t clk_count_rundown;

  /*
    we need a stamp_id;  to ensure we are not missing anything.
  */


  uint32_t clk_count_mux_neg ;
  uint32_t clk_count_mux_pos ;
  uint32_t clk_count_mux_rd ;


  // rundown_dir.

  // really don't need to read/use this
  // uint32_t meas_count;

  // perhaps modified by pattern controller.
  // for auto-zero.  Should be recorded in run... I think.
  uint32_t himux_sel;

};



typedef struct Run  Run;



// sould pass stream

void ctrl_run_read( uint32_t spi, Run *run, bool verbose);


void run_show( const Run *run, bool verbose /* FILE *f */ );






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

  // volatile bool data_ready ;


  /*
    the command processor (called in deep instack) can configure this. ok.
    continuation function.
    so we can set this anywhere (eg. in command processor). and control will pass.
    and we can test this.
    - allows a stack to run to completion even if early termination -  to clean up resources.
    ------------
    don't really need this. just use an exit_flag, to communicate;
  */
  // void *continuation_ctx;
  // void (*continuation_f)(void *);

  bool halt_func;   // set this, to exit whatever loop we are running.



  // TODO should be using CString for this ? i think.
  // need to initialize
  char  cmd_buf[CMD_BUF_SZ ];
  unsigned cmd_buf_i;


  // issue is using null to indcate present.
  Cal     *cal[ 20 ];

  unsigned cal_slot_idx;    // current idx; // rename cal_slot_idx.

  unsigned cal_id_max;      // max cal id known.



  // maybe change name  buffer_measure ?
  unsigned buffer_i;
  MAT     *buffer;


  unsigned stats_buffer_i;
  MAT     *stats_buffer;


  MAT     *last;

  // voltage source 2.
  uint8_t   spi_4094_reg;
  uint32_t  spi_voltage_source;

  bool      verbose;

  bool      block;  // used to pause for user input
} ;


typedef struct app_t app_t;


void app_update_led(app_t *app);

void calc_cal( app_t *app,  MAT *y, MAT *xs, MAT *aperture  );

// app_loop1
void app_loop1(app_t *app );
void app_loop2(app_t *app );
void app_loop3(app_t *app );

void app_loop4 ( app_t *app,  unsigned cal_slot_a,  unsigned cal_slot_b  );


void app_loop22 ( app_t *app );


MAT * run_to_matrix(  const Run *run, unsigned model, MAT * out );

MAT * m_calc_predicted( const MAT *b, const MAT *x, const MAT *aperture);

void app_update_console_cmd(app_t *app);
// void app_update_led(app_t *app);
void app_update( app_t * app );


void app_simple_sleep( app_t * app, uint32_t period );
void app_loop_main(app_t *app);

// rename default_handler()?
void app_spi1_default_interupt(app_t *app );

double app_simple_read( app_t *app);

void app_voltage_source_1_set( app_t *app, double value );
void app_voltage_source_2_set( app_t *app, double value );



