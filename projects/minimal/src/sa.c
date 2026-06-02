


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




typedef struct decode_t
{
  // uint32_t    magic;

  // sa_state_t   *sa ;   pointer back to sa.

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




static void decode_reset( decode_t *decode)
{
  assert( decode);

  printf( "clear ");

  decode->hi = 0;
  decode->lo = 0;

  decode->count = 0;

  // decode->adc_sigmux  = 0;    // if use

}



static void decode_noaz_lo_first( decode_t *decode, data_t *data)
{
  assert( decode);

  if( !data)  {

    // important. can use decode specific structure with decode specific reset.
    decode_reset( decode);
    return;
  }
  assert( data && data->magic == DATA_MAGIC);

  const reg_sr_t status = data->status;


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
  }
}




static void decode_az_hi_first( decode_t *decode, data_t *data)
{
  assert( decode);

  if( !data)  {

    // important. can use decode specific structure with decode specific reset.
    decode_reset( decode );
    return;
  }
  assert( data && data->magic == DATA_MAGIC);


  const reg_sr_t status = data->status;


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

    // normalize count
    data->count_sum_norm = count_sum  / data->adc_sigmux;
    data->reading_valid     = true;

    // record/update LO. for next time
    decode->lo = lo;
  }
}



static void decode_az_hi_first_aggregate( decode_t *decode, data_t *data)
{
  assert( decode);

  if( !data)  {

    // important. can use decode specific structure with decode specific reset.
    decode_reset( decode);
    return;
  }
  assert( data && data->magic == DATA_MAGIC);

  const reg_sr_t status = data->status;
  // const term_t term     = data->term;


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

    if( decode->count == 10) {

      // convert value
      double count_sum = decode->hi - decode->lo;

      data->count_sum_norm = count_sum  / ( data->adc_sigmux * decode->count);
      data->reading_valid = true;

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


  if( status.sample.first) {

    // mus reset decode state  for both decoders here
    sa->decode_normal( sa->ctx_normal, NULL);
    sa->decode_oob(    sa->ctx_oob,    NULL);
  }


  if( data->term.oob_aperture) {

    printf("oob, ");
    sa->decode_oob(    sa->ctx_oob,    data);
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
  }

  else if( strcmp( sa->input, "ratio") == 0 ) {

    assert( 0);
  }

  else {
    assert( 0);
  }

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

  // cannot rebuild sequence terms.
  // unless have all the arguments...
  if( strlen( sa->input) == 0)
    return;



  if( sa->noaz) {

    sa_compile_terms_noaz_lo_first( sa);

    sa->decode_normal = (void (*)( void *, data_t *)) decode_noaz_lo_first;
    sa->ctx_normal    = malloc( sizeof( decode_t));
  }
  else {

    sa_compile_terms_az_hi_first( sa);

    if( !sa->aggregate) {

      sa->decode_normal = (void (*)( void *, data_t *)) decode_az_hi_first;
      sa->ctx_normal    = malloc( sizeof( decode_t));

    } else {

      // somehow we have to pass the aggregate amount to the ctx_decode
      sa->decode_normal = (void (*)( void *, data_t *)) decode_az_hi_first_aggregate;
      sa->ctx_normal    = malloc( sizeof( decode_t));

      // sa->ctx_normal.count_n = sa->aggregate;
    }
  }

  // set the oob handler
  sa->ctx_oob       = malloc( sizeof( decode_t));
  sa->decode_oob    = (void (*)( void *, data_t *)) decode_az_hi_first;


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



  if(strcmp(cmd, "noaz") == 0) {

    sa->noaz = true;
    sa_compile_terms( sa);
  }

  else if(strcmp(cmd, "az") == 0) {

    sa->noaz = false;
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



  else
    return false;


  return true;
}

