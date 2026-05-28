

#include <assert.h>
#include <stdio.h>
#include <math.h>   // fabs,
#include <string.h>   // strcmp, memset
#include <stdlib.h>   // malloc               // TODO review


#include <support.h> // str_decode_uint
#include <environment.h>


#include <lib3/format.h>      // str_format_bits()
#include <lib3/util.h>        // UNUSED

#include <mode.h>




/*
  better location for these fucntions
  they are ok here, since they support mode handling.
  although not typed on mode


*/

void adc_aperture_set( adc_state_t *adc, uint32_t u)
{
  adc->p_aperture = u;
}




void sa_trig_delay_set( sa_state_t *sa, uint32_t u)
{
  sa->p_trig_delay = u;
}




void cr_sa_mode_set( reg_cr_t *reg_cr, unsigned u0)
{
  // now the sequencer mode

  // deprecated, and unused for the moment
  assert( 0);

  assert( u0 <=  0b111);

  reg_cr->sa_mode = u0;
}








#include <data/data.h>
#include <data/cal.h>





/*
  EXTR.
  - instead of mallocing, could just make a union structure.
  for differrent representations.
  - then pre-allocate on stack in main.c
  - and pass to mode_t on init()
  same as other objects we create.
  -----------

  - but malloc() is nice. because it completely localizes the decode strategy.
    proper strategy pattern, and can be created anywhere in the codebase for custom behavior.

  - alternatively if we dont want to pre-code the structures using union types -
    then allocate/ and store against the module (eg. like decode,buffer ).

  - ratiometric - difficult for auto-ranging.  because of nominal value.
  - one way to handle.  would just be to take the max of the two readings.
      and use that for auto-ranging.
      else only use 10V range.
  ----------

  - for ratio metric.
      may want to treat as two seperate measurements.   eg. 0,1   and 2,3.  with no oob.

      this make the decode structures basically the same/repeated.
      consider - just nest the second measurement counts.. eg. the same way we handle the oob.
      - the issue is to clear everything on the first time.

    - OR. just maintain pointers to the same shared structure -. for normal,oob, and ratio_snd.
      then can have the same code work.
*/











#define HI_CH1  S1
#define HI_CH2  S3

#define COM_LC  S5
#define LO_CH2  S7

#define LO_STAR S6





typedef struct decode_t
{
  // uint32_t    magic;

  // decode_oob_t    oob;

  // persist...  for AZ. from last reading
  uint32_t adc_refmux_pos_hi;
  uint32_t adc_refmux_neg_hi;

  uint32_t adc_refmux_pos_lo;
  uint32_t adc_refmux_neg_lo;


} decode_t;






static void decode_reset( decode_t *decode)
{
  assert( decode);

  // base case
  // clear state for re/trigger

  printf( "clear ");
  // decode_reset()
  decode->adc_refmux_pos_hi = 0;
  decode->adc_refmux_neg_hi = 0;
  decode->adc_refmux_pos_lo = 0;
  decode->adc_refmux_neg_lo = 0;

}





static void decode_noaz_lo_first( decode_t *decode, data_t *data)
{
  assert( decode);
  assert( data && data->magic == DATA_MAGIC);


  const reg_sr_t status = data->status;
  // const term_t term  = data->term;
  double cal_w          = data->cal->w;
  assert( cal_w);


  // this is the recursive case
  // we do not handle the reset/base case here.


  if( status.sample.idx % 2 == 0) {

    // LO   record counts.
    printf( "lo ");
    decode->adc_refmux_pos_lo     = data->adc_refmux_pos;
    decode->adc_refmux_neg_lo     = data->adc_refmux_neg;
    assert( data->reading_valid == false);
  }
  else {

    // HI convert value
    printf( "hi ");
    data->count_sum =
        ((double) data->adc_refmux_pos      - (cal_w * data->adc_refmux_neg))
      - ((double) decode->adc_refmux_pos_lo - (cal_w * decode->adc_refmux_neg_lo));

    data->reading_valid     = true;
  }
}




static void decode_az_hi_first( decode_t *decode, data_t *data)
{

  assert( decode);
  assert( data && data->magic == DATA_MAGIC);


  const reg_sr_t status = data->status;
  // const term_t term     = data->term;
  double cal_w          = data->cal->w;
  assert( cal_w);


  // this is the recursive case
  // we do not handle the reset/base case here.


  if( status.sample.idx % 2 == 0) {

    // HI.  record counts.
    printf( "hi ");
    decode->adc_refmux_pos_hi     = data->adc_refmux_pos;
    decode->adc_refmux_neg_hi     = data->adc_refmux_neg;
    assert( data->reading_valid == false);
  }
  else {

    // LO convert value
    printf( "lo ");

    // if( decode->adc_refmux_pos_lo) printf( "have prior pos ");
    // if( decode->adc_refmux_neg_lo) printf( "have prior neg ");

    /*
      if have prior LO if available, use average with this Lo.
    */
    double lo_pos =
      decode->adc_refmux_pos_lo
      ? (data->adc_refmux_pos + decode->adc_refmux_pos_lo ) / 2.
      : data->adc_refmux_pos;

    double lo_neg =
      decode->adc_refmux_neg_lo
      ? (data->adc_refmux_neg + decode->adc_refmux_neg_lo ) / 2.
      : data->adc_refmux_neg;

    data->count_sum =
        ((double) decode->adc_refmux_pos_hi - (cal_w * decode->adc_refmux_neg_hi))
      - ((double) lo_pos                    - (cal_w * lo_neg ));


    // record/update LO. for next time
    decode->adc_refmux_pos_lo     = data->adc_refmux_pos;
    decode->adc_refmux_neg_lo     = data->adc_refmux_neg;


    data->reading_valid     = true;
  }
}






typedef struct decode_x_t
{
  uint32_t    magic;

  bool        hi_first;

  decode_t    oob;

  decode_t    normal;

  decode_t    second; // for ratio


} decode_x_t;




#define DECODE_X_MAGIC 23782191

// still need somewhere to store this memory...


static void decode_x_init( decode_x_t *decode, bool hi_first)
{
  memset( decode, 0, sizeof( decode_x_t));
  decode->magic = DECODE_X_MAGIC;

  decode->hi_first = hi_first;

  decode_reset( &decode->oob);
  decode_reset( &decode->normal);
  decode_reset( &decode->second);

}








