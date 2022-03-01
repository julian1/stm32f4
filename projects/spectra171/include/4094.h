

#pragma once

// this isn't general spi..
// where to put this.
// probably in the actual dac header
// or dac2.h ? 
// hc4094.
#define REG_DAC_RST         1
#define REG_DAC_LDAC        (1<<1)
#define REG_DAC_UNI_BIP_A   (1<<2)
#define REG_DAC_UNI_BIP_B   (1<<3)
#define REG_RAILS_OE        (1<<6)
#define REG_RAILS_ON        (1<<7)

// rst high (AL) / ldac lo.
// reg = REG_DAC_RST; 

void spi_4094_setup(uint32_t spi);
uint8_t spi_4094_reg_write(uint32_t spi, uint8_t v);

