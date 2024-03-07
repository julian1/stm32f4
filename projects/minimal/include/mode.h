
#pragma once


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
#define S1          ((1<<3)|(1-1))
#define S2          ((1<<3)|(2-1))
#define S3          ((1<<3)|(3-1))
#define S4          ((1<<3)|(4-1))
#define S5          ((1<<3)|(5-1))      // 12
#define S6          ((1<<3)|(6-1))      // 13
#define S7          ((1<<3)|(7-1))
#define S8          ((1<<3)|(8-1))


// dual 1of 4 muxes.
// S for switch maybe SS ? or W
#define WOFF        0
#define W1          ((1<<2)|(1-1))
#define W2          ((1<<2)|(2-1))
#define W3          ((1<<2)|(3-1))
#define W4          ((1<<2)|(4-1))


/*
// relay.  TODO better name  LR_SET, LR_RESET.
#define LR_OFF      0
#define LR_RESET      0b01      // bottom contacts closed.
#define LR_SET      0b10      // top contacts closed.

RSET.   RRESET.  relay set. and relay reset.
reset is schem default contacts.

 LR_SET  ==  LR_BOT      0b01      // bottom contacts closed.
 LR_RESET == LR_TOP      0b10      // top contacts closed.



*/

#define LR_SET      0b01
#define LR_RESET    0b10




typedef struct _4094_state_t
{


  // U401
  uint8_t U401_UNUSED : 6;
  // uint8_t U402        : 2;
  // uint8_t U403			  : 2;
  uint8_t K404        : 2;


  uint8_t U402_UNUSED : 8;


  // U414
  uint8_t K401 : 2;
  uint8_t K406 : 2;
  uint8_t K405 : 2;
  uint8_t U414_UNUSED : 2;


  // U415
  uint8_t U415_UNUSED : 6;
  uint8_t K407        : 2;

  // 500.
  // 600.


/*
  // U705
  uint8_t K701 : 2;
  uint8_t U705_UNUSED : 6;

  uint8_t U706_UNUSED : 8;
  uint8_t U709_UNUSED : 8;
*/


  // u1004
  uint8_t U1003 : 4;      // 4 bit mux.
  uint8_t U1006 : 4;

  // U1008.
  uint8_t U1009 : 4;
  uint8_t U1008_UNUSED : 4;

  // U1017
  uint8_t U1010 : 4;
  uint8_t U1012 : 4;

} _4094_state_t;



//////////

/*
      SPI_INTERUPT_OUT,
      MEAS_COMPLETE_CTL,
      CMPR_LATCH_CTL,
      adcmux,                  // 19 bits.

      monitor,                // 15.    bit 14 from 0.. + 8= j    bit 10,    1024.
      LED0,                   // bit 13.  8192.
      SIG_PC_SW_CTL,
      himux2,              // remove the himux2  12.
      himux,
      azmux

  so put in direct mode. then try to bllink the led.
  so try to blink
  assert( sizeof(F) == 4);
*/



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
/*
  uint8_t azmux   : 4;
  uint8_t himux   : 4;
  uint8_t himux2  : 4;     // 12
  uint8_t sig_pc_sw_ctl : 1;
  uint8_t led0    : 1;       // bit 13.   14   2 bytes.

  uint8_t monitor : 8;    // bit 14. 15 // this bit vector overflows - so gets aligned on a new byte boundary. which is not what we want...

  uint8_t adcmux : 4;     // 26
  uint8_t cmpr_latch_ctl : 1;
  uint8_t meas_complete_ctl : 1;    // // perhaps change name meas_valid,  or sa_valid.  to reflect the trig,valid control interface.
  uint8_t spi_interupt_ctl : 1;     // 29bits

  uint8_t dummy   : 3;
*/



  uint8_t   leds_o     : 4;               // 0
  uint8_t   monitor_o  : 8;               // 4
  uint8_t   spi_interrupt_ctl_o : 1;      // 12
  uint8_t   meas_complete_o : 1;          // 13
/*
  uint8_t   sig_pc1_sw_o : 1;             // 14
  uint8_t   sig_pc2_sw_o : 1;             // 15 */
  uint8_t   sig_pc_sw_o : 2;             // 14 
  uint8_t   azmux_o : 4 ;                 // 16
  uint8_t   adc_cmpr_latch_o : 1;          // 20
  uint8_t   adc_refmux_o : 4;                   // 21     // better name adc_refmux   adc_cmpr_latch
  uint8_t   dummy_bits_o : 7;               // 25

} reg_direct_t;