static void decode_x( decode_x_t *decode, data_t *data)
{
  /*

    functions localize all behavior for hi/lo/ ratiometric handling, needed to output a reading.
    (although final reading needs cal).

    probably don't want strategy detail (hi,lo, ratiometric etc)
    to escape this context.

    note. that az_high_first generalizes decode, without regard to azmux, oob or zgjc.
  */

  assert( decode && decode->magic == DECODE_X_MAGIC);
  assert( data && data->magic == DATA_MAGIC);


  const reg_sr_t status = data->status;
  const term_t term     = data->term;


  if( status.sample.first) {

    // base case/ re-trigger
    decode_reset( &decode->oob);
    decode_reset( &decode->normal);
    decode_reset( &decode->second);
  }


  if( term.oob_aperture) {

    printf( "(oob)      ");
    // delegate to oob conversion handler
    decode_az_hi_first( &decode->oob , data);
    return;
  }


  if( decode->hi_first) {
    printf( "(hi first) ");
    decode_az_hi_first( &decode->normal, data);
  }

  else {
    printf( "(lo first) ");
    decode_noaz_lo_first( &decode->normal, data);
  }


  /*
    if the type is ratio.  then look at the index, to distinguish.
    and ddelegate appropriately  for the normal, or second data

  */


  /*
    consider if want to handle normalization here.

    Yes. for ratio-metric,  because autoranging/ OV detection needs something sensible to display.
    And may want to divide by (data->adc_sigmux * 2);

    // normalized count
    data->count_sum_norm = data->count_sum  / data->adc_sigmux;
  */


}






static void compile_sa_az_hi_first( sa_state_t *sa)
{

  assert( !sa->noaz);

  // clear memory
  _Static_assert( ARRAY_SIZE( sa->terms) ==  4);
  memset( sa->terms, 0, sizeof( sa->terms));


  bool is_ch1 = strcmp( sa->input, "ch1") == 0;
  bool is_ch2 = strcmp( sa->input, "ch2") == 0;
  bool is_0   = strcmp( sa->input, "0") == 0;

  if( is_ch1 || is_ch2 || is_0) {

    /*
      normal az operation.
      HI/input first
      with oob reading included for fast auto-ranging.
    */

    uint32_t hi = is_ch1 ? HI_CH1 : is_ch2 ? HI_CH2 : LO_STAR ;
    uint32_t pc = is_ch1 ? 0b01   : is_ch2 ? 0b10   : 0b00;
    uint32_t lo = is_ch1 ? COM_LC : is_ch2 ? LO_CH2 : LO_STAR;


    const term_t  terms[ 4] =  /*( const wrapper_t ) */ {

      // oob reading, az mode.  hi first
      { // 0
      .azmux        = hi,
      .pc_sample    = pc,
      .oob_aperture = true,                   // use fast/constant aperture
      .zgjc         = true,                   // set zgjc here, also cm-dither, zero for noaz.
      .next_idx     = 1,
      },
      { // 1
      .azmux        = lo,
      .oob_aperture = true,
      .next_idx     = 2,
      },

      // normal reading, az mode, hi first
      { // 2
      .azmux        = hi,
      .pc_sample    = pc,
      .zgjc         = true,                   // set zgjc here, also cm-dither, zero for noaz.
      .next_idx     = 3,
      },
      { // 3
      .azmux        = lo,
      .next_idx     = 2,                      // jump to 2.
      },
    };
    memcpy( sa->terms, terms, sizeof( terms));


    // set decode strategy
    sa->decode_strategy = (void (*)( void *, data_t *)) decode_x;
    sa->decode_ctx      = malloc( sizeof( decode_x_t));  // TODO FIXME memory

    // init and set high first
    decode_x_init( sa->decode_ctx, true );

  }

  else if( strcmp( sa->input, "ratio") == 0 ) {

    assert( 0);
  }

  else {
    assert( 0);
  }
}




static void compile_sa_noaz_lo_first( sa_state_t *sa /* , const char *sbool noaz, bool oob  */)
{
  /*
    could probably just use the az_hi_first.
    and swap the terms around and patch the indices, and zgjc setting
    but it is complicated enough, to do it manually.
  */

  assert( sa->noaz);

  // clear memory
  _Static_assert( ARRAY_SIZE( sa->terms) ==  4);
  memset( sa->terms, 0, sizeof( sa->terms));


  bool is_ch1 = strcmp( sa->input, "ch1") == 0;
  bool is_ch2 = strcmp( sa->input, "ch2") == 0;
  bool is_0   = strcmp( sa->input, "0") == 0;


  if( is_ch1 || is_ch2 || is_0) {

    uint32_t hi = is_ch1 ? HI_CH1 : is_ch2 ? HI_CH2 : LO_STAR ;
    uint32_t pc = is_ch1 ? 0b01   : is_ch2 ? 0b10   : 0b00;
    uint32_t lo = is_ch1 ? COM_LC : is_ch2 ? LO_CH2 : LO_STAR;


    const term_t  terms[ 4] =  /*( const wrapper_t ) */ {

      // oob reading, is still az mode, hi first
      { // 0
      .azmux        = hi,
      .pc_sample    = pc,
      .oob_aperture = true,                     // use fast/constant aperture
      .zgjc         = true,                     // set zgjc here, also cm-dither, zero for noaz.
      .next_idx     = 1,
      },
      { // 1
      .azmux        = lo,
      .oob_aperture = true,
      .next_idx     = 2
      },

      // normal reading, noaz mode, lo first
      { // 2
      .azmux        = lo,
      .zgjc         = true,
      .next_idx     = 3,
      },
      { // 3
      .azmux        = hi,
      .pc_sample    = pc,
      .next_idx     = 3,                      // repeat
      },
    };
    memcpy( sa->terms, terms, sizeof( terms));


    // set decode strategy
    sa->decode_strategy = (void (*)( void *, data_t *)) decode_x;
    sa->decode_ctx      = malloc( sizeof( decode_x_t));  // TODO FIXME memory

    // init and set lo first
    decode_x_init( sa->decode_ctx, false);


  }
  else if( strcmp( sa->input, "ratio") == 0 ) {

    assert( 0);
  }

  else {
    assert( 0);
  }

}







/*
  - consider if issue around synchronization of state updates from REPL here .
  - the mode can be modified before it is written to the board.
    so there is potential for the decode handler to be out of synch, with sequencer running on the analog board.
  ----
  - but the analog board - is always written after REPL. command. without handling data.

*/

static void compile_sa( sa_state_t *sa)
{

  // cannot rebuild sequence terms.
  // unless have all the arguments...
  if( strlen( sa->input) == 0)
    return;

  if( sa->noaz)
    compile_sa_noaz_lo_first( sa);
  else
    compile_sa_az_hi_first( sa);
}




void sa_set( sa_state_t *sa, const char *s )
{
  assert( strcmp( s, "0") == 0
    || strcmp( s, "ch1") == 0
    || strcmp( s, "ch2") == 0
    || strcmp( s, "ratio") == 0);


  strncpy( sa->input, s, sizeof( sa->input));
  sa->input[ sizeof( sa->input) - 1] = 0;

  compile_sa( sa);
}













