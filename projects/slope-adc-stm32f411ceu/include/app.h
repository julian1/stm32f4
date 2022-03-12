

#pragma once


#include "cbuffer.h"





#define REG_LED               7
#define REG_TEST              8

// run parameters
#define REG_COUNT_UP          10
#define REG_COUNT_DOWN        11
#define REG_COUNT_TRANS_UP    12
#define REG_COUNT_TRANS_DOWN  13
#define REG_COUNT_FIX_UP      14
#define REG_COUNT_FIX_DOWN    15
#define REG_COUNT_FLIP        16  // deprecated
#define REG_CLK_COUNT_RUNDOWN 17
#define REG_RUNDOWN_DIR       18  // deprecated


// modulation control parameters
#define REG_CLK_COUNT_INIT_N  30
#define REG_CLK_COUNT_FIX_N   31
// #define REG_CLK_COUNT_VAR_N   32
#define REG_CLK_COUNT_VAR_POS_N   37
#define REG_CLK_COUNT_VAR_NEG_N   38


#define REG_CLK_COUNT_APER_N_LO 33    // aperture. rename?
#define REG_CLK_COUNT_APER_N_HI 34

#define REG_USE_SLOW_RUNDOWN  35
#define REG_HIMUX_SEL         36

#define REG_MEAS_COUNT        40




// eg. bitwise, active lo.  avoid turning on more than one.
// although switch has 1.5k impedance so should not break
#define HIMUX_SEL_SIG_HI      (0xf & ~(1 << 0))
#define HIMUX_SEL_REF_HI      (0xf & ~(1 << 1))
#define HIMUX_SEL_REF_LO      (0xf & ~(1 << 2))
#define HIMUX_SEL_ANG         (0xf & ~(1 << 3))





struct Params
{
  // fix counts. are setup and written in
  // uint32_t reg_led ;
  uint32_t clk_count_aper_n;   // aperture.

  uint32_t clk_count_init_n ;
  uint32_t clk_count_fix_n ;
  // uint32_t clk_count_var_n ;
                                    /* ok clk_count_var_n is used for both +ve and -ve var.
                                    BUT. we want to be able to vary pos and neg.
                                    while holding nplc
                                    --
                                    which means I think we want to separate this.
                                  */
  uint32_t clk_count_var_pos_n;
  uint32_t clk_count_var_neg_n;


  uint32_t use_slow_rundown;
  uint32_t himux_sel;

  uint32_t meas_count;
  // number of obs per measurement. is a higher level concept.
};


// this isn't very good. because displayed nplc
// will be constructive.
uint32_t nplc_to_aper_n( double nplc );
double aper_n_to_nplc( uint32_t int_n);
double aper_n_to_period( uint32_t int_n);

typedef struct Params Params;


void params_read( Params * params );
void params_report(Params * params );
bool params_equal( Params *params0,  Params *params1 );
void params_write( Params *params );
void params_set_main( Params *params,  uint32_t clk_count_aper_n, bool use_slow_rundown, uint8_t himux_sel );
// void params_set_extra( Params *params,  uint32_t clk_count_init_n, uint32_t  clk_count_fix_n, uint32_t clk_count_var_n);

void params_set_extra( Params *params,  uint32_t clk_count_init_n, uint32_t  clk_count_fix_n, uint32_t clk_count_var_pos_n, uint32_t clk_count_var_neg_n);


/*
  change name Meas.
  or Measurement.
*/

struct Run
{
  uint32_t count_up;
  uint32_t count_down;
  uint32_t count_trans_up;
  uint32_t count_trans_down;
  uint32_t count_fix_up;
  uint32_t count_fix_down;
  uint32_t count_flip;

  // rundown_dir.

  uint32_t clk_count_rundown;

};

typedef struct Run  Run;

void run_read( Run *run );
void run_report( Run *run );
MAT * run_to_matrix( Params *params, Run *run, MAT * out );


// has to be defined
// typedef struct  CBuf ;


typedef struct app_t app_t;

/*
typedef struct Loop1
{
  app_t   *app;
  MAT     *b;
  Params  params;   // loop structure will be passed by reference anyway.
  unsigned obs;     // how many obs  to average.

} Loop1;

*/


#define CMD_BUF_SZ  100

typedef struct app_t
{
  CBuf console_in;
  CBuf console_out;

  bool data_ready ;


  // FBuf      measure_rundown;
  // Loop1     loop1;

  Params  params;   // loop structure will be passed by reference anyway.
  MAT     *b;       // calebration coefficients
  unsigned obs;     // how many obs  to average.



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


} app_t;


unsigned collect_obs( app_t *app, Params *params, unsigned row, unsigned discard, unsigned gather, MAT *x);
void update_console_cmd(app_t *app);





// loop1
void loop1(app_t *app );
void loop2(app_t *app );

// loop2
// void cal_collect_obs(app_t *app, MAT *x, MAT *y );
// MAT * calibrate( app_t *app);

// loop3
void permute(app_t *app, MAT *b);

#define X_COLS   3




