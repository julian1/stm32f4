
#pragma once


// application specific.

// this is mixing up 4094, and fpga stuff.
// but for most muxes, the ctrl approach doesn't matter.

// #include <stdint.h>




// control led - but only in 0/default mode
#define REG_LED         7



// need to rename named _4094_GLB_OE or similar to respect prefix convention
// maybe rename this register... GENERAL REG_GENERAL.   else it's too confusing.
// needs to be renamed REG_4094_OE . eg. keep specific.
#define REG_4094          9
#define GLB_4094_OE       (1<<0)    // first bit


#define REG_MODE          12

#define MODE_LO           0     // all bits held lo. but blink led. default.
#define MODE_HI           1     // all bits held hi
#define MODE_PATTERN      2     // put modulation pattern on all bits
#define MODE_DIRECT       3     // support direct writing via direct register
#define MODE_PC           4     // simple precharge switching modulation.
#define MODE_AZ           5     // az.
#define MODE_NO_AZ        6     // no az. and elecm. etc



#define REG_DIRECT        14
// #define REG_DIRECT2       15


#define REG_STATUS        17
#define REG_RESET         18

// acquisition trigger
#define REG_SA_ARM_TRIGGER  19
#define REG_SA_P_CLK_COUNT_PRECHARGE 21

// adc parameters
#define REG_ADC_P_APERTURE          20
#define REG_ADC_P_CLK_COUNT_RESET   25


// adc read counts
#define REG_ADC_CLK_COUNT_MUX_RESET 34    // TODO fix/ re-assign enum .
#define REG_ADC_CLK_COUNT_MUX_NEG   30
#define REG_ADC_CLK_COUNT_MUX_POS   31
#define REG_ADC_CLK_COUNT_MUX_RD    32
#define REG_ADC_CLK_COUNT_MUX_SIG   33





/*

  this is 4094. and also direct writing of regs.

  define behavior in a mux specific way. is useful.
  make it easy to reassign pins.
  eg. if have to flip ic, and then re-route traces.
*/

// 1of8 muxes.
#define SOFF        0
#define S1          ((1<<3)|(1-1))
#define S2          ((1<<3)|(2-1))
#define S3          ((1<<3)|(3-1))
#define S4          ((1<<3)|(4-1))
#define S5          ((1<<3)|(5-1))
#define S6          ((1<<3)|(6-1))
#define S7          ((1<<3)|(7-1))


// muxes
#define HIMUX_HIMUX2          S2
#define HIMUX_DCV             S7

#define HIMUX2_DCV_SOURCE     S1
#define HIMUX2_STAR_LO            S4
#define HIMUX2_REF_HI         S5
#define HIMUX2_REF_LO         S6

#define AZMUX_PCOUT           S1
#define AZMUX_BOOT            S2
#define AZMUX_STAR_LO              S6
#define AZMUX_REF_LO          S7



// dcv- source
#define U1003_POS             S1
#define U1003_NEG             S2
#define U1003_GND             S3

#define U1006_REF10V          S1
#define U1006_REF1V           S2
#define U1006_REV0V1          S3
#define U1006_DCV_LO          S4
// ...
#define U1006_TEMP            S8




// could also be a macro #define S(1) == ...


// S for switch maybe SS ? or W
// dual 1of 4 muxes.
#define WOFF        0
#define W1          ((1<<2)|(1-1))
#define W2          ((1<<2)|(2-1))
#define W3          ((1<<2)|(3-1))
#define W4          ((1<<2)|(4-1))


#define U506_GAIN_1     W1
#define U506_GAIN_10    W2
#define U506_GAIN_100   W3

/*
  TODO prefix. LR for latching relay
  Use LLR_TOP LLR_BOT LLR_OFF
  or hyphenated LR_TOP  LR_BOT etc.


*/
#define LR_OFF      0
#define LR_BOT      0b01      // bottom contacts closed.
#define LR_TOP      0b10      // top contacts closed.


#define SW_PC_SIGNAL    1
#define SW_PC_BOOT      0