void _4094_state_clear_relays(_4094_state_t *state)
{


  // U401 conditioning
  state->K404  = SR_NONE;
  state->K403  = SR_NONE;
  state->K405  = SR_NONE;
  state->K406  = SR_NONE;


  // U402
  state->K407  = SR_NONE;
  state->K402  = SR_NONE;

  // u405
  state->K401 = SR_NONE;


  // u607
  state->K602  = SR_NONE;

  // u713 AMPS
  state->K701  = SR_NONE;
  state->K704  = SR_NONE;
  state->K707  = SR_NONE;

  // u705
  state->K702  = SR_NONE;
  state->K703  = SR_NONE;


  // u706
  state->K708  = SR_NONE;
  state->K705  = SR_NONE;
  state->K706  = SR_NONE;

}




#if 0

/* could pass the environment on creation. but think better if it belongs
  to the setter/controller context.
  similarly the memory for the decoder strategy could be  passed on creation.

*/

void mode_init(_mode_t *mode, environment_t *environment, decode_union_t * u)
{

}

#endif

void mode_reset(_mode_t *mode)
{
   /*
      all relays need to be defined as b01 or b10.
      for default initialization
      otherwise they will not get an initial pulse/value.
    */


  /*
    TODO FIXME memory leak. here
    mode->sa->decode_ctx = malloc( sizeof( decode_t));
  */


  *mode = ( const _mode_t) {


    .magic  = MODE_MAGIC,


    // U401
    .serial. K404  = SR_RESET,
    .serial. K403  = SR_RESET,
    .serial. K405  = SR_RESET,
    .serial. K406  = SR_RESET,

    // U402
    .serial. K407  = SR_RESET,
    .serial. K402  = SR_RESET,

    // u405
    .serial. K401  = SR_RESET,

    .serial. K701  = SR_RESET,
    .serial. K702  = SR_RESET,
    .serial. K703  = SR_RESET,

    ////////////

    // set lo-side drive of com-lc to A400-1/ star-ground
    .serial.U423      = D3,

    // set loside input boot buffer mux to A400-1/ star ground
    .serial.U426      = S4,


    ////////////

    // amplifier set fb 1x feb 2026.
    .serial .U506     = S8,


    // signal acquisition defaults
    .sa.p_trig_delay  = CLK_FREQ * 100e-3,         // 100ms
    .sa.p_precharge   = CLK_FREQ * 500e-6,          //  500us.


    .trigger_source   = 1,   // set internal trigger active

    // default adc
    .adc.p_aperture   = CLK_FREQ * 0.2,             // 200ms. 10nplc 50Hz.  // Not. should use current calibration?  // should be authoritative source of state.
    .adc.p_reset      = CLK_FREQ * 500e-6,          // 500us.

    .adc.p_aperture_oob = CLK_FREQ * 0.04,             // 1nplc

    // .reg_cr.mode      = 0,

    // eg sigmux should be on during normal integration.
    .reg_cr.adc_p_active_sigmux  = true

  };

}

/*
void mode_reset( _mode_t *mode)
{

  printf("mode_reset\n");

  // same as init
  mode_init( mode);
}

*/




void mode_gain_set( _mode_t *mode, uint32_t u)
{

  printf("set amp gain %lu\n", u);

  switch(u) {

    case 1:
      mode->serial .U506 = S8;
      break;

    case 10:
      mode->serial.U506 = S2;
      break;

    case 100:
      mode->serial.U506 = S3;
      break;

    case 1000:
      mode->serial.U506 = S4;
      break;

    default:
      assert(0);
  }

}








static void mode_lts_reset( _mode_t *mode )
{
  // mux agnd, instead of off. to reduce input leakage on mux followers.
  mode->serial.U1012  = S8 ;
  mode->serial.U1003  = S8 ;


}



void mode_lts_source_set( _mode_t *mode, double f0)
{

  printf("set lts\n");

  mode_lts_reset( mode);

  if(f0 >= 0) {
    printf("with +");
    mode->serial.U1003  = S1 ;       // positive source.
  } else if (f0 < 0) {

    printf("with -");
    mode->serial.U1003  = S2 ;      // negatie source
  }


  if( fabs(f0)  == 10) {
    printf("10V\n");
    mode->serial.U1012  = S1;         // 10V tap
  }
  else if(fabs(f0) == 1) {
    printf("1V\n");
    mode->serial.U1012  = S2;       // 1V.
  }
  else if(fabs(f0) == 0.1) {
    printf("0.1V\n");
    mode->serial.U1012  = S3;       // 0.1V.
  }
  else if(fabs(f0) == 0.01) {
    // not implemented/resistor not poplated
    printf("0.01V\n");
    mode->serial.U1012  = S4;       // 0.01V.
  }
/*
  note. makes no sense to source a zero here

*/
  else {
    // TODO argument validation
    // when called programmatically, should not fail.

    assert(0);
  }
}


void mode_daq_set( _mode_t *mode, unsigned u0, unsigned u1 )
{
  // mode_lts_reset( mode);

  // mode->serial.U1006  = S7;    // JA
  // mode->serial.U1007  = S7;

  // OK. we have to pass encoded arguments here.

  // set the hig/lo dac inputs.
  mode->serial.U1009  = u0;
  mode->serial.U1010  = u1;
}




void mode_invert_dac_set( _mode_t *mode, unsigned u0 )
{
  // invert dac
  printf("invert mdac\n");

  // dac7811.
  // 12 bits -  mask is FFF  == 4095
  // 0001 command to load and update.

  assert( u0 <= 0xfff);

  /*
    TODO reiew
    consider embedding the write code in the mode - is not quite right.
    mode should have the value.
    and use write code - in the underlying spi write.
    ---
    it is ok. for the moment
  */
  uint8_t cmd = 0x01;
  mode->mdac0_val = (cmd <<12) | (u0 & 0xfff);
}


void mode_sts_dac_set( _mode_t *mode, unsigned u0 )
{
  // sts dac
  printf("sts mdac\n");

  // dac7811.
  // 12 bits -  mask is FFF  == 4095
  // 0001 command to load and update.

  assert( u0 <= 0xfff);

  // move this to where dac is written. or insider the peripheral
  uint8_t cmd = 0x01;
  mode->mdac1_val = (cmd <<12) | (u0 & 0xfff);

}





