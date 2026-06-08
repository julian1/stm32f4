


#include <assert.h>
#include <stdio.h>
// #include <math.h>   // fabs,
#include <string.h>   // strcmp, memset
#include <stdlib.h>   // malloc               // TODO review


#include <support.h> // str_decode_uint


#include <lib3/util.h>        // ARRAY_SIZE


#include <data/data.h>

#include <sa.h>


#include <device/spi-fpga0-reg.h>








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



#define DECODE_MAGIC 987234987



typedef struct decode_t
{
  uint32_t    magic;

  /*
    for low level aggregation. we need the count limit
    so either have to pass the variable.
    or inject the sa structure.
    - passing sa. means we have access to other flags.
  */

  const sa_state_t *sa;

  // pos - neg counts. weight adjusted. not normalized.
  double hi;
  double lo;


  unsigned count;   // when aggregating counts
                    // otherwise unused

/*
  // uint32_t count_aggregate;    // if handle aggregation here
  // uint32_t adc_sigmux ;        // summed across
  // uint32_t adc_sigmux_lo/hi;   // if maintain separate counts.
*/
} decode_t;




static void decode_init( decode_t *decode, const sa_state_t *sa)
{
  assert( decode);

  decode->magic = DECODE_MAGIC ;

  decode->sa = sa;

  decode->hi = 0;
  decode->lo = 0;
  decode->count = 0;
}




static void decode_reset( decode_t *decode )
{

  assert( decode && decode->magic == DECODE_MAGIC );
  /*
    - calling reset(). more than once is less good. on malloc structure
        bad. if sets the magic. because possible
        probably want init() separate from reset() which leaves count_n. or sa_t * pointer untouched.
  */


  printf( "clear ");

  decode->hi = 0;
  decode->lo = 0;

  decode->count = 0;

  // decode->adc_sigmux  = 0;    // if use

}





static void decode_noaz_lo_first( decode_t *decode, data_t *data)
{
  assert( decode && decode->magic == DECODE_MAGIC );

  if( !data)  {

    // important. can use decode specific structure with decode specific reset.
    decode_reset( decode);
    return;
  }
  assert( data && data->magic == DATA_MAGIC);

  const reg_sr_t status = data->status;
  const term_t term     = data->term;


  if( status.sample.idx % 2 == 0) {

    // LO   record counts.
    printf( "lo, ");
    decode->lo = (double) data->adc_refmux_pos  - (data->cal_w * data->adc_refmux_neg);     // we could even normalize here...
  }
  else {

    // HI convert value
    printf( "hi, ");
    decode->hi = (double) data->adc_refmux_pos  - (data->cal_w * data->adc_refmux_neg);     // we could even normalize here...

    double count_sum = decode->hi  - decode->lo;
    // printf("sum %.2f, ", count_sum);

    // normalize count
    data->count_sum_norm = count_sum  / data->adc_sigmux;   // should be sigmux * 2 ? for both a HI and LO.
                                                            // doesn't matter the normalizing term. just has to be consistent.
    data->reading_valid  = true;

    assert( !term.second);
  }
}




static void decode_az_hi_first( decode_t *decode, data_t *data)
{
  assert( decode && decode->magic == DECODE_MAGIC );

  if( !data)  {

    // important. can use decode specific structure with decode specific reset.
    decode_reset( decode );
    return;
  }
  assert( data && data->magic == DATA_MAGIC);


  const reg_sr_t status = data->status;
  const term_t term     = data->term;


  if( status.sample.idx % 2 == 0) {

    // HI.  record counts.
    printf( "hi, ");
    decode->hi = (double) data->adc_refmux_pos  - (data->cal_w * data->adc_refmux_neg);     // we could even normalize here...
  }
  else {

    // LO convert value
    printf( "lo, ");
    double lo = (double) data->adc_refmux_pos  - (data->cal_w * data->adc_refmux_neg);     // we could even normalize here...

    // if have prior LO if available, use average with this Lo.
    double av_lo =
      decode->lo
      ? ( lo + decode->lo  ) / 2.           // is this correct???? i think so.
      : lo;

    double count_sum = decode->hi - av_lo;

    assert( !term.second);


    // normalize count
    data->count_sum_norm = count_sum  / data->adc_sigmux;
    data->reading_valid     = true;

    // record/update LO. for next time
    decode->lo = lo;
  }
}



