

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
#define REG_RESET               20// hold modulation in reset.


// meas/run vars
#define REG_COUNT_UP            30  // need to start at 30
#define REG_COUNT_DOWN          31
#define REG_COUNT_TRANS_UP      32
#define REG_COUNT_TRANS_DOWN    33
#define REG_COUNT_FIX_UP        34
#define REG_COUNT_FIX_DOWN      35
#define REG_CLK_COUNT_RUNDOWN   37


#define REG_MEAS_COUNT          40

// these are output registers dependent upon the pattern used.
// #define REG_MEAS_HIMUX_SEL      41      // what was being muxed for integration. sig, azero, acal .
// #define REG_MEAS_VAR_POS_N      42      // we don't need this...






#define HIMUX_SEL_SIG_HI      0b1110
#define HIMUX_SEL_REF_HI      0b1101
#define HIMUX_SEL_REF_LO      0b1011
#define HIMUX_SEL_ANG         0b0111






/*
  verilog
  possible to use an array -
  eg.  just with an index.

  we really want something simple for azero.
  that doesn't get in the way for other operations.

  to select the himux_sel input.   for each input.
  but we potentially want to var_pos also.
  --------

  - could just be a simple state machine. that assigns vars.
  - if 0 , then make hires mux sig-in.
  - if 1,

  - this ought to work for

  eg. himux_sel  is an array. and a looping state machine just updates values.
  AND. we have a counter. so we can set it to one. to make it easy.

  OR  slightly more complicated. specify the count. no. maybe

           input      n
  slot 0   sel sig    1
  slot 1   sel ref-lo 1
  return.

  - could almost have a little programming language quite simply. with cmds. and program counter.
  no. doesn't allow an auto-gain.
  --------------------

  NO.
    we just want to set up some fixed patterns.  nothing more complicated.
    we don't need other controls.

  pattern 1.   just sig.
  pattern 2.   azero alternate sig and ref-lo for .
  pattern 3.   azero alternate sig and ref-lo, and every 10 sample ref hi.
  pattern 4.   alternate switching var_pos between two values.

  this is much simpler.

  so we have separate state machines - and then choose/mux the state machine we want.

  ----


*/

/*
// eg. bitwise, active lo.  avoid turning on more than one.
// although switch has 1.5k impedance so should not break
#define HIMUX_SEL_SIG_HI      (0xf & ~(1 << 0))
#define HIMUX_SEL_REF_HI      (0xf & ~(1 << 1))
#define HIMUX_SEL_REF_LO      (0xf & ~(1 << 2))
#define HIMUX_SEL_ANG         (0xf & ~(1 << 3))   // 0b0111
*/


#define PATTERN_SIG_HI      0
#define PATTERN_REF_HI      1
#define PATTERN_REF_LO      2
#define PATTERN_ANG         3

#if 0
// Do we really actually care about timing

struct Params
{
  // fix counts. are setup and written in
  // uint32_t reg_led ;
  uint32_t clk_count_aper_n;   // aperture.

#if 0
  uint32_t clk_count_reset_n ;
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
#endif

  uint32_t pattern;

  uint32_t meas_count;
  // number of obs per measurement. is a higher level concept.
};
#endif

// this isn't very good. because displayed nplc
// will be constructive.
uint32_t nplc_to_aper_n( double nplc );
double aper_n_to_nplc( uint32_t int_n);
double aper_n_to_period( uint32_t int_n);

#if 0


/*
  // OK. we need the params - in order to compute the times...
  // Or. maybe we should just read them after the run.
  -----

  EXTR.
    we want to read the var_pos_n etc. after *each* run.
    BECAUSE. we want to allow the pattern controller to permute

*/

typedef struct Params Params;


void params_read( Params * params );
void params_report(Params * params );
bool params_equal( Params *params0,  Params *params1 );
void params_write( Params *params );
void params_set_main( Params *params,  uint32_t clk_count_aper_n, bool use_slow_rundown, uint8_t himux_sel );
// void params_set_extra( Params *params,  uint32_t clk_count_reset_n, uint32_t  clk_count_fix_n, uint32_t clk_count_var_n);

void params_set_extra( Params *params,  uint32_t clk_count_reset_n, uint32_t  clk_count_fix_n, uint32_t clk_count_var_pos_n, uint32_t clk_count_var_neg_n);

#endif

/*
  change name Meas.
  or Measurement.
*/

// do it directly
void ctrl_set_pattern( uint32_t pattern );
void ctrl_set_aperture( uint32_t aperture);
void ctrl_set_mux( uint32_t mux );

void ctrl_enable_reset( void );
void ctrl_disable_reset(void);





struct Run
{
  uint32_t count_up;
  uint32_t count_down;
  // we don't have to read some of these
  uint32_t count_trans_up;
  uint32_t count_trans_down;
  uint32_t count_fix_up;
  uint32_t count_fix_down;
  // uint32_t count_flip;
  uint32_t clk_count_rundown;
  // rundown_dir.

  uint32_t meas_count;

  // the pattern controller may change on its own - so should read for *each* run.
  uint32_t clk_count_aper_n;   // aperture.
  uint32_t clk_count_fix_n;
  uint32_t clk_count_var_pos_n;

};

typedef struct Run  Run;

void run_read( Run *run );
void run_report( Run *run );
MAT * run_to_matrix( /*Params *params,*/ Run *run, MAT * out );


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

  // Params  params;   // loop structure will be passed by reference anyway.
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


unsigned collect_obs( app_t *app, /*Params *params,*/ unsigned row, unsigned discard, unsigned gather, MAT *x);
void update_console_cmd(app_t *app);





// loop1
void loop1(app_t *app );
void loop2(app_t *app );

// loop2
// void cal_collect_obs(app_t *app, MAT *x, MAT *y );
// MAT * calibrate( app_t *app);

// loop3
void permute(app_t *app, MAT *b);

#define X_COLS   4
// #define X_COLS   3