static bool mode_loside_set( _mode_t *mode, const char *s)
{
  /*
    TODO add extra argument for the input.
      decode two arguments here. for boot channel U426.
      and output drive U423

  */

  // TODO need argument for the input channel to use
  // set to use channel 2.

  /*
    set boot for ch2. boot.
  */
  mode->serial.U426    = D2;      // ch 2  BOOT

  if(strcmp(s, "gnd") == 0
    || strcmp(s, "star") == 0) {

    // consider setting as default in the main.c mode initialization
    // could turn off the boot here.
    mode->serial.U423 = D3;      // drive com-lc with A400-1. star-ground

    uint16_t  val = 0x0;          // turn off
    mode_invert_dac_set( mode, val);
  }
  else if(strcmp(s, "invert") == 0
    || strcmp(s, "inverter") == 0) {

    /*
      should only really set boot.  if using the invert
    */

    mode->serial.U423 = D1;      // drive com-lc from mdac output

    uint16_t  val = 0xfff;        // full.
    mode_invert_dac_set( mode, val);
  }
  else if(strcmp(s, "divider") == 0
    || strcmp(s, "offset") == 0) {   // invert + divider.... for dither

    // invert but using the divider.
    mode->serial.U423 = D2;      // drive com-lc from mdac and divider

    uint16_t  val = 0x0;          // off
    mode_invert_dac_set( mode, val);
  }
  else if(strcmp(s, "boot") == 0) {

    mode->serial.U423 = D4;      // drive com-lc with boot direct
    uint16_t  val = 0x0;          // off
    mode_invert_dac_set( mode, val);
  }
  else {

    printf("bad lo-side argument\n");
    return 0;
  }

  return 1;
}





///////

/*
// The name dcv-source name scheme. is not quite right.
// it is all the ch2 inpputs. that are possible.
// i thinkk should set the feeder mux also.
// rather than ch1. and ch2.

// set ch2 temp.
// set ch2 ref
// set ch2 lts
// set ch2 sense
// set ch2 hv-div

it works well to coordinate the three input muxes together like this.

*/

/*
  Using string switch like - could simplify all this string handling ...

*/


void mode_ch2_reset(_mode_t *mode)
{
  // mode->serial.K405 = SR_RESET;    // accum ch2 off

  // inpput muxes.
  mode->serial.U419 = SOFF;     // himux or should be ref-lo? or agnd - for leakage?
  mode->serial.U420 = SOFF;     // lomux

  mode->serial.U409 = D4;   // feedmux  hi/lo
}



bool mode_ch2_set_relax( _mode_t *mode, const char *s0)
{

    if(strcmp(s0, "off") == 0 || strcmp(s0, "reset") == 0) {      // reset

      mode_ch2_reset(mode);
    }

    else if(strcmp(s0, "ref") == 0
		|| strcmp(s0, "ref-hi") == 0) {

      mode_ch2_reset(mode);

      mode->serial.U419 = S4;   // REF-HI
      mode->serial.U420 = S7;   // REF-LO
      mode->serial.U409 = D4;   // feedmux  hi/lo
    }
    else if(strcmp(s0, "ref-lo") == 0
		/*||  strcmp(s0, "ref_lo") == 0*/) {

      mode_ch2_reset(mode);
/*
    WANT range option. LO2.
    to sample ref-lo.
    using identical seq- elements.

    eg. using the azmux.

    Actually just star-ground  A400.  of the azmux.

*/
      mode->serial.U419 = S3;   // REF-LO
      mode->serial.U420 = S7;   // REF-LO
      mode->serial.U409 = D4;   // feedmux  hi/lo
    }
    else if(strcmp(s0, "temp") == 0) {

      mode_ch2_reset(mode);

      mode->serial.U419 = S1;   // TEMP
      mode->serial.U420 = S1;   // A400-1
      mode->serial.U409 = D4;   // feedmux  hi/lo
    }
    else if(strcmp(s0, "lts") == 0) {

      mode_ch2_reset(mode);

      mode->serial.U419 = S2;   // lts-source-hi
      mode->serial.U420 = S1;   // A400-1
      mode->serial.U409 = D4;   // feedmux
    }
    else if(strcmp(s0, "daq") == 0) {

      mode_ch2_reset(mode);

      mode->serial.U419 = S5;   // daq
      mode->serial.U420 = S5;   // com-lc
      mode->serial.U409 = D4;   // feedmux  hi/lo
    }
    else if(strcmp(s0, "shunts") == 0) {

      mode_ch2_reset(mode);

      mode->serial.U419 = S6;   // shunts hi
      mode->serial.U420 = S6;   // shunts lo
      mode->serial.U409 = D4;   // feedmux  hi/lo
    }
    else if(strcmp(s0, "tia") == 0) {

      mode_ch2_reset(mode);

      mode->serial.U419 = S8;   // tia
      mode->serial.U420 = S1;   // A400-1
      mode->serial.U409 = D4;   // feedmux  hi/lo
    }
    else if(strcmp(s0, "sense") == 0) {

      mode_ch2_reset(mode);

      mode->serial.U409 = D1;         // sense hi and lo
    }

    else if(strcmp(s0, "div") == 0) {

      mode_ch2_reset(mode);

      mode->serial.U409 = D3;         // dcv div
    }
    else {

      return 0;
    }

  return 1;
}



void mode_ch2_set( _mode_t *mode, const char *s0)
{
  bool ret = mode_ch2_set_relax( mode, s0);
  assert( ret);
}



/*
bool mode_repl_statement(
  _mode_t     *mode,
  const char  *cmd,

  // why not pass environmental state.
  const uint32_t line_freq
) {
*/

