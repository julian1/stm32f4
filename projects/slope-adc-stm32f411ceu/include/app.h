

#pragma once


#include "cbuffer.h"





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
// #define REG_CLK_COUNT_VAR_N  12
#define REG_CLK_COUNT_VAR_POS_N 13
#define REG_CLK_COUNT_VAR_NEG_N 14
#define REG_CLK_COUNT_APER_N_LO 15// aperture. rename?
#define REG_CLK_COUNT_APER_N_HI 16
#define REG_USE_SLOW_RUNDOWN    17
#define REG_HIMUX_SEL           18
#define REG_PATTERN             19
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


// treat as param variable
#define REG_LAST_HIMUX_SEL           40 // what was being muxed for integration. sig, azero, acal .
#define REG_LAST_CLK_COUNT_FIX_N     41
#define REG_LAST_CLK_COUNT_VAR_POS_N 42
#define REG_LAST_CLK_COUNT_VAR_NEG_N 43
#define REG_LAST_CLK_COUNT_APER_N_LO 44
#define REG_LAST_CLK_COUNT_APER_N_HI 45


#define REG_MEAS_COUNT          50



#define HIMUX_SEL_SIG_HI      0b1110
#define HIMUX_SEL_REF_HI      0b1101
#define HIMUX_SEL_REF_LO      0b1011
#define HIMUX_SEL_ANG         0b0111




// this isn't very good. because displayed nplc
// will be constructive.
uint32_t nplc_to_aper_n( double nplc );
double aper_n_to_nplc( uint32_t int_n);
double aper_n_to_period( uint32_t int_n);



// do it directly
void ctrl_set_pattern( uint32_t pattern );
void ctrl_set_aperture( uint32_t aperture);
uint32_t ctrl_get_aperture( /*spi */ void );



void ctrl_set_mux( uint32_t mux );

uint32_t ctrl_get_mux( void /* uint32_t spi */);

void ctrl_reset_enable( void );
void ctrl_reset_disable(void);








struct Param
{
  /*
    must have params to compute the clk sums, in run_to_matrix_t
    therefore need to pass around, or else read off the mcu.
  */
  // the pattern controller may change on its own - so should read for *each* run.
  uint32_t clk_count_aper_n;   // aperture.
  uint32_t clk_count_fix_n;
  uint32_t clk_count_var_pos_n;

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

  // really don't have to
  uint32_t meas_count;

};



typedef struct Run  Run;





void param_read( Param *param);
void param_report( Param *param);



void param_read_last( Param *param);



void run_read( Run *run );

void run_report( Run *run);

MAT * run_to_matrix( Param *param, Run *run, MAT * out );


MAT * calc_predicted( MAT *b, MAT *x, MAT *aperture);



typedef struct app_t app_t;




#define CMD_BUF_SZ  100

typedef struct app_t
{
  CBuf console_in;
  CBuf console_out;

  bool data_ready ;


  /*
    the command processor (called in deep instack) can configure this. ok.
    continuation function.
    so we can set this anywhere (eg. in command processor). and control will pass.
    and we can test this.
    - allows a stack to run to completion even if early termination -  to clean up resources.
  */
  void *continuation_ctx;
  void (*continuation_f)(void *);

  // TODO should be using CString for this ? i think.
  // need to initialize
  char  cmd_buf[CMD_BUF_SZ ];
  unsigned cmd_buf_i;



  MAT     *b;       // calebration coefficients

  // need to intialize
  unsigned buffer_i;
  MAT     *buffer;


  // need to intialize
  unsigned stats_buffer_i;
  MAT     *stats_buffer;




} app_t;


// renam app_update_console_command
void app_update_console_cmd(app_t *app);

void app_led_update(app_t *app);


// loop1
void loop1(app_t *app );
void loop2(app_t *app );
void loop3(app_t *app );


// loop3
void permute(app_t *app, MAT *b);


/*
  should be able to avoid this . set once, and then query the matrix.
  eg. m_cols( m);
*/
#define X_COLS   4

// #define X_COLS   3





