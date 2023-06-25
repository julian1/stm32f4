

#include <stdint.h> // uint32_t

// change name spi-mux?

// configure the muxing of the 'second' spi port, using cs2.

extern void mux_ice40(uint32_t spi);

extern void mux_no_device(uint32_t spi );
extern void mux_4094(uint32_t spi );

/*
extern void mux_adc03(uint32_t spi);
extern void mux_w25(uint32_t spi);
extern void mux_dac(uint32_t spi);
extern void mux_adc(uint32_t spi);

*/
