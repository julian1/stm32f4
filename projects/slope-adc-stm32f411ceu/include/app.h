

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
// void ctrl_set_pattern( uint32_t pattern );
void ctrl_set_aperture( uint32_t aperture);
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

// EXTR.

// we can read the params from the device......... NEAT.
// but use a different structure, and only do it occasionally.


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
  // uint32_t count_flip;
  uint32_t clk_count_rundown;
  // rundown_dir.

  // really don't have to
  uint32_t meas_count;



  /*
    if using pattern controller.
    but probably better to read into a separate params like structure.
  */
  uint32_t himux_sel_last;

  // parameters used to obtain the reading.
  // 
  // Param   param_last;
};



typedef struct Run  Run;


/* matrix_to_run is just appending information?

  Or just pass param and run around separately.

*/

struct Run2
{
  Run   *run;
  Param *param;    // contains hires_mux

  unsigned n;

  MAT   *xs;
  MAT   *aperture;
};


typedef struct Run2  Run2;





// being able to read config. or override is very good.
// eg. fpga can still be the authoritative source
void param_read( Param *param);
void param_report( Param *param);

/*
  Eg. if using the pattern controller, to change the parameters
  it's only four variables. at most.
  just recording himux_sel_last would allow pattern controller, to do fairly usef


*/

void param_read_last( Param *param);



void run_read( Run *run );

void run_report( Run *run);

MAT * run_to_matrix( Param *param, Run *run, MAT * out );

// MAT * run_to_aperture( Run *run, MAT * out);


// where should this code go. it doesn't belong in app.
// should be a better name
//
MAT * calc_predicted( MAT *b, MAT *x, MAT *aperture);


// has to be defined
// typedef struct  CBuf ;


typedef struct app_t app_t;




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


void update_console_cmd(app_t *app);


// change name do_runs. do_meas ?
// unsigned collect_obs( app_t *app, /*Params *params,*/ unsigned row, unsigned discard, unsigned gather, MAT *x);
// void collect_obs( app_t *app, unsigned discard_n, unsigned gather_n, unsigned *row, double y_, MAT *xs, MAT *aperture,  MAT *y);
// void collect_obs( app_t *app, unsigned discard_n, unsigned gather_n, unsigned *row, MAT *xs);
// void collect_obs( app_t *app, Param *param, unsigned discard_n, unsigned gather_n, unsigned *row, MAT *xs);

// void collect_obs( app_t *app, Param *param, unsigned discard_n, unsigned gather_n, unsigned *row, MAT *xs,  unsigned *himux_sel_last, unsigned himux_sel_last_n );

void collect_obs( app_t *app, unsigned discard_n, unsigned gather_n, unsigned *row,  Run2 *run2 ); 

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





