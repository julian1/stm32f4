

#include <stdint.h> // uint32_t



// configure the muxing of the 'second' spi port, using cs2.

extern void mux_fpga(uint32_t spi);
extern void mux_adc03(uint32_t spi);
extern void mux_w25(uint32_t spi);
extern void mux_dac(uint32_t spi);
extern void mux_adc(uint32_t spi);


////////////////
// these are wrapper functions. around spi-ice40.
// should be called ice40-reg-write() etc.


// abstract to avoid including spi-ice40.h.
// TODO - either re
// probably belong in a different file.
// prefix with reg_ instead of io.

extern void mux_io(uint32_t spi);
extern void io_set( uint32_t spi, uint8_t r, uint8_t v);
extern void io_clear( uint32_t spi, uint8_t r, uint8_t v);
extern void io_write( uint32_t spi, uint8_t r, uint8_t v);
extern void io_toggle( uint32_t spi, uint8_t r, uint8_t v);

extern void io_write_mask( uint32_t spi, uint8_t r, uint8_t mask, uint8_t v);