static void decode_az_hi_first_aggregate( decode_t *decode, data_t *data)
{
  assert( decode && decode->magic == DECODE_MAGIC );
  /*
    sum the counts rather than record.

    We want to be able to use this strategy. even with aggregate = 1;
    in order to suppress the consequeive lo averaging of normal  decode_az_hi_first
  */


  if( !data)  {

    // important. can use decode specific structure with decode specific reset.
    decode_reset( decode);
    return;
  }
  assert( data && data->magic == DATA_MAGIC);

  const reg_sr_t status = data->status;
  const term_t term     = data->term;


  if( status.sample.idx % 2 == 0) {

    // HI.  sum counts
    /*
      risk of overflow with uin32_t here?????
        Math.pow(2,32) / 20MHz. = 214 seconds.
    */
    printf( "hi %u, ", decode->count);
    decode->hi += (double) data->adc_refmux_pos  - (data->cal_w * data->adc_refmux_neg);     // we could even normalize here...
  }
  else {


    printf( "lo %u, ", decode->count);
    decode->lo += (double) data->adc_refmux_pos  - (data->cal_w * data->adc_refmux_neg);     // we could even normalize here...

    // the aggregate count... should count LOs. and His. separately?  as test...
    ++decode->count;

    // if use aggregate of 1... then we get normal hi-lo behavior...

    // OK. we want to be able tif we can set decode.   we want to support aggregate

    // support one or more
    assert( decode->sa->aggregate != 0);

    if( decode->count == decode->sa->aggregate /* 10 */) {

      // convert value
      double count_sum = decode->hi - decode->lo;

      data->count_sum_norm = count_sum  / ( data->adc_sigmux * decode->count);
      data->reading_valid = true;

      assert( !term.second);
/*
      if( term.second)
        data->count_sum_norm2 = ...
      else
        data->count_sum_norm = ...
*/

      if( decode->sa->verbose) {
        printf( "count_sum %g, ", count_sum);
        printf( "norm %g, ",      data->count_sum_norm);
      }

      decode_reset( decode);
    }
  }
}




/*
  consider. move all the sa.  stuff into own file.

*/


void sa_decode_reading( const sa_state_t *sa, data_t *data)
{
  assert( sa);

  const reg_sr_t  status = data->status;

  assert( sa->ctx_oob);
  assert( sa->ctx_normal);


  /* this is a bit complicated.
    beccause both oob and normal need to see the first flag.
    to reset the data counts after re/trigger.
    but we only get the first flag in context of oob.
    so use a null data argument instead. to communicate first
    and the need to reset.
  */

  if( status.sample.first) {

    // use NULL data argument to indicate first
    sa->decode_oob(    sa->ctx_oob,    NULL);

    // both normal and second context. use normal handler
    sa->decode_normal( sa->ctx_normal, NULL);
    sa->decode_normal( sa->ctx_second, NULL);
  }


  if( data->term.oob) {

    printf("oob, ");
    sa->decode_oob( sa->ctx_oob,    data);
  }
  else if( data->term.second) {
    // use a separate decode_t state.
    // EXTR.
    // behavior is similar to oob.
    // indicates we need separate state to manage az,noaz cases and count aggregation,
    // an independent reading is needed for downstream OL detection, and perhaps AR. on second input etc.
    // so in this context - treat as a separate independent reading, rather than jump to calculating the ratio.
    // EXTR.
    // using this flag means we do not have to resort to checking the index.
    // to determine a second channel.

    sa->decode_normal( sa->ctx_second, data);
  }
  else {
    printf("normal, ");
    sa->decode_normal( sa->ctx_normal, data);

    /*
      EXTR.
      use a separate strategy function for 'ratio' mode here.
      rather than interpreting flags/ term index for the channel.
      the point of localizing and the decode function assignment is to match the conversion terms.
    */
    /*
      EXTR.
      Also 'ratio' does *not* require the sigmux to normalize.
      - so it is a bit different than just dividing left/right
      So. treat as a distinct decoder.
      rather than a second decoder. manaaged outside the strategy function.
      *and* this way we can set it up to match the term order/setup.
    */
  }
}









static void sa_compile_terms_az_hi_first( sa_state_t *sa)
{
  assert( sa);

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
      .oob          = true,                   // use fast/constant aperture
      .zgjc         = true,                   // set zgjc here, also cm-dither, zero for noaz.
      .next_idx     = 1,
      },
      { // 1
      .azmux        = lo,
      .oob          = true,
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
  }

  else if( strcmp( sa->input, "ratio") == 0 ) {

    assert( 0);
  }

  else {
    assert( 0);
  }
}




static void sa_compile_terms_noaz_lo_first( sa_state_t *sa /* , const char *sbool noaz, bool oob  */)
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
      .oob          = true,                     // use fast/constant aperture
      .zgjc         = true,                     // set zgjc here, also cm-dither, zero for noaz.
      .next_idx     = 1,
      },
      { // 1
      .azmux        = lo,
      .oob          = true,
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
  }

  else if( strcmp( sa->input, "ratio") == 0 ) {

    assert( 0);
  }

  else {
    assert( 0);
  }

}