bool mode_repl_statement( _mode_t *mode, const char  *cmd, const environment_t *environment)
{

  assert( mode && mode->magic == MODE_MAGIC);
  assert( environment && environment->magic == ENVIRONMENT_MAGIC) ;


  char s0[ 100 + 1];
  char s1[ 100 + 1];
  // char s2[100 + 1 ];
  uint32_t u0, u1;
  double f0;
  // int32_t i0;

  /*

      TODO For case handling. just convert the entire command to lower case.
      except we dont really have a buffer

  */




  if(strcmp(cmd, "reset") == 0) {

    // reset the mode.
    mode_reset( mode);
  }




  /*
      we have to disambiguate values with float args explicitly...
      because float looks like int
  */

  ////////////////////////


  else if(strcmp(cmd, "noaz") == 0) {


    mode->sa.noaz = true;
    compile_sa( &mode->sa);
  }

  else if(strcmp(cmd, "az") == 0) {

    mode->sa.noaz = false;
    compile_sa( &mode->sa);
  }

  else if(
        (sscanf(cmd, "set az %100s", s0) == 1)
    ||  (sscanf(cmd, "sa %100s", s0) == 1)
    )  {

    sa_set( &mode->sa, s0);

  }



  else if( sscanf(cmd, "nplc %100s", s0) == 1
    && str_decode_float( s0, &f0))
  {
    // use float here, to express sub 1nplc periods
    if( ! nplc_valid( f0 ))  {
        printf("bad nplc arg\n");
        // return 1;
    } else {

      // should be called cc_aperture or similar.
      uint32_t aperture = nplc_to_aperture( f0, environment->line_freq);

      aperture_print( aperture,  environment->line_freq);

      adc_aperture_set( &mode->adc, aperture );

      // mode->adc.p_aperture = aperture;
    }
  }



  else if( sscanf(cmd, "aper %100s", s0) == 1
    && str_decode_float( s0, &f0))
  {

    // printf("set aperture\n");
    uint32_t aperture = period_to_aperture( f0 );
    // assert(u1 == 1 || u1 == 10 || u1 == 100 || u1 == 1000); // not really necessary. just avoid mistakes
    aperture_print( aperture,  environment->line_freq);
    mode->adc.p_aperture = aperture;
  }


  else if((sscanf(cmd, "precharge %100s", s0) == 1)
    && str_decode_uint( s0, &u0))  {

    // TODO consider use time, rather than raw count
    mode->sa.p_precharge = u0;
  }

  else if((sscanf(cmd, "trig delay %100s", s0) == 1)
    && str_decode_uint( s0, &u0))  {

    // TODO consider use time, rather than raw count
    mode->sa.p_trig_delay = u0;
  }






#if 0
  // channel set
  else if( sscanf(cmd, "set ch1 %100s", s0) == 1)
  {
    if(strcmp(s0, "off") == 0 || strcmp(s0, "reset") == 0) {
      mode_ch1_reset(mode);
    }
    else if(strcmp(s0, "dcv") == 0) {
      mode_ch1_set_dcv(mode);
    }
    else if(strcmp(s0, "dcv-source") == 0) {
      mode_ch1_set_dcv_source(mode);
    }
    else assert(0);
  }
#endif


  // need to work out if keep the set...


  else if((sscanf(cmd, "set gain %100s", s0) == 1)
    && str_decode_uint( s0, &u0))  {


    mode_gain_set( mode, u0 );
  }





  else if( sscanf(cmd, "set lts %lf", &f0) == 1) {

    mode_lts_source_set( mode, f0);
  }




  else if( sscanf(cmd, "set daq %100s %100s", s0, s1 ) == 2
    && str_decode_uint( s0, &u0)
    && str_decode_uint( s1, &u1)
  )  {

    // eg. 'dcv-source daq s1 s2'
    // mode_ch2_set_daq( mode, u0, u1);
    mode_daq_set( mode, u0, u1);
  }



  // TODO consider better name than iso. eg. just sts,  or floating sts. flt-sts-dcv-source
  // consider remove prefix m. from the dac.

  // inverter dac
  else if(( sscanf(cmd, "set invert %100s", s0) == 1
    || sscanf(cmd, "set mdac0 %100s", s0) == 1)    // doesn't work?

    && str_decode_uint( s0, &u0)
  )  {

    if( u0 <= 0xfff)
      mode_invert_dac_set( mode, u0);
    else
      printf("arg out of range\n");

  }

  // iso dac
  else if(( sscanf(cmd, "set sts %100s", s0) == 1
    || sscanf(cmd, "set mdac1 %100s", s0) == 1 )
    && str_decode_uint( s0, &u0)
  )  {

    if( u0 <= 0xfff)
      mode_sts_dac_set( mode, u0);
    else
      printf("arg out of range\n");
  }


#if 0
  else if( sscanf(cmd, "dcv-source sts %100s", s0) == 1
    && str_decode_int( s0, &i0)) {

    // hex values are not working.
    printf("value %ld hex %lx\n", i0, i0 );

      // this isnt quite working.
    // note. can take a negative value.
    // eg. 0x3fff or -0x3fff
    mode_ch2_set_sts( mode, i0);
  }

#endif


  else if( sscanf(cmd, "set loside %100s", s0) == 1
  )  {

    bool ret = mode_loside_set( mode, s0);
    if(!ret) {
      printf("bad argument\n");
    }
  }





  else if( sscanf(cmd, "set ch2 %100s", s0) == 1)
  {
    // why not manage this argument passing

    bool ret = mode_ch2_set_relax( mode, s0);
    if(!ret) {
      printf("arg unrecognized\n");
      assert(0);
      return 0;
    }

  }



  /*
    two val set cmd
    mmore contol over low level mode fields
  */
  else if( sscanf(cmd, "set %100s %100s", s0, s1) == 2
    && str_decode_uint( s1, &u0)
  ) {

      // printf("set %s %lu\n", s0, u0);


/*
      if(strcmp(s0, "azmux") == 0) {
        mode->reg_direct.azmux_o = u0;
      }
*/

/*
      else if(strcmp(s0, "seqn") == 0) {
        mode->sa.p_seq_n = u0;
      }
*/

      // control register mode
      if(strcmp(s0, "mode") == 0) {      // sa mode unused. may 2026.

        cr_sa_mode_set( &mode->reg_cr, u0);
      }

/*
      else if(strcmp(s0, "direct") == 0) {

        _Static_assert( sizeof(mode->reg_direct) == 4);
        _Static_assert( sizeof(u0) == 4);

        memcpy( &mode->reg_direct, &u0, sizeof(mode->reg_direct));
      }
*/


/*
      // set led field
      else if(strcmp(s0, "leds") == 0) {
        mode->reg_direct.leds_o = u0;
      }

      // by field
      else if(strcmp(s0, "monitor") == 0) {
        mode->reg_direct.monitor_o = u0;
      }
      else if(strcmp(s0, "adc_refmux") == 0) {
        mode->reg_direct.adc_refmux_o = u0;
      }

      else if(strcmp(s0, "adc_cmpr_latch") == 0) {
        mode->reg_direct.adc_cmpr_latch_o = u0;
      }
      else if(strcmp(s0, "spi_interrupt_ctl") == 0) {
        mode->reg_direct.spi_interrupt_ctl_o = u0;
      }

*/
      ////////////////////////////////////////////
      // 4094 components.
      // perhaps rename serial. _4094_serial etc.

/*
      else if(strcmp(s0, "u504") == 0) {
        mode->serial.U504 = u0;
      }

*/
      else if(strcmp(s0, "u1003") == 0) {
        mode->serial.U1003 = u0;
      }
/*      else if(strcmp(s0, "u1006") == 0) {
        mode->serial.U1006 = u0;
      }
*/
      else if(strcmp(s0, "u1012") == 0) {
        mode->serial.U1012 = u0;
      }

      else if(strcmp(s0, "u1009") == 0) {
        mode->serial.U1009 = u0;
      }
      else if(strcmp(s0, "u1010") == 0) {
        mode->serial.U1010 = u0;
      }


#if 0
      else if( strcmp(s0, "dac") == 0 || strcmp(s0, "u1016") == 0 || strcmp(s0, "u1014") == 0) {
        // let the mode update - determine setting up spi params.
        mode->dac_val = u0;
      }
#endif
      /*
          handle latch relay pulse encoding here, rather than at str_decode_uint() time.
          valid values are 1 (0b01)  and 2 (0b10). not 1/0.
          reset is default schem contact position.
      */

      else if(strcmp(s0, "u409") == 0 || strcmp(s0, "feedmux") == 0) {

        // should use set channel functionality instead.
        mode->serial.U409 = u0 ;
      }

      else if(strcmp(s0, "u423") == 0) {
        mode->serial.U423 = u0 ;
      }
      else if(strcmp(s0, "u426") == 0) {
        mode->serial.U426 = u0 ;
      }




      else if(strcmp(s0, "k401") == 0) {
        mode->serial.K401 = u0 ? SR_SET: SR_RESET;
      }
      else if(strcmp(s0, "k402") == 0) {
        mode->serial.K402 = u0 ? SR_SET: SR_RESET;
      }
      else if(strcmp(s0, "k403") == 0) {
        mode->serial.K403 = u0 ? SR_SET: SR_RESET;
      }
      else if(strcmp(s0, "k404") == 0) {
        mode->serial.K404 = u0 ? SR_SET: SR_RESET;
      }
      else if(strcmp(s0, "k405") == 0) {
        mode->serial.K405 = u0 ? SR_SET: SR_RESET;
      }
      else if(strcmp(s0, "k406") == 0) {
        mode->serial.K406 = u0 ? SR_SET: SR_RESET;
      }
      else if(strcmp(s0, "k407") == 0) {
        mode->serial.K407 = u0 ? SR_SET: SR_RESET ;      // 0 == reset
      }


      else if(strcmp(s0, "k701") == 0) {
        mode->serial.K701 = u0 ? SR_SET: SR_RESET;
      }
      else if(strcmp(s0, "k702") == 0) {
        mode->serial.K702 = u0 ? SR_SET: SR_RESET;
      }
      else if(strcmp(s0, "k703") == 0) {
        mode->serial.K703 = u0 ? SR_SET: SR_RESET;
      }
      else if(strcmp(s0, "k704") == 0) {
        mode->serial.K704 = u0 ? SR_SET: SR_RESET;
      }
      else if(strcmp(s0, "k705") == 0) {
        mode->serial.K705 = u0 ? SR_SET: SR_RESET;
      }



      else {

        printf("unknown target %s for 2 var set\n", s0);
        return 0;

      }
  }



  else {

    return 0;
  }



  return 1;
}









