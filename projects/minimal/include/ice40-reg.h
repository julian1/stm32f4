
#pragma once

/*
  fpga specific
  this doeesn't belong in mode.h
  because then it would then get pulled in by the spi peripheral code, which needs mode.h.


*/

// better name CR_  for control register?
// CR_DIRECT ? etc.
// SR_STATUS


#define REG_SPI_MUX                     8
#define REG_4094                        9
#define REG_MODE                        12
#define REG_DIRECT                      14
#define REG_STATUS                      17


///////////////////////

//  sample acquisition.
#define REG_SA_ARM_TRIGGER              20

#define REG_SA_P_CLK_COUNT_PRECHARGE    21
#define REG_SA_P_AZMUX_LO_VAL           22
#define REG_SA_P_AZMUX_HI_VAL           23
#define REG_SA_P_SW_PC_CTL_HI_VAL       24


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






///////////////////
// reg spi mux
// note active bits.

// spi mux is one-hot/

#define SPI_MUX_NONE      0
#define SPI_MUX_4094      1
#define SPI_MUX_DAC8811   (1<<1)



#define MODE_DIRECT       0     // support direct writing via direct register
#define MODE_LO           1     // all bits held lo. but blink led. default.
#define MODE_HI           2     // all bits held hi
#define MODE_PATTERN      3     // put modulation pattern on all bits
#define MODE_PC           4     // TODO. - rename PC_ONLY simple precharge switching modulation.

// change name MODE_AZ to MODE_SA_AZ.


/*
#define MODE_LO           0     // all bits held lo. but blink led. default.
#define MODE_HI           1     // all bits held hi
#define MODE_PATTERN      2     // put modulation pattern on all bits
#define MODE_DIRECT       3     // support direct writing via direct register
#define MODE_PC           4     // simple precharge switching modulation.
#define MODE_SA_AZ        5     // az.
#define MODE_SA_NO_AZ     6     // no az. for electrometer style input. etc

*/


