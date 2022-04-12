

#pragma once


void spi_voltage_source_2_setup(    uint32_t spi, uint8_t *spi_4094_reg);
void spi_voltage_source_2_powerdown(uint32_t spi, uint8_t *spi_4094_reg);


void spi_voltage_source_2_set_val(uint32_t spi, uint32_t dac_channel, double val );


bool spi_voltage_source_2_in_on(uint8_t *spi_4094_reg);



