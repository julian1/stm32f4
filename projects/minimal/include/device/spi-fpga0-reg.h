
#pragma once


#include <stdint.h>

/*
  fpga specific registers and structure

*/



// spi device cs
#define SPI_CS_DEASSERT                   0
#define SPI_CS_FPGA0                      1
#define SPI_CS_4094                       2
#define SPI_CS_MDAC0                      3
#define SPI_CS_MDAC1                      4




// keep 4094 OE in separate register, not _CR_ combined register.
// because only used once at config time
#define REG_4094_OE                       9
#define REG_CR                            12
#define REG_SR                            17


///////////////////////
// sample acquisition control parameters
#define REG_SA_P_CLK_COUNT_TRIG_DELAY     19
#define REG_SA_P_CLK_COUNT_PRECHARGE      20

#define REG_SA_P_SEQ_N                    21
#define REG_SA_P_SEQ0                     22
#define REG_SA_P_SEQ1                     23
#define REG_SA_P_SEQ2                     24
#define REG_SA_P_SEQ3                     25

#define REG_SA_SEQ_ELT                    26


///////////////////////
// adc control parameters
#define REG_ADC_P_CLK_COUNT_APERTURE      30
#define REG_ADC_P_CLK_COUNT_RESET         31
#define REG_ADC_P_CLK_COUNT_APERTURE_OOB  32


// adc reading counts
#define REG_ADC_CLK_COUNT_REFMUX_NEG      40
#define REG_ADC_CLK_COUNT_REFMUX_POS      41
#define REG_ADC_CLK_COUNT_REFMUX_BOTH     42
#define REG_ADC_CLK_COUNT_RSTMUX          43
#define REG_ADC_CLK_COUNT_SIGMUX          44
#define REG_ADC_CLK_COUNT_APERTURE        45


// adc reading extra stat counts
#define REG_ADC_STAT_COUNT_REFMUX_POS_UP  50
#define REG_ADC_STAT_COUNT_REFMUX_NEG_UP  51
#define REG_ADC_STAT_COUNT_CMPR_CROSS_UP  52


#define REG_TEST1                         60
#define REG_TEST2                         61






typedef struct __attribute__((__packed__))
reg_cr_t
{

  // sequencer mode. 3 bits
  uint8_t     sa_mode          : 3;


  /* adc - whether to switch the input sigmux
    used for initial cal weighting

    consider move seq_elt_t
    or place in a common structure
  */
  uint8_t     adc_p_active_sigmux : 1;

  // for az mode
  // uint8_t     sa_p_noaz     : 1;

  // uint8_t     sa_p_use_aperture_oob : 1;

 // input     p_use_slow_rundown,
 // input     p_use_fast_rundown,

  uint32_t    dummy_bits_o  : 28;

} reg_cr_t;

_Static_assert (sizeof(reg_cr_t) == 4, "bad typedef size");





typedef struct __attribute__((__packed__))
reg_sr_t
{

  // hw flags do not belong here. since constant across power cycles.

  // interrupt source
  // almost an isr
  struct {

    uint8_t   magic         : 4;      // TODO, change back to 8 bits. and move out of isr.

    uint8_t   adc           : 1;
    // consider no longer use
    uint8_t   cmpr          : 1;
    uint8_t                 : 2;  // 8
  } isr;


  struct {

    uint8_t   idx           : 3;
    /* we need first...
      to know to clear the buffers... after retrigger
    */
    uint8_t  first         : 1;

    uint8_t                : 4;     //  16
  } sample;



  // comparator flags
  struct {
    uint8_t   amp_zero_lt   : 1;
    uint8_t   amp_zero_gt   : 1;

    uint8_t   amp_ovld_lt   : 1;
    uint8_t   amp_ovld_gt   : 1;

    uint8_t   amp_unld_lt   : 1;
    uint8_t   amp_unld_gt   : 1;

    uint8_t   boot_ch1_lt   : 1;
    uint8_t   boot_ch1_gt   : 1;      // 24


    uint8_t   boot_ch2_lt   : 1;
    uint8_t   boot_ch2_gt   : 1;      // 26

    uint8_t                 : 6;

  } cmpr;                             // 32

} reg_sr_t;





_Static_assert (sizeof(reg_sr_t) == 4, "bad typedef size");





typedef struct __attribute__((__packed__))
seq_elt_t
{

/*
    extr. instead of always having a next_id.
    consider add code and use union to represent different elt types

    - can then distinguish a seq_elt from insn like behavior
    such as jmp/goto  with a pc or idx

    Note. for rv32.  rv32 opcode is only 7 bits. only bits 0-6
*/

  // uint32_t    code       : 4;

  uint32_t    azmux         : 4;

  // uint32_t    pc_protect            : 2;   // during protect/azmux switch phase.
  uint32_t    pc            : 2;              // during sample/adc phase. TODO. consider renmae


  /* encoding next_idx like a linked list
    means can remove two registers - the seq_n terminal, and wrap-around idx value.
    - must support noaz, where return to the same idx.
  */
  uint32_t    next_idx      : 3;   // 9


  // flags for decode
  uint32_t    hi            : 1;        // TODO. bad name.  hi == input signal/sample. or zero
  uint32_t    convert       : 1;        // convert to reading on this input


  /////////////////////////////

  // control/ sequencing and other
  /*
      consider a common structure to localize adc control
      to run independently of the sequencer

  */
  uint32_t    oob_aperture  : 1;        // oob.   use oob aperture.
  // uint32_t    oob_modulo : 1;        // action only on modulo count


  /*
    consider move active_sigmux flag here - and out of the generalized control register.
    so adc can operate independently of sequencer then it may make sense.
  */
  // uint8_t     adc_p_active_sigmux : 1;

  uint32_t    dither_cm_dac : 1;
  uint32_t    dither_runup  : 1;      // 14


  /*
      not unreasonably to encode leds here...
      although sample_idx. may be good enough
  */
  // uint32_t    leds    : 4;          // 18

  // ignore/nop - convenience for software/side - but would contribute hardware complexity
  // uint32_t ignore : 1;




  uint32_t          : 18;
} seq_elt_t;

