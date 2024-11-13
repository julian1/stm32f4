
#pragma once

// this is not device. related. should not be in spi-port.
// but also not in spi.h.  because that's a basic spi peripheral interface


// wait ready is not related to device. 
void spi_wait_ready(uint32_t spi );

// but this isn't the right place. because it's 
// spi


