
#pragma once

/*
  don't move to mode.h
  because it drags high-level unrelated app state / 4094 stuff for simple spi peripherals.


*/

// better name CR_  for control register?
// CR_DIRECT ? etc.
// SR_STATUS


#define REG_SPI_MUX       8
#define REG_4094          9     // add suffix. or reg_4094_cr configuration-register. or OE or something.

#define REG_MODE          12
#define REG_DIRECT        14
#define REG_STATUS        17



//  sample acquisition.
#define REG_SA_ARM_TRIGGER              19
#define REG_SA_P_CLK_COUNT_PRECHARGE    21

// adc parameters
#define REG_ADC_P_CLK_COUNT_APERTURE    20   // clk sample time. change name aperture.  // TODO reassign.
#define REG_ADC_P_CLK_COUNT_RESET       25





///////////////////
// reg spi mux
// note active bits.

// is this bitwise???

#define SPI_MUX_NONE      0
#define SPI_MUX_4094      1
#define SPI_MUX_DAC8811   (1<<1)