static decode_t *decode_singleton_create( uint32_t id)
{
  /*
    this is a bit ugly.
    - but declaring as a struct in sa_t means we loses constness for sa. and for the mode.
    also the local decode_t definition must be exposed in the sa.h. header .
    also name clashes with the deocode_t module.
    --
    most important - we lose ability to vary the type to specialize the strategy/policy handling.
  */

  UNUSED( id);

  decode_t *p =  malloc( sizeof( decode_t));
  assert( p);
  return p;
}

/*
  - consider if possible issue with synchronization of state updates from REPL here with receiving data.
  - the mode can be modified before it is written to the board.
    so there is potential for the decode handler to be out of synch, with sequencer running on the analog board.
  ----
  - but the analog board - is always written after REPL. command. without handling data.

*/

static void sa_compile_terms( sa_state_t *sa)
{
  printf("sa compile terms\n");

  /*
    with sa_t injected into decode_t on init()...
    - we could use the  flags { noaz, aggregate } to correctly
      decode values.
    - instead of using strategy function pointers
    - not clear how we should do this.
  */

  // cannot rebuild sequence terms.
  // unless know all the arguments...
  if( strlen( sa->input) == 0)
    return;



  // the oob ctx
  sa->ctx_oob       = decode_singleton_create( 0);
  decode_init( sa->ctx_oob, sa);

  // the normal ctx
  sa->ctx_normal    = decode_singleton_create( 1);
  decode_init( sa->ctx_normal, sa);

  // the second ctx
  sa->ctx_second    = decode_singleton_create( 2);
  decode_init( sa->ctx_second, sa);

  // note how normal and second, both use normal function


  // set the oob handler
  sa->decode_oob    = (void (*)( void *, data_t *)) decode_az_hi_first;


  //
  if( sa->noaz) {

    sa_compile_terms_noaz_lo_first( sa);

    sa->decode_normal = (void (*)( void *, data_t *)) decode_noaz_lo_first;
  }
  else {

    sa_compile_terms_az_hi_first( sa);

    if( !sa->aggregate) {

      sa->decode_normal = (void (*)( void *, data_t *)) decode_az_hi_first;

    } else {

      sa->decode_normal = (void (*)( void *, data_t *)) decode_az_hi_first_aggregate;

    }
  }


}


//////////////////



void sa_set_input( sa_state_t *sa, const char *s )
{
  // TODO  consider rename  sa_set_input_input()

  assert( strcmp( s, "0") == 0
    || strcmp( s, "ch1") == 0
    || strcmp( s, "ch2") == 0
    || strcmp( s, "ratio") == 0);


  strncpy( sa->input, s, sizeof( sa->input));
  sa->input[ sizeof( sa->input) - 1] = 0;

  sa_compile_terms( sa);
}








void sa_trig_delay_set( sa_state_t *sa, uint32_t u)
{
  // called externally - by cal/transer rourintes
  sa->p_trig_delay = u;
}






bool sa_repl_statement( sa_state_t *sa, const char  *cmd)
{

  assert( sa);

  // printf("bool sa_repl_statement()\n   %s", cmd);

  char s0[ 100 + 1];
  uint32_t u0; // , u1;



  if( strcmp(cmd, "noaz") == 0) {

    sa->noaz = true;
    sa_compile_terms( sa);
  }

  else if(strcmp(cmd, "az") == 0) {

    sa->noaz = false;
    sa_compile_terms( sa);
  }
  else if( strcmp(cmd, "azero %100s") == 0
    && str_decode_uint( s0, &u0))  {

    // support 3458a. syntax, AZERO ON/OFF
    sa->noaz = !u0;
    sa_compile_terms( sa);
  }


  else if(
    strcmp(cmd, "sa 0") == 0
    || strcmp(cmd, "sa ch1") == 0
    || strcmp(cmd, "sa ch2") == 0
    || strcmp(cmd, "sa ratio") == 0
    ) {

    sa_set_input( sa, cmd);
    sa_compile_terms( sa);
  }


  else if( sscanf(cmd, "sa aggregate %100s", s0) == 1
    && str_decode_uint( s0, &u0))
  {

    sa->aggregate = u0;
    sa_compile_terms( sa);
  }




  //////////

  else if((sscanf(cmd, "sa precharge %100s", s0) == 1)
    && str_decode_uint( s0, &u0))  {

    // TODO consider use time, rather than raw count
    sa->p_precharge = u0;
  }

  else if((sscanf(cmd, "sa trig delay %100s", s0) == 1)
    && str_decode_uint( s0, &u0))  {

    // TODO consider use time, rather than raw count
    sa->p_trig_delay = u0;
  }



  // buffer verbosity
  if( sscanf(cmd, "sa verbose %u", &sa->verbose) == 1) {

  }

  else
    return false;


  return true;
}

