

#include <stdint.h> // uint32_t



// configure the muxing of the 'second' spi port, using cs2.

extern void mux_fpga(uint32_t spi);
extern void mux_adc03(uint32_t spi);
extern void mux_w25(uint32_t spi);
extern void mux_dac(uint32_t spi);
extern void mux_adc(uint32_t spi);



