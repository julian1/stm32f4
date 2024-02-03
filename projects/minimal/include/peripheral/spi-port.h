
#pragma once

#include <stdint.h>  // uint32_t


void spi1_port_cs1_cs2_setup(void);


///////

// abstracted over spi
// hardware ness with set_nss_high etc.
// doesn't work well with more than one cs.

void spi_port_cs1_enable(uint32_t );    // active lo
void spi_port_cs1_disable(uint32_t);


void spi_port_cs2_enable(uint32_t) ;
void spi_port_cs2_disable(uint32_t );


void spi1_port_interupt_setup( void (*pfunc_)(void *), void *ctx);




