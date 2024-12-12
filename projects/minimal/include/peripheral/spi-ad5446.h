
#pragma once

#include <stddef.h> // size_t


#include <peripheral/spi.h> 



typedef struct spi_t spi_t;

void spi_ad5446_write16( spi_t *spi, uint16_t val);



