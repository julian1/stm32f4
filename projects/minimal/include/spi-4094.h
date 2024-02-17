

#pragma once

#include <stddef.h> // size_t

void spi_mux_4094(uint32_t spi );

uint8_t spi_4094_reg_write(uint32_t spi, uint8_t v);


uint32_t spi_4094_reg_write_n(uint32_t spi, const unsigned char *s, size_t n);

