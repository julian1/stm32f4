
#pragma once

#include <stddef.h> // size_t, uint32_t

void spi_mux_ice40(uint32_t spi);   // moved from mux.h


uint32_t spi_ice40_reg_write32(uint32_t spi, uint8_t reg, uint32_t val);
uint32_t spi_ice40_reg_read32(uint32_t spi, uint8_t reg);


uint32_t spi_ice40_reg_write_n(uint32_t spi, uint8_t reg, const void *s, size_t n );



void spi_mux_ice40_simple(uint32_t spi);



