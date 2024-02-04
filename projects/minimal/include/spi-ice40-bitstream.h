
#pragma once


// don't need to pass app. just pass spi, and milliseconds for timer.
// should pass the flash sector
// todo rename

typedef struct app_t app_t;

int spi_ice40_bitstream_send(app_t *app);

