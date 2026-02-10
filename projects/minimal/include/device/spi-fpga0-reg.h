
#pragma once

/*
  fpga specific
  low level comms - code does not belong in mode.h
  otherwise would be pulled in by unrelated spi peripheral handling code

*/



// encoding for cs select of spi devices
#define SPI_CS_DEASSERT                   0
#define SPI_CS_FPGA0                      1
#define SPI_CS_4094                       2
#define SPI_CS_MDAC0                      3     // SPI_CS_INVERT_DAC
#define SPI_CS_MDAC1                      4



// place OE in own separate register, rather than general combined _CR_control register.
// because used once only at config time

#define REG_4094_OE                       9
#define REG_CR                            12
#define REG_DIRECT                        14


// rename SR ?
#define REG_STATUS                        17


// fpga operating mode
#define MODE_DIRECT                       0     // mcu spi control of the direct register
#define MODE_ADC_MUX_REF_TEST             5
#define MODE_SA_MOCK_ADC                  6     // no az. and elecm. etc
#define MODE_SA_ADC                       7      // normal operation. with sa and adc



///////////////////////
//  sample acquisition.

#define REG_SA_P_CLK_COUNT_PRECHARGE      20

#define REG_SA_P_SEQ_N                    21
#define REG_SA_P_SEQ0                     22
#define REG_SA_P_SEQ1                     23
#define REG_SA_P_SEQ2                     24
#define REG_SA_P_SEQ3                     25



///////////////////////
// adc parameters


#define REG_ADC_P_CLK_COUNT_APERTURE      30
#define REG_ADC_P_CLK_COUNT_RESET         31


// adc counts
#define REG_ADC_CLK_COUNT_REFMUX_NEG      40
#define REG_ADC_CLK_COUNT_REFMUX_POS      41
#define REG_ADC_CLK_COUNT_REFMUX_BOTH     42
#define REG_ADC_CLK_COUNT_RSTMUX          43
#define REG_ADC_CLK_COUNT_SIGMUX          44
#define REG_ADC_CLK_COUNT_APERTURE        45


// extra stat counts.
#define REG_ADC_STAT_COUNT_REFMUX_POS_UP  50
#define REG_ADC_STAT_COUNT_REFMUX_NEG_UP  51
#define REG_ADC_STAT_COUNT_CMPR_CROSS_UP  52






/*
  Feb. 2026
  - more important than the seq. idx.   we should return the azmux value and may be pc value that was used.
      can then test this against S1, S2. etc   to know if the value is a hi or lo value

    ie. for n==2
    S1,S3 == hi
    S5,S6,S7 == lo.

    no.

        we do not need the azmux value.
    if n == 2.  and idx == 0 then value is a zero.
                 idx == 1 then hi.
      if n == 4 ratiometric then as above.
          and 2 is lo, and 3 is hi.

    EXTR. can use first to clear buffers. after a trigger.

*/



// define  - once means can store this in data for other display routines.
// TODO - use macro for mask and shift
// reg_status values

// TODO - consider rename sample to reading  ie. status_reading_idx.  or  status_sa_idx status_sa_azmux sa_pc etc.

/*

  EXTR.
  FIX THIS - use bitfields...

  just the same as reg_direct... or reg_cr

*/

#if 0

#define STATUS_MAGIC(status)            (0xff  & status)                // { 8'b10101010

#define STATUS_HW_FLAGS(status)         (0b111 & (status >> 8 ))

#define STATUS_SAMPLE_IDX(status)       (0b111 & (status >> 16))

#define STATUS_SAMPLE_FIRST(status)     (0b1 & (status >> 19))

#define STATUS_SAMPLE_SEQ_N(status)     (0b111 & (status >> 20))


// state of the azmux and pc.
#define STATUS_SAMPLE_AZMUX(status)     ()
#define STATUS_SAMPLE_PC(status)     ()

#endif









typedef struct __attribute__((__packed__))
reg_cr_t
{

  uint8_t   mode        : 3;      // only 3 bits

 // input           p_use_slow_rundown,
 // input           p_use_fast_rundown,
 // input           p_use_input_signal,     // adc whether to swtich in the input signal

  uint8_t p_adc_use_input_signal : 1;   // better name ? p_adc_switch_input_signal


  uint32_t   dummy_bits_o : 28;

} reg_cr_t;


_Static_assert (sizeof(reg_cr_t) == 4, "bad typedef size");





typedef struct __attribute__((__packed__))
reg_sr_t
{

  uint8_t   magic ;     // 8

  uint8_t   hw_flags : 4;
  uint8_t   unused : 4;   // 16

  uint8_t   sample_idx : 3;
  uint8_t   first : 1;
  uint8_t   sample_seq_n : 3;
  uint8_t   unused2 : 1;   // 24


  uint32_t   unused3 : 8;    // 31


  // uint32_t   azmux : 4;
  // uint32_t   pc : 2;

} reg_sr_t;


_Static_assert (sizeof(reg_sr_t) == 4, "bad typedef size");


/*

#define STATUS_MAGIC(status)            (0xff  & status)                // { 8'b10101010

#define STATUS_HW_FLAGS(status)         (0b111 & (status >> 8 ))

#define STATUS_SAMPLE_IDX(status)       (0b111 & (status >> 16))

#define STATUS_SAMPLE_FIRST(status)     (0b1 & (status >> 19))

#define STATUS_SAMPLE_SEQ_N(status)     (0b111 & (status >> 20))


// state of the azmux and pc.
#define STATUS_SAMPLE_AZMUX(status)     ()
#define STATUS_SAMPLE_PC(status)     ()
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


_Static_assert (sizeof(reg_direct_t ) == 4, "bad typedef size");







typedef struct seq_elt_t
{
  uint32_t    azmux : 4;
  uint32_t    pc    : 2;

  uint32_t          : 26;
} seq_elt_t;   // size 32

_Static_assert (sizeof(seq_elt_t) == 4, "bad typedef size");









// #define REG_SPI_MUX                    8

///////////////////
// reg spi mux
// note active bits.
/* HERE
// spi mux is one-hot
#define SPI_MUX_NONE          0
#define SPI_MUX_4094          1
#define SPI_MUX_DAC          (1<<1)
#define SPI_MUX_ISO_DAC      (1<<2)
#define SPI_MUX_ISO_DAC2     (1<<3)
*/


// pass-through parameter, for comms.
// - o encode how to decode the sample sequence
// just 4 bits. projected in the status register.
// rename reg_seq_mode_status  perhaps.
// #define REG_SEQ_MODE                   18

// dec 2024.
// #define REG_SA_P_TRIG                     26

// #define STATUS_SAMPLE_SEQ_MODE(status)  (0b111 & (status >> 24) )   // TODO remove
// #define STATUS_SPI_MUX(status)          (0b111 & (status >> 12 ))     // bad name - nothing to do with ADC. do with spi.




//
// fpga has no concept of these - pass-through, so would be good to define in mode, where used.
// but have to be shared to data as well.
// so put here
/*
  // EXTR - but status should be recording the last.
      not what is set in mode.
*/



/*
dec 2024.
  absolutely no reason to try to hard encode all these behaviors

  instead - just create ad-hoc strategy handlers, for data decoding.


#define SEQ_MODE_BOOT         1
#define SEQ_MODE_NOAZ         2
#define SEQ_MODE_AZ           3
#define SEQ_MODE_RATIO        4
#define SEQ_MODE_AG           5
#define SEQ_MODE_DIFF         6
#define SEQ_MODE_SUM_DELTA          7


// #define SEQ_MODE_ELECTRO      3
*/


