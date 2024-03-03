
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



///////////////////
// reg spi mux
// note active bits.

// is this bitwise???

#define SPI_MUX_NONE      0
#define SPI_MUX_4094      1
#define SPI_MUX_DAC8811   (1<<1)