#if 0
typedef struct decode_oob_t
{
  /*
    oob reading. is always the same.  regardless az,noaz. or dither etc.
    so just factor into  one place
  */

  // persist...  for AZ. from last reading
  uint32_t adc_refmux_pos_hi;
  uint32_t adc_refmux_neg_hi;

} decode_oob_t;

#endif



#if 0

static void decode( decode_t *decode, data_t *data)
{
  /*
    oob. is hi first by convention.
    comparators get a look at the input in priority.
    to check for oscillation detection/strong OVLD,UNDL condition.
    even if comparators are not used for ranging control.
  */

  const reg_sr_t status = data->status;
  const term_t term  = data->term;
  double cal_w          = data->cal->w;
  assert( cal_w);

  assert( term.oob_aperture);

  if( status.sample.first) {

    printf( "clear ");

    // clear state after a re/trigger action
    decode->adc_refmux_pos_hi = 0; // UINT32_MAX
    decode->adc_refmux_neg_hi = 0;
  }

  if( status.sample.idx % 2 == 0) {

    printf( "hi ");

    // HI.  record counts.
    decode->adc_refmux_pos_hi     = data->adc_refmux_pos;
    decode->adc_refmux_neg_hi     = data->adc_refmux_neg;
    assert( data->reading_valid == false);
  }
  else {

    printf( "lo ");

    // LO convert value
    data->count_sum =
        ((double) decode->adc_refmux_pos_hi - (cal_w * decode->adc_refmux_neg_hi))
      - ((double) data->adc_refmux_pos      - (cal_w * data->adc_refmux_neg));

    data->reading_valid     = true;
  }
}

#endif




#if 0

void mode_ch1_accum( _mode_t *mode, bool val)
{
  mode->serial.K406 = val ? SR_SET : SR_RESET;
}


void mode_ch2_accum( _mode_t *mode, bool val)
{
  mode->serial.K406 = val ? SR_SET : SR_RESET;

}

#endif


#if 0


/*
  Not sure we even need these functions -
    for range - it is clearer to set up.
    for calibration, set up states explicity

*/

void mode_ch1_reset(_mode_t *mode)      // change name reset() ?
{

  mode->serial.K402 = SR_RESET;    // input off
  mode->serial.K406 = SR_RESET;    // accum ch1 off
  mode->serial.K405 = SR_RESET;    // accum ch2 off


  mode->serial.K407 = SR_RESET;    // dcv-source off

  mode->serial.K401 = SR_RESET;    // ohms off
  mode->serial.K404 = SR_RESET;    // lts-source off
  mode->serial.K403 = SR_RESET;    // 10Meg impedance off
}


void mode_ch1_set_dcv(_mode_t *mode)
{
  mode_ch1_reset( mode);
  mode->serial.K402 = SR_SET;      // input on
}

void mode_ch1_set_dcv_source(_mode_t *mode)   // rename use K404. instead.
{
  mode_ch1_reset( mode);
  mode->serial.K407 = SR_SET;      // dcv-source on
}


#endif










#if 0
  /*
    perhaps keep the 'set' prefix to clearly disambiguate these actions under common syntactic form.
  */

  // three val
  else if( sscanf(cmd, "set %100s %100s %100s", s0, s1, s2) == 3
    && str_decode_uint( s1, &u0)
    && str_decode_uint( s2, &u1)
  ) {
      /*
        setting/encoding sequence values directly.
        eg.
        > set seq0 0b01 s3
        > set seq0 0b00 soff
      */

      // maybe be handy to have in a function. or else return
      uint32_t val =  ((u0 & 0b11) << 4) | ( u1 & 0b1111);

      if(strcmp(s0, "seq0") == 0) {
        mode->sa.p_seq0 = val;
      }
      else if(strcmp(s0, "seq1") == 0) {
        mode->sa.p_seq1 = val;
      }
       else if(strcmp(s0, "seq2") == 0) {
        mode->sa.p_seq2 = val;
      }
      else if(strcmp(s0, "seq3") == 0) {
        mode->sa.p_seq3 = val;
      }
      else {
        printf("unknown target %s for 3 var set\n", s0);
        return 0;
      }

  }

#endif




