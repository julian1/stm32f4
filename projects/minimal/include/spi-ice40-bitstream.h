
#pragma once


// ice40 pins, not assocated with spi1.
// void ice40_port_extra_setup(void);


// don't need to pass app. just pass spi, and milliseconds for timer.

typedef struct app_t app_t;

int spi_ice40_bitstream_test(app_t *app);