_Static_assert (sizeof(seq_elt_t) == 4, "bad typedef size");








  /*
    apr 2026

    consider - flags returned in SR register. to decode
    or some user bits - passed blindly from az sequencer into the SR.
    is_hi
    is_oob
    convert_on_receive
    ----

    can even encode monitor, or leds here.
    for use when az-sequencer is in holding pattern

    it makes sense - because the az-sequencer is the driver of most of these of these fields.
    in normal operation

  */




#if 0
/*
  apr 2026
  reg_direct can be removed.
  instead use the az-sequencer with a separate state pattern - that holds the azmux,pc,leds,montior
  in order to use for tests/ etc.

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


  uint8_t   leds_o            : 4;               // 0
  uint8_t   monitor_o         : 8;               // 4

/*
  - EXTR. TODO use spearate vars for the precharge switches.
  - to provide separate control over both switches.
  - will be much cleaner

*/

  // should be the same as seq_elt_t
  uint8_t   pc_o              : 2;                // 14
  // uint8_t   pc_ch1_o       : 1;                // 14
  // uint8_t   pc_ch2_o       : 1;                // 15


  uint8_t   azmux_o           : 4 ;               // 16

  uint8_t   adc_refmux_o      : 4;                // 21     // better name adc_refmux   adc_cmpr_latch
  uint8_t   adc_cmpr_latch_o  : 1;                // 20

  uint8_t   spi_interrupt_ctl_o : 1;              // 12

  uint8_t                     : 1;                // 13     was meas_complete_o



  uint8_t                     : 7;                // 25 = (32-25)  TODO. make anonymous
} reg_direct_t;

_Static_assert (sizeof(reg_direct_t) == 4, "bad typedef size");
#endif





#if 0
/*
  output reg [ 2-1:0]  pc_sw_o,
  output reg [ 4-1:0]  azmux_o,
  output reg  [3-1: 0] sample_idx_o,
  output reg           sample_first_o,

  the difficulty with controlling the outputs directly
  is to be able to get a converstion from adc.
  and get the two samples to construct a reading.

  -
  Think that using the sequence[ 0 ]  register
  to control pc,azmux directly
  would be better here.

  do we really need control over sample_idx???

  -------------

  just toggle sample_idx_o...  directly so the decoder can work it out.
  OR.  toggle the HI,LO, COMPUTE flags. associated with each sample
  it may make sense... to do a LO-first. and then measure the output.

  - it is not even clear  that using direct is the best way to do this.

    could still use the sequence registers...
*/


typedef struct __attribute__((__packed__))
reg_direct_t
{

  uint8_t pc_sw_o           : 2;

  uint8_t azmux_o           : 4;

  uint8_t sample_idx_o,     : 3;

  /* really looks wrong. remove
  // aperture should be controlled directly by sequencer
  // and flagged.
  */
  uint8_t sample_first_o    : 1;    // 10


  // something very strange here...
  // uint32_t    dummy_bits_o  : 13 ;    // fails
  uint32_t                  : 12 ;    //  12 + 10 == 22 bits.  anything more overflows? OK

} reg_direct_t;

_Static_assert (sizeof( reg_direct_t) == 4, "bad typedef size");

#endif
















#if 0



  /*
  471     .out( {   dummy_bits_o,               // 25
  472               meas_complete_o,          // 24+1     // interrupt_ctl *IS* generic so should be at start, and connects straight to adum. so place at beginning. same argument for meas_complete
  473               spi_interrupt_ctl_o,      // 23+1     todo rename. drop the 'ctl'.
  474               adc_cmpr_latch_o,         // 22+1
  475               adc_refmux_o,             // 18+4     // better name adc_refmux   adc_cmpr_latch
  476               azmux_o,                  // 14+4
  477               sig_pc_ch_o,              // 12+2
  478               monitor_o,                // 4+8
  479               leds_o                    // 0+4

  */


/*
  Feb. 2026
  - more important than the seq. n.

      consider return azmux value used and perhaps be pc value that was used.
      can then test this against S1, S2. etc   to know if the value is a hi or lo value

    ie. for n==2
    S1,S3 == hi
    S5,S6,S7 == lo.

    no.

        just use the idx knowing the first value is always a zero.
        and the first and third are zero for ratiometric when seq_n = 4;

    if n == 2.  and idx == 0 then value is a zero.
                 idx == 1 then hi.
      if n == 4 ratiometric then as above.
          and 2 is lo, and 3 is hi.

    EXTR. can use first to clear buffer. after a trigger.

*/



/*
// operation modes
#define MODE_DIRECT                       0     // mcu can control outputs directly by writing the direct register
#define MODE_ADC_MUX_REF_TEST             5     // test integrator, refmux switching
#define MODE_SA_MOCK_ADC                  6     // sample acquisition, but adc is mocked
#define MODE_SA_ADC                       7     // normal operation. with sa and adc

*/

#endif
