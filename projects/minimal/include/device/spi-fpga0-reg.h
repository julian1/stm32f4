
#pragma once

/*
  fpga specific structure -  for comms

*/



// spi device cs encodin
#define SPI_CS_DEASSERT                   0
#define SPI_CS_FPGA0                      1
#define SPI_CS_4094                       2
#define SPI_CS_MDAC0                      3
#define SPI_CS_MDAC1                      4



// place OE in own separate register, rather than general combined _CR_control register.
// because use once only at config time

#define REG_4094_OE                       9
#define REG_CR                            12
#define REG_DIRECT                        14


// consider rename SR ?
#define REG_STATUS                        17


// operation modes
#define MODE_DIRECT                       0     // mcu can control outputs directly by writing the direct register
#define MODE_ADC_MUX_REF_TEST             5     // test integrator, refmux switching
#define MODE_SA_MOCK_ADC                  6     // sample acquisition, but adc is mocked
#define MODE_SA_ADC                       7     // normal operation. with sa and adc



///////////////////////
// sample acquisition control parameters
#define REG_SA_P_CLK_COUNT_PRECHARGE      20

#define REG_SA_P_SEQ_N                    21
#define REG_SA_P_SEQ0                     22
#define REG_SA_P_SEQ1                     23
#define REG_SA_P_SEQ2                     24
#define REG_SA_P_SEQ3                     25



///////////////////////
// adc control parameters
#define REG_ADC_P_CLK_COUNT_APERTURE      30
#define REG_ADC_P_CLK_COUNT_RESET         31



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






/*
  Feb. 2026
  - more important than the seq. idx.   we should return the azmux value and may be pc value that was used.
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

    EXTR. can use first to clear buffers. after a trigger.

*/



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

  uint8_t   hw_flags    : 4;
  uint8_t               : 4;   // 16

  uint8_t   sample_idx  : 3;
  uint8_t   first       : 1;
  uint8_t   sample_seq_n : 3;
  uint8_t               : 1;   // 24


  uint32_t              : 8;    // 31


  // uint32_t   azmux : 4;
  // uint32_t   pc : 2;

} reg_sr_t;

_Static_assert (sizeof(reg_sr_t) == 4, "bad typedef size");







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
} seq_elt_t;

_Static_assert (sizeof(seq_elt_t) == 4, "bad typedef size");








