
#pragma once

/*
  could we improve the modlseparation.

  ----
  fpga specific
  this doeesn't belong in mode.h
  because then it would then get pulled in by the spi peripheral code, which needs mode.h.


*/

// better name for 4094 register - CR_  for control register?
// CR_DIRECT ? etc.
// SR_STATUS


#define REG_SPI_MUX                     8
#define REG_4094                        9
#define REG_MODE                        12
#define REG_DIRECT                      14
// pass-through parameter, for comms.
// - o encode how to decode the sample sequence
// just 4 bits. projected in the status register.
#define REG_SEQ_MODE                    18

#define REG_STATUS                      17






///////////////////
// reg spi mux
// note active bits.

// spi mux is one-hot
#define SPI_MUX_NONE      0
#define SPI_MUX_4094      1
#define SPI_MUX_DAC8811   (1<<1)
#define SPI_ISO_CS        (1<<2)
#define SPI_ISO_CS2       (1<<3)



#define MODE_DIRECT       0     // support direct writing via direct register
// #define MODE_LO           1     // all bits held lo. but blink led. default.
// #define MODE_HI           2     // all bits held hi
#define MODE_PATTERN      3     // put modulation pattern on all bits
// #define MODE_PC_TEST      4


// sequence acquisition
#define MODE_SA_MOCK_ADC    6     // no az. and elecm. etc
#define MODE_SA_ADC        7     // no az. and elecm. etc






///////////////////////

//  sample acquisition.
// TODO. remove/rename in favor of ER_TRIGGER_SOURCE_INTERNAL
// instead change to control trigger-sourcce select for internal or external

#define REG_SA_P_CLK_COUNT_PRECHARGE    20

#define REG_SA_P_SEQ_N                  21
#define REG_SA_P_SEQ0                   22
#define REG_SA_P_SEQ1                   23
#define REG_SA_P_SEQ2                   24
#define REG_SA_P_SEQ3                   25



///////////////////////
// adc parameters
#define REG_ADC_P_CLK_COUNT_APERTURE    30
#define REG_ADC_P_CLK_COUNT_RESET       31


// adc counts
#define REG_ADC_CLK_COUNT_REFMUX_RESET  40
#define REG_ADC_CLK_COUNT_REFMUX_NEG    41
#define REG_ADC_CLK_COUNT_REFMUX_POS    42
#define REG_ADC_CLK_COUNT_REFMUX_RD     43
#define REG_ADC_CLK_COUNT_MUX_SIG       44


// extra stat counts.
#define REG_ADC_STAT_COUNT_REFMUX_POS_UP  50
#define REG_ADC_STAT_COUNT_REFMUX_NEG_UP  51
#define REG_ADC_STAT_COUNT_CMPR_CROSS_UP  52






//
// fpga has no concept of these - pass-through, so would be good to define in mode, where used.
// but have to be shared to data as well.
// so put here
/*
  // EXTR - but status should be recording the last.
      not what is set in mode.
*/
#define SEQ_MODE_BOOT         1
#define SEQ_MODE_NOAZ         2
#define SEQ_MODE_AZ           3
#define SEQ_MODE_RATIO        4
#define SEQ_MODE_AG           5
#define SEQ_MODE_DIFF         6
#define SEQ_MODE_SUM_DELTA          7


// #define SEQ_MODE_ELECTRO      3

