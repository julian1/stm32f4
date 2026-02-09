
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
#define REG_MODE                          12
#define REG_DIRECT                        14


// rename SR ?
#define REG_STATUS                        17


// reg_mode values
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
#define REG_ADC_CLK_COUNT_REFMUX_NEG     40
#define REG_ADC_CLK_COUNT_REFMUX_POS     41
#define REG_ADC_CLK_COUNT_REFMUX_BOTH    42
#define REG_ADC_CLK_COUNT_RSTMUX         43
#define REG_ADC_CLK_COUNT_SIGMUX         44



// extra stat counts.
#define REG_ADC_STAT_COUNT_REFMUX_POS_UP 50
#define REG_ADC_STAT_COUNT_REFMUX_NEG_UP 51
#define REG_ADC_STAT_COUNT_CMPR_CROSS_UP  52






/*
  Feb. 2026
  - more important than the seq. idx.   we should return the azmux value and may be pc value that was used.
      can then test this against S1, S2. etc   to know if the value is a hi or lo value

    ie. for n==2
    S1,S3 == hi
    S5,S6,S7 == lo.
*/



// define  - once means can store this in data for other display routines.
// TODO - use macro for mask and shift
// reg_status values

// TODO - consider rename sample to reading  ie. status_reading_idx.  or  status_sa_idx status_sa_azmux sa_pc etc.
#define STATUS_HW_FLAGS(status)         (0b111 & (status >> 8 ))
#define STATUS_SAMPLE_IDX(status)       (0b111 & (status >> 16))
#define STATUS_SAMPLE_SEQ_N(status)     (0b111 & (status >> 20))








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


