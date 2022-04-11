
#pragma once

#include "stdint.h" // uint32_t


void spi_port_cs1_setup(uint32_t spi);
void spi_port_cs2_setup(uint32_t spi); 


void spi_cs2_set(uint32_t spi );
void spi_cs2_clear(uint32_t spi );

