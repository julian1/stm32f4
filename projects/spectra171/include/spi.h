
#pragma once

#include "stdint.h" // uint32_t

// void spi_setup(uint32_t spi);
void spi1_port_setup(void); // with CS.
void spi1_port_setup2(void); // with CS2

void spi1_c2_set(void);
void spi1_cs2_clear(void);

