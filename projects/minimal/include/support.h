
#pragma once

#include <stdbool.h>
#include <stdint.h>

// this is not device. related. should not be in spi-port.
// but also not in spi.h.  because that's a basic spi peripheral interface


// spi wait ready is not related to any device.
void spi_wait_ready(uint32_t spi );

// gpio helper
void gpio_write_val(uint32_t gpioport, uint16_t gpios, bool val);

void gpio_write_with_mask( uint32_t gpioport, uint32_t shift, uint32_t mask, uint32_t val);

// avoid pulling libopencm3 headers as dependency, in high level code
void mcu_reset(void);




