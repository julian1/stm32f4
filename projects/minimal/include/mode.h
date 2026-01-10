
#pragma once

#include <stdbool.h>
#include <stdint.h>


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
  muxes and relays are part of 4094 system. nothing to do with fpga.
  and should not be associated in headers

  - better name MUX_S1 ?   REL_SET.

  - important we can actually put the dac state register here also.

*/

// 1of8 muxes.
#define SOFF        0
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

#define S1          ((1-1)<<1|0b1)
#define S2          ((2-1)<<1|0b1)
#define S3          ((3-1)<<1|0b1)
#define S4          ((4-1)<<1|0b1)
#define S5          ((5-1)<<1|0b1)
#define S6          ((6-1)<<1|0b1)
#define S7          ((7-1)<<1|0b1)
#define S8          ((8-1)<<1|0b1)





/*
  if we had put the enable pin first, on each 4094.
  then the same codes could be used for both.

*/

// dual 1of 4 muxes.
#define DOFF        0
/*
#define D1          ((1<<2)|(1-1))    // 4.
#define D2          ((1<<2)|(2-1))    // 5
#define D3          ((1<<2)|(3-1))    // 6
#define D4          ((1<<2)|(4-1))    // 7
*/
#define D1          ((1-1)<<1|0b1)
#define D2          ((2-1)<<1|0b1)
#define D3          ((3-1)<<1|0b1)
#define D4          ((4-1)<<1|0b1)


// better prefix - SR for set/reset
#define SR_SET      0b01
#define SR_RESET    0b10




#define SW_PC_BOOT        0
#define SW_PC_SIGNAL      1







typedef struct _4094_state_t
{

  // U401
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
  // uint8_t       : 1;

  // from amplifier
  // u421
  uint8_t U419 : 4;
  uint8_t U420 : 4;


  /////////


  // u1004
  uint8_t U1003 : 4;      // 4 bit mux.
  uint8_t U1012 : 4;      //

  // u1008
  uint8_t U1009 : 4;
  uint8_t U1010 : 4;


  // u606
  uint8_t U605  : 4;
  uint8_t U610  : 3;
  uint8_t       : 1;

  // u607
  uint8_t       : 2;
  uint8_t U608_SW : 1;
  uint8_t       : 1;
  uint8_t K602  : 2;
  uint8_t       : 2;



  // u713
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
  uint8_t U706  : 4;


  // u709
  uint8_t U703  : 4;
  uint8_t       : 4;






} _4094_state_t;



//////////




typedef struct  __attribute__((__packed__))
reg_direct_t
{
  /* this is direct mode state.
      in adc mode,   we would have 2 or four sets of mux registers for the values to switch.
      and the other lines would be given to the fpga to run
      ----
      and this state would be written in the main mode state.
      -------
  */


  uint8_t   leds_o     : 4;               // 0
  uint8_t   monitor_o  : 8;               // 4

/*
  - EXTR. TODO use spearate vars for the precharge switches.
  - to provide separate control over both switches.
  - will be much cleaner

*/

  // use two bit representation - to be consistent with az. sequen
  // uint8_t   pc_o : 2;                       // 14
  uint8_t   pc_ch1_o : 1;                 // 14
  uint8_t   pc_ch2_o : 1;                 // 15


  uint8_t   azmux_o : 4 ;                 // 16

  uint8_t   adc_refmux_o : 4;                   // 21     // better name adc_refmux   adc_cmpr_latch
  uint8_t   adc_cmpr_latch_o : 1;          // 20

  uint8_t   spi_interrupt_ctl_o : 1;      // 12
  uint8_t   meas_complete_o : 1;          // 13



  uint8_t   dummy_bits_o : 7;               // 25 = (32-25)  TODO. make anonymous

/*
471     .out( {   dummy_bits_o,               // 25
472               meas_complete_o,          // 24+1     // interupt_ctl *IS* generic so should be at start, and connects straight to adum. so place at beginning. same argument for meas_complete
473               spi_interrupt_ctl_o,      // 23+1     todo rename. drop the 'ctl'.
474               adc_cmpr_latch_o,         // 22+1
475               adc_refmux_o,             // 18+4     // better name adc_refmux   adc_cmpr_latch
476               azmux_o,                  // 14+4
477               sig_pc_ch_o,              // 12+2
478               monitor_o,                // 4+8
479               leds_o                    // 0+4

*/
} reg_direct_t;




// need somehting like this...
// instead of the left-shift bits.
// similarly.

typedef struct seq_elt_t
{
  uint32_t    azmux : 4;
  uint32_t    pc    : 2;

  uint32_t          : 26;
} seq_elt_t;   // size 32




// sequence acquisition

