

#pragma once

// argument order should probably be reversed.
// no. the reg is the value. 
// renam spi_4094_reg to spi_4094_value. sinice its not a register.

void voltage_source_2_setup(    uint32_t spi, uint8_t *spi_4094_reg);
void voltage_source_2_powerdown(uint32_t spi, uint8_t *spi_4094_reg);


void voltage_source_2_set_val(uint32_t spi, uint32_t dac_reg, double val );