#if 0
    if(strcmp(s0, "off") == 0 || strcmp(s0, "reset") == 0) {      // reset
      mode_ch2_reset(mode);
    }

    else if(strcmp(s0, "ref") == 0
		|| strcmp(s0, "ref-hi") == 0) {

      mode_ch2_set_ref( mode);
    }
    else if(strcmp(s0, "ref-lo") == 0
		||  strcmp(s0, "ref_lo") == 0) {
      mode_ch2_set_ref_lo( mode);
    }
    else if(strcmp(s0, "temp") == 0) {
      mode_ch2_set_temp( mode);
    }
    else if(strcmp(s0, "lts") == 0) {
      mode_ch2_set_lts( mode);
    }
    else if(strcmp(s0, "daq") == 0) {
      mode_ch2_set_daq( mode);
    }
    else if(strcmp(s0, "shunts") == 0) {
      mode_ch2_set_shunts(mode);
    }
    else if(strcmp(s0, "tia") == 0) {
      mode_ch2_set_tia( mode );
    }
    else if(strcmp(s0, "sense") == 0) {
      mode_ch2_set_sense(mode);
    }
    else if(strcmp(s0, "div") == 0) {
      mode_ch2_set_dcv_div(mode);
    }
    else {
      printf("unrecognized\n");
      return 0;
      // assert(0);
    }

#endif






#if 0



static void mode_ch2_set_iso( _mode_t *mode, signed u0 )
{
  /* this function is possible. but it is a bit confusing parallel state.
    because output does not appear
    and it is read from from the daq.
    so should probably be handled exceptionally

    and would need a call to the daq.
  - void mode_ch2_set_daq( _mode_t *mode, unsigned u0, unsigned u1 )
  */

  UNUSED(mode);
  UNUSED(u0);
  assert(0);
}



// remove argument handling is messy here.
void mode_ch2_set_ref( _mode_t *mode )
{
  // rename mode_dcv_ref_source

  mode_lts_reset( mode);

  if(u0 == 7) {
    printf("with ref-hi +7V\n");
    mode->serial.U1006  = S4;       // ref-hi
    // mode->serial.U1007  = S4;       // ref-lo
  }
  else if( u0 == 0 ) {
    // need bodge for this
    printf("with ref-lo\n");
    mode->serial.U1006  = S8;       // ref-lo
    // mode->serial.U1007  = S4;       // ref-lo - looks funny. gives bad measurement. on DMM.
  }
  else
    assert(0);
}




// this is a poor abstraction.
void mode_ch2_set_channel( _mode_t *mode, unsigned u0 )
{

  // neither channel
  mode->serial.K407 = SR_RESET;
  mode->serial.U409 = DOFF;       // hi/lo mux.

  if(u0 == 1) {

    mode->serial.K407 = SR_SET;
  } else if(u0 == 2) {

    mode->serial.U409 = D4;
  }

}

#endif





/*
  TODO.
  this function is awful.

  instead just set up a closure/handler - at the time we set the AZ mux sequencing.

  instead code as,

     set_seq( idx, pc, azmux );
     set_seq_n();

  also the catcher/interpreter of the result.
  remember it's main use used can also

  just encode by hand. as we setup modes at top level.

*/

  // anything on channel two.

/*
  // zero serial
  sa->terms[ 0].azmux  = S6;     // A400-1
  sa->terms[ 0].pc = 0b00;

  // val
  sa->terms[ 1].azmux  = S3;     // CH2-IN
  sa->terms[ 1].pc = 0b10;


  // could set the LS drive. serial input switch here - eg. BOOT1/BOOT2/ agnd. if wanted.
  // so it is set up for the double range.
  // set the data catcher closurec.
*/

#if 0

void mode_seq_set( _mode_t *mode, uint32_t seq_mode , uint8_t arg0, uint8_t arg1 )
{
  /*
    doesn't have to be exhausive wrt cases.
    can still setup manually.
  */
/*
    we could define these in define int. also.
    so that the strings would be correctly decoded.

    S3 - dcv
    S7 - star-lo
    S1 - himux
    S8 - lomux
*/

  mode->reg_seq_mode = seq_mode;                 // to guide decoder

  switch(seq_mode) {


    // boot mode - might be particularly useful when sampling.

    case SEQ_MODE_BOOT: {
      // sample a hi, but don't switch the pc switch, generally only used for electrometer, very high input impedance.

      mode->sa.p_seq_n = 1;
      if(arg0 == S3 ) {
        mode->sa.p_seq0 = (0b00 << 4) | S3;        // dcv
      }
      else if(arg0 == S1 ) {
        mode->sa.p_seq0 = (0b00 << 4) | S1;        // himux
      }
      else assert(0);
      break;
    }


    /*/ for noaz.
    // if we were to slect a lo here...
    // if it's a hi - then switch the PC - for symmetry. if lo. then don't bother.
    */

    case SEQ_MODE_NOAZ: {
      // clearer - to express as another mode, rather than as a bool.
      // azero off - just means swtich the pc for symmetry/ and keep charge-injetion the same with azero mode.

      mode->sa.p_seq_n = 1;
      if(arg0 == S3 ) {
        mode->sa.p_seq0 = (0b01 << 4) | S3;        // dcv
      }
      else if(arg0 == S1 ) {
        mode->sa.p_seq0 = (0b10 << 4) | S1;        // himux
      }
      else if(arg0 == S7 ) {
        mode->sa.p_seq0 = (0b00 << 4) | S7;        // star-lo
      }
      else if(arg0 == S8 ) {
        mode->sa.p_seq0 = (0b00 << 4) | S8;        // lomux
      }

      else assert(0);
      break;
    }



    case SEQ_MODE_AZ: {
    // write the seq

      mode->sa.p_seq_n = 2;

      // hi goes serial

      if(arg0 == S3 )
        mode->sa.p_seq0 = (0b01 << 4) | S3;      // dcv
      else if(arg0 == S1 )
        mode->sa.p_seq0 = (0b10 << 4) | S1;        // himux
      else
        assert(0);

      if(arg1 == S7)
        mode->sa.p_seq1 = (0b00 << 4) | S7;        // star-lo
      else if(arg1 == S8)
        mode->sa.p_seq1 = (0b00 << 4) | S8;        // lomux
      else
        assert(0);
/*
    // applies both chanels.
      if(arg0 == S3 && arg1 == S7) {
        mode->sa.p_seq0 = (0b01 << 4) | S3;        // dcv
        mode->sa.p_seq1 = (0b00 << 4) | S7;        // star-lo
      }
      else if(arg0 == S1 && arg1 == S8)  {
        mode->sa.p_seq0 = (0b01 << 4) | S1;        // himux   WRONG. FIXME.   not switching the PC.
        mode->sa.p_seq1 = (0b00 << 4) | S8;        // lomux
      }
      else if(arg0 == S3 && arg1 == S8 )  {                // eg. for ref.
        mode->sa.p_seq0 = (0b01 << 4) | S3;        // dcv
        mode->sa.p_seq1 = (0b00 << 4) | S8;        // lomux
      }
      else assert(0);
*/
      break;
    }

/*
    case SEQ_MODE_ELECTRO: {

      // same as no az, except don't switch the precharge
      mode->sa.p_seq_n = 1;
      mode->sa.p_seq0 = (0b00 << 4) | S3;        // dcv
      break;
    }
*/

    case SEQ_MODE_AG:
    case SEQ_MODE_RATIO: {
      // 4 cycle, producing single output
      // Issue - is for internal - we need to set the common lo. eg. ref-lo. or start

      mode->sa.p_seq_n = 4;
      mode->sa.p_seq0 = (0b01 << 4) | S3;        // dcv
      mode->sa.p_seq1 = (0b00 << 4) | S8;        // ref-lo // star-lo
      mode->sa.p_seq2 = (0b10 << 4) | S1;        // himux
      mode->sa.p_seq3 = (0b00 << 4) | S8;        // ref-lo /// lomux
      break;
    }

/*
    case SEQ_MODE_AG: {
      // auto-gain 4 cycle - same as ratio. producing a single output


      mode->sa.p_seq_n = 4;
      // sample
      mode->sa.p_seq0 = (0b01 << 4) | S3;        // dcv
      mode->sa.p_seq1 = (0b00 << 4) | S7;        // star-lo
      // reference
      mode->sa.p_seq2 = (0b10 << 4) | S1;        // himux
      mode->sa.p_seq3 = (0b00 << 4) | S7;        // lomux
      break;
    }
*/
    case SEQ_MODE_DIFF: {
      // 2 cycle, hi- hi2, with both precharge switches switches. single output.
      mode->sa.p_seq_n = 2;
      mode->sa.p_seq0 = (0b01 << 4) | S3;        // dcv
      mode->sa.p_seq2 = (0b01 << 4) | S1;        // himux
      break;
    }

    case SEQ_MODE_SUM_DELTA: {    // change name.  SUM_DELTA. 0w

      // similar. take hi/lo, hi2/lo, .  but where lo is shared. so can calculate hi-lo, hi2-lo, hi-hi2.
      // advantage of a single sequence - is that flicker noise should cancel some.
      // noting that input can be external terminals - or the dcv-source and its inverted output.
      // to encodekkkkkkkkk
      // can do as 3 values or 4 values.   3 is more logical.

      mode->sa.p_seq_n = 3;
      mode->sa.p_seq0 = (0b01 << 4) | S3;        // dcv
      mode->sa.p_seq1 = (0b00 << 4) | S7;        // star-lo
      mode->sa.p_seq2 = (0b10 << 4) | S1;        // himux
      break;
    }

    default:
      assert( 0);
  }


}

