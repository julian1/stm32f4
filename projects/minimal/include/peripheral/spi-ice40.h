
#pragma once

// peripheral interface/abstraction
// not device/instance

#include <stddef.h> // size_t


#include <peripheral/spi.h>


// we need a better name  for this virtual device.

uint32_t spi_ice40_reg_write32( spi_t *, uint8_t reg, uint32_t val);
uint32_t spi_ice40_reg_read32( spi_t *, uint8_t reg);
uint32_t spi_ice40_reg_write_n( spi_t *, uint8_t reg, const void *s, size_t n );



