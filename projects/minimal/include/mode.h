
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <device/spi-fpga0-reg.h>

/*
  EXTR.
    don't care about defining inidividual registers for muxes etc.
    instead the entire state representation is considered as high-level register. with a subset of pre-determined set elements.

    - the clearing mask for relays, is normally always the same. but the need to manipulate b2b fets changes thing.
    - with a straight array.   WE *CAN* also define using a parallel alternative structure with bitfield.
  ----
    - sequencing - may also want to switch relays, wait. then turn on the analog switches.

    - EXTR. THE state of ALL relays must be defined.  0 just means use prior state.  which is wrong.  either L1, or L2.  not 0.

    - It would be easier to do this with a memcpy.

                                      first transition            // second transition
                                      U406    U401
*/

// is wrong. we have to switch all the relays to a defined state



/*
  EXTR. we can do the three step sequence of b2b fets in two steps. relying on faster speed

  1. turn off b2b fets.   turn on relay.   relay slower.
  2. turn off latch to relay. and turn on b2b fets.         <- so this is more than a mask.
  -----------------

  OR.    have two bits. and the interpreter.   will manage

  Eg. b2b-fets stage1. b2b-fets stage2.   - and encode... issue is that noo

  --------
  - So we don't duplicate everything.  just if a relay has a different state.
  - OR we just encode all states - as two states.  transition
  - eg. just double  the bitvector.
  ---
  - then we can encode.   - and don't need messy mask abstractions.

*/





/*
  for hardware - make the enable pin the first bit in representation.
  that way can keep the same encoding for 2x04, and 1x08 muxes.
*/

// 1of8 muxes
#define SOFF        0
#define S1          ((1-1)<<1|0b1)
#define S2          ((2-1)<<1|0b1)
#define S3          ((3-1)<<1|0b1)
#define S4          ((4-1)<<1|0b1)
#define S5          ((5-1)<<1|0b1)
#define S6          ((6-1)<<1|0b1)
#define S7          ((7-1)<<1|0b1)
#define S8          ((8-1)<<1|0b1)


// dual 1 of 4 muxes
#define DOFF        0
#define D1          ((1-1)<<1|0b1)
#define D2          ((2-1)<<1|0b1)
#define D3          ((3-1)<<1|0b1)
#define D4          ((4-1)<<1|0b1)



// relay. consider enum
#define SR_NONE     0b00
#define SR_SET      0b01
#define SR_RESET    0b10


/*
// precharge switches
#define SW_PC_BOOT        0
#define SW_PC_SIGNAL      1

*/



#define MODE_MAGIC   445


typedef struct _4094_state_t
{

  // U401 conditioning
  uint8_t K404  : 2;
  uint8_t K403  : 2;
  uint8_t K405  : 2;
  uint8_t K406  : 2;


  // U402
  uint8_t U409  : 3;
  uint8_t       : 1;
  uint8_t K407  : 2;
  uint8_t K402  : 2;



  // u405
  uint8_t U423 : 3;
  uint8_t U426 : 3;
  uint8_t K401 : 2;


  // to amplifier.
  // U510
  uint8_t       : 4;
  uint8_t U506  : 4;    // adg1208.


  // from amplifier
  // u421
  uint8_t U419 : 4;
  uint8_t U420 : 4;


  /////////


  // u1004 LTS
  uint8_t U1003 : 4;      // 4 bit mux.
  uint8_t U1012 : 4;      //

  // u1008 DAQ/STS
  uint8_t U1009 : 4;
  uint8_t U1010 : 4;


  // u606 OHMS
  uint8_t U605  : 4;
  uint8_t U610  : 3;
  uint8_t       : 1;

  // u607
  uint8_t       : 2;
  uint8_t U608_SW : 1;
  uint8_t       : 1;
  uint8_t K602  : 2;
  uint8_t       : 2;



  // u713 AMPS
  uint8_t K701  : 2;
  uint8_t K704  : 2;
  uint8_t K707  : 2;
  uint8_t       : 2;


  // u705
  uint8_t       : 2;
  uint8_t K702  : 2;
  uint8_t K703  : 2;
  uint8_t       : 2;


  // u706
  uint8_t K708  : 2;
  uint8_t K705  : 2;
  uint8_t K706  : 4;


  // u709
  uint8_t U703  : 4;
  uint8_t       : 4;






} _4094_state_t;



//////////

// clear relay state

void _4094_state_clear_relays(_4094_state_t *state);








// sequence acquisition

typedef struct sa_state_t
{
  uint32_t p_clk_count_trig_delay;  // consider rename p_trig_delay
  uint32_t p_clk_count_precharge;   // consider rename p_precharge   or p_cc_precharge

  // number of phases
  // rename? - opportunity for confusion, suffix of _n in verilog indicates  inverted signal.
  uint32_t p_seq_n;

  seq_elt_t p_seq_elt [ 4] ;


} sa_state_t;


void sa_trig_delay_set( sa_state_t *sa, uint32_t u);