#endif




#if 0

  //  maybe make explicit all values  U408_SW_CTL. at least for the initial mode, from which others derive.

  .serial .U408_SW_CTL = 0,      // b2b fets/ input protection off/open
  .serial.U408_SW_CTL = 0,

  // AMP FEEDBACK SHOULD NEVER BE TURNED OFF.
  // else draws current, and has risk damaging parts. mux pin 1. of adg. to put main amplifier in buffer/G=1 configuration.
  .serial. U506 =  D1,     // should always be on
  .serial.U506 =  D1,           // amplifier should always be on.

  .serial. K603_CTL  = SR_RESET,     // ohms relay off.


  /////////////////////////
  // 700
  // has inverting cmos buffer
  .serial. K702_CTL  = SR_RESET,
  .serial.K702_CTL  = 0b11,

  // 0.1R shunt off. has inverting cmos buffer
  .serial. K703_CTL  = SR_RESET,
  .serial.K703_CTL  = 0b11,

  // shunts / TIA - default to shunts
  .serial. K709_CTL  = SR_SET,

  // agn200 shunts are off.
  .serial. K707_CTL  = SR_SET,
  .serial. K706_CTL  = SR_SET,
  .serial. K704_CTL  = SR_SET,
  .serial. K705_CTL  = SR_SET,


#endif




#if 0
  // mode_lts_reset( mode);

  // mode->serial.U1006  = S3;       // JA mux dac

  if(u0 >= 0) {
    printf("positive");
    mode->serial.U1003  = S2;       // positive source.
  } else if (u0 < 0) {

    printf("neg");
    mode->serial.U1003  = S1;      // negatie source
  }

#endif









#if 0
  else if( sscanf(cmd, "dcv-source chan %lu", &u0) == 1)  {

    //
    //  set channel.  1 on.  2. on 1 off.
    // kind of need off and on.?
    // eg. 'dcv-source daq s1 s2'

    mode_ch2_set_channel( mode, u0);
  }

#endif



#if 0

  else if( sscanf(cmd, "boot%100s", s0) == 1
    && str_decode_uint( s0, &u0))  {

    mode_seq_set( mode, SEQ_MODE_BOOT, u0, 0 );
  }
  else if( sscanf(cmd, "noazero %100s", s0) == 1
    && str_decode_uint( s0, &u0))  {

    mode_seq_set( mode, SEQ_MODE_NOAZ, u0, 0 );
  }
  else if( sscanf(cmd, "azero %100s %100s", s0, s1) == 2
    && str_decode_uint( s0, &u0)
    && str_decode_uint( s1, &u1)) {

    mode_seq_set( mode, SEQ_MODE_AZ, u0, u1 );
  }

    // ratio is hardcoded to use lomux at the moment. and not star-lo.
  else if(strcmp(cmd, "ratio") == 0) {
    mode_seq_set( mode, SEQ_MODE_RATIO, 0,0 );
  }

  else if(strcmp(cmd, "ag") == 0)
    mode_seq_set( mode, SEQ_MODE_AG, 0, 0 );

  else if(strcmp(cmd, "diff") == 0)
    mode_seq_set( mode, SEQ_MODE_DIFF, 0 , 0);

  else if(strcmp(cmd, "sum-test") == 0)
    mode_seq_set( mode, SEQ_MODE_SUM_DELTA, 0, 0 );

#endif










#if 0
  spi_controller_configure( spi_4094);
  spi_4094_write_n( spi_4094, (void *) &x , 4  );
/*
  uint32_t x = 0xffffffff;
  uint32_t x = 0b10101010101010101010101010101010;
  uint32_t y = 0b01010101010101010101010101010101;

  // sleep 10ms, for relays
  msleep(10, system_millis);

  // and write device
  spi_4094_write_n( spi_4094, (void *) &y , 2  );

  // sleep 10ms, for relays
  msleep(10, system_millis);

  // and write device
  spi_4094_write_n( spi_4094, (void *) &x , 2  );
*/
#endif




  /*
    setup the sequence numbers for the different modes.
    we could inject this field - as a string - into data as well.
      or write it using a status bit, of adc for good synchronization. from mode -> adc -> to stamped values, read by data.

    this is a read_mode.  or sequence_mode.
  */



// Might be cleaner to have functions() for these.
// or just pass. note the CH can be represented as an argument.


