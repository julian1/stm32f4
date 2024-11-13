
#pragma once

#include <stddef.h> // size_t

typedef struct spi_t spi_t;

void spi_dac8811_port_configure(uint32_t spi);

void spi_dac8811_write16( spi_t *spi, uint16_t val);


