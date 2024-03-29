
#pragma once

#include <stddef.h> // size_t


extern void spi_ice40_setup(uint32_t spi);



///////////////
uint32_t spi_ice40_reg_write32(uint32_t spi, uint8_t reg, uint32_t val);
uint32_t spi_ice40_reg_read32(uint32_t spi, uint8_t reg);


uint32_t spi_ice40_reg_write_n(uint32_t spi, uint8_t reg, const void *s, size_t n );