typedef struct sa_state_t
{

  uint32_t p_clk_count_precharge ;   // p_cc_precharge

  // number of phase
  // rename? - opportunity for confusion, suffix of _n in verilog indicates  inverted signal.
  uint32_t p_seq_n;

  seq_elt_t p_seq_elt [ 4] ;


  // remove. aug. 2025
  // uint32_t p_trig;


} sa_state_t;





typedef struct adc_state_t
{


  /*
       ----
      reg_direct_t reg_direct2; only for a ratiometric function.
      No. it's only the azmux. that needs a different value.
  */

  // 'p' implies its a paraameter
  // could prefix with cc. clock_count

  uint32_t  p_aperture;     // p_cc_aperture
  uint32_t  p_reset;        // p_cc_reset


} adc_state_t;




/*
  Can use,
  - 1. pre-constructed bit-vectors for state - for different modes. like K. services manuals.
  - 2. or construct on an as need basis.
  - 3. or derive from another mode, by copying and modifying.

*/



typedef struct spi_t  spi_t;


// prefix with underscore because 'mode_t'  is in conflict with sys/types 'mode_t'.

typedef struct _mode_t
{
  // all state needed to achive a dmm function.
  // but not enough for different states.

/*
  // TODO add 4094 to name.
      eg. _4094_first _4094_second .

      or shift_reg_first, serial_first, serial second
*/

  _4094_state_t     first;

  _4094_state_t     second;



  // could factor to a struct, but only one value.
  uint16_t mdac0_val;

  uint16_t mdac1_val;

  // name is confusing, because this is ice40 mode,  while the app mode, is app mode represents all app state.
  // perhap rename reg_ice40_mode
  uint32_t  reg_mode;

  // not explicitly an adc parameter.  signal acquisition or  adc.
  // only when fpga is in mode 0
  reg_direct_t    reg_direct;


  // uint32_t  reg_seq_mode;


  sa_state_t    sa;


  adc_state_t    adc;

  bool         trigger_selection;     // stm32. state

} _mode_t ;






typedef struct devices_t devices_t;

void spi_mode_transition_state( devices_t *devices, const _mode_t *mode, volatile uint32_t *system_millis /*, uint32_t update_flags */);



/*

  a light set of functions to help with common mode settings

  consider - how useful these setters are.
  versus -simply coding the mode fields/flags in-place.
  perhaps useful for repl.
  -
  sometime - we just need to flip a single mux - and then these obscure the simple action.
  -
  consider change name dcv1-source perhaps.
  actually implied. because dcv2 is always constant function of dcv1.  - eg. inverted.
*/


// TODO - consider remove the _set_

void mode_set_amp_gain( _mode_t *mode, uint32_t u);





void mode_set_seq( _mode_t *mode, uint32_t seq_mode , uint8_t arg0, uint8_t arg1 );



bool mode_repl_statement( _mode_t *mode,  const char *cmd, uint32_t line_freq );



/*
  helper functions to select channel inputs

  for clarity and test code, better than manipulating underlying relays
  and does a reasonable job to abstract pinouts.

  consider if encoding and passing an argument may better
  ------
  issue is encoding in the interpreter.
    set ch1  off
    set ch1  dcv
*/



void mode_lts_set( _mode_t *mode, double f0 /*signed i0*/);       // arg is 10,0,-10
void mode_daq_set( _mode_t *mode, unsigned u0, unsigned u1 );   // factor into daq_set and ch2_set
void mode_mdac0_set( _mode_t *mode, unsigned u0 );                // inverter
void mode_mdac1_set( _mode_t *mode, unsigned u0 );                // isolated sts dac.




// void mode_invert_set( _mode_t *mode, bool u0);    // off/on.


void mode_ch1_reset(_mode_t *mode);
void mode_ch1_set_dcv(_mode_t *mode);
void mode_ch1_set_dcv_source(_mode_t *mode);    // change name lts eg.  using input relay



void mode_ch2_reset(_mode_t *mode);
void mode_ch2_set_ref( _mode_t *mode);
void mode_ch2_set_ref_lo( _mode_t *mode);
void mode_ch2_set_temp( _mode_t *mode );
void mode_ch2_set_lts(_mode_t *mode);
void mode_ch2_set_daq( _mode_t *mode );
void mode_ch2_set_shunts(_mode_t *mode);
void mode_ch2_set_tia( _mode_t *mode );
void mode_ch2_set_sense(_mode_t *mode);
void mode_ch2_set_dcv_div(_mode_t *mode);




void mode_ch1_accum( _mode_t *mode, bool);
void mode_ch2_accum( _mode_t *mode, bool);



// void mode_set_accum( _mode_t *mode, bool val);





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