#define SW_PC_SIGNAL    1
#define SW_PC_BOOT      0







typedef struct adc_state_t
{
  /*
       ----
      reg_direct_t reg_direct2; only for a ratiometric function.
      No. it's only the azmux. that needs a different value.
  */

  // 'p' implies its a paraameter
  // prefix with cc. clock_count
  
  uint32_t  reg_adc_p_aperture;     // regadc_cc_aperture
  uint32_t  reg_adc_p_reset;        // regadc_cc_reset


} adc_state_t;




typedef struct sa_state_t
{
 
  uint32_t  reg_sa_p_clk_count_precharge ;   // regadc_cc_precharge

} sa_state_t;


/*
  Can use,
  - 1. pre-constructed bit-vectors for state - for different modes. like K. services manuals.
  - 2. or construct on an as need basis.
  - 3. or derive from another mode, by copying and modifying.

*/


// note that 'mode_t'  is in conflict with sys/types 'mode_t'.

typedef struct _mode_t
{
  // all state needed to achive a dmm function.
  // but not enough for different states.

  _4094_state_t     first;

  _4094_state_t     second;



  /*
     other app specific  state, that must be persisted.
      and not managed by 4094 or fpga.
    eg. for range-changin. etc.
*/

  // could factor to a struct, but only one value.
  uint16_t dac_val;


  // confusinig name.   perhaps change name to AF. _alternative_function
  // or reg_ice40_mode
  uint32_t  reg_mode;

  // not explicitly an adc parameter.  signal acquisition or  adc.
  reg_direct_t    reg_direct;


  adc_state_t    adc;

  sa_state_t    sa;


} _mode_t ;


// change name spi_spi_mode_transition_state
// because spi is the first name
void spi_mode_transition_state( uint32_t spi, const _mode_t *mode, volatile uint32_t *system_millis /*, uint32_t update_flags */ );











#if 0




  // U406 4094.
  uint8_t U408_SW_CTL : 1;      // perhaps change to lower case....   eg. u408_sw
  uint8_t U406_UNUSED : 1;
  uint8_t K406_CTL    : 2;        // Be better to encode as 2 bits.   can then assign 0b01 or 0b10 etc.
  uint8_t U406_UNUSED_2 : 4;

  // U401 4094
  uint8_t U401_UNUSED : 8;    // controls U404. boot. mux
                              //

  // input muxing
  // U403
  uint8_t K403_CTL    : 2;
  uint8_t K401_CTL    : 2;
  uint8_t K402_CTL    : 2;
  uint8_t K405_CTL    : 2;


  // U506
  uint8_t U506        : 3;        // adg1209.  gain mux. (MUST BE BROUGHT UP with enable-pin enabled otherwise power supplies current limit, risk of part damage).
  uint8_t U500_UNUSED : 5;        // change name _500_unused --- or similar.  think we should prefix U505_MUX. or


  // ohms current current
  // U606
  uint8_t U605        : 4;    // adg1208.
  uint8_t U606_UNUSED : 3;        // change name _500_unused --- or similar.  think we should prefix U505_MUX. or
  uint8_t LINE_SENSE_OE : 1;

  // U607
  uint8_t K603_CTL    : 2;
  uint8_t K601_CTL    : 2;
  uint8_t K602_CTL    : 2;
  uint8_t U607_UNUSED : 2;


  // current.
  // U704
  uint8_t U704_UNUSED : 2;
  uint8_t K702_CTL    : 2;
  uint8_t K703_CTL    : 2;
  uint8_t K709_CTL    : 2;

  // U705
  uint8_t K707_CTL    : 2;
  uint8_t K706_CTL    : 2;
  uint8_t K704_CTL    : 2;
  uint8_t K705_CTL    : 2;

  // U706
  uint8_t U703        : 4;    // adg1208.
  uint8_t U702        : 3;    // adg1209.
  uint8_t U706_UNUSED : 1;


  // dcv source
  // U1004  - 5th bit.
  uint8_t U1003   : 4;    // adg1208  4 bits.  EN last. is inconsistent.  with chip pin-order. and 500, 600, 700.  good keep.....
  uint8_t U1006   : 4;    // adg1208  4 bits.

#endif
  /////////////////////////////////////
  // put . FPGA STATE IN  HERE/
  // fpga MODE.

  // THE REASON TO NOT CONSIDER DOING THIS - is that do_state transition always pulses the relatys.
  // BUT. can still always control fpga directly after setting up the major state.

  /////////////////////////////////////

  // put this in another structure.
  // to make it clear. it is fpga state.

