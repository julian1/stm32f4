
#pragma once


#include <stdio.h>  // FILE


int spi_ice40_bitstream_send( spi_ice40_t *spi , FILE *f, size_t size , volatile uint32_t *system_millis);