typedef struct adc_state_t
{

  // 'p' implies its a paraameter
  // consider prefix with cc. clock_count

  uint32_t  p_aperture;     // p_cc_aperture
  uint32_t  p_reset;        // p_cc_reset


} adc_state_t;


//   TODO - should be typed on the sa.  not mode.
void adc_aperture_set( adc_state_t *adc, uint32_t u);









// use underscore prefix because 'mode_t'  conflicts with sys/types 'mode_t'.

typedef struct _mode_t
{

  uint32_t      magic;

  // all state needed to achive a dmm function.
  // but not enough for different states.


  _4094_state_t serial;

//   _4094_state_t     second;


  // inverter - rename invert_dac_val
  uint16_t      mdac0_val;

  // sts_dac_val
  uint16_t      mdac1_val;


  // control register
  reg_cr_t      reg_cr;

  // when in mode 0
  reg_direct_t  reg_direct;


  sa_state_t   sa;


  adc_state_t  adc;


  // 1 == internal trigger (mcu) active
  bool         trigger_source;


  // state not written, but chich must persist range change
  // alternatively inject from app as pointer into range.
  uint8_t _10meg_impedance : 1;



} _mode_t ;



void mode_init(_mode_t *mode);
void mode_reset(_mode_t *mode);





// bool mode_repl_statement( _mode_t *mode,  const char *cmd, const uint32_t line_freq);
bool mode_repl_statement(
  _mode_t     *mode,
  const char  *cmd,
  const uint32_t line_freq
);


// typed on reg_cr - is there a better place for this
void reg_cr_mode_set( reg_cr_t *reg_cr, unsigned u0);


bool mode_az_set_relax(_mode_t *mode, const char *s); // for repl
void mode_az_set(_mode_t *mode, const char *s);

void mode_gain_set( _mode_t *mode, uint32_t u);


// ch1.
void mode_ch1_reset(_mode_t *mode);
void mode_ch1_set_dcv(_mode_t *mode);
void mode_ch1_set_dcv_source(_mode_t *mode);    // change name lts eg.  using input relay


// ch2
void mode_ch2_reset(_mode_t *mode);

// set input
bool mode_ch2_set_relax( _mode_t *mode, const char *s0);    // for repl.
void mode_ch2_set( _mode_t *mode, const char *s);



// better name.  set_cap. or something
void mode_ch1_accum( _mode_t *mode, bool);
void mode_ch2_accum( _mode_t *mode, bool);


/////////////////////////

void mode_lts_source_set( _mode_t *mode, double f0 );       // arg is 10,0,-10
void mode_daq_set( _mode_t *mode, unsigned u0, unsigned u1 );   // factor into daq_set and ch2_set

void mode_sts_dac_set( _mode_t *mode, unsigned u0 );
void mode_invert_dac_set( _mode_t *mode, unsigned u0 );





/*
#define S1          ((1<<3)|(1-1))
#define S2          ((1<<3)|(2-1))
#define S3          ((1<<3)|(3-1))
#define S4          ((1<<3)|(4-1))
#define S5          ((1<<3)|(5-1))    // 12
#define S6          ((1<<3)|(6-1))    // 13
#define S7          ((1<<3)|(7-1))
#define S8          ((1<<3)|(8-1))
*/

/*
#define D1          ((1<<2)|(1-1))    // 4.
#define D2          ((1<<2)|(2-1))    // 5
#define D3          ((1<<2)|(3-1))    // 6
#define D4          ((1<<2)|(4-1))    // 7
*/


/*

// could use these - slightly more specific
#define PC-DCV        0b01
#define PC-HIMUX      0b10


//  Seq. elts.  U410
// for both, direct register mode, and normal sample acquisition.
#define AZMUX-DCV     S1
#define AZMUX-LO      S2
#define AZMUX-HIMUX   S3
#define AZMUX-LOMUX   S4
*/

// from dmm16 to dmm17.  DCV changed from S3 to S1.    and CH2 went from S1 to S3. confusing.
// remember azmux is coded with 4 bits, so use S not D.

#if 0
#define AZMUX_CH1_HI  S1    // DCV
#define AZMUX_CH1_LO  S2
#define AZMUX_CH2_HI  S3    // INMUX
#define AZMUX_CH2_LO  S4

#endif


#if 0
#define MODE_CH1_OFF         0
#define MODE_CH1_DCV         1
#define MODE_CH1_DCV_SOURCE  2

void mode_ch1_set( _mode_t *mode, uint32_t val );

#define MODE_CH2_OFF         0
#define MODE_CH2_SENSE       1
#define MODE_CH2_DCI         2
#define MODE_CH2_DCV_DIV     3
#define MODE_CH2_DCV_SOURCE  3

void mode_ch2_set( _mode_t *mode, uint32_t val);
#endif


/*
// not sure if should move to ice40_reg.  or just remove.
// two channels.
// eg. (PC0 | PC01) <<

#define PCOFF       0b00    // both off.

// THIS IS TOO CONFUSING.
// PC1, PC2 is confusing.
#define PC01         0b01
#define PC10         0b10
// #define PCBOTH      0b11 // both on.
*/





