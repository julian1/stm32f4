

#pragma once

#include <stddef.h> // size_t


void spi_4094_setup(uint32_t spi);
uint8_t spi_4094_reg_write(uint32_t spi, uint8_t v);


uint32_t spi_4094_reg_write_n(uint32_t spi, unsigned char *s, size_t n);

