

#include <stdint.h> // uint32_t



// configure the muxing of the 'second' spi port, using cs2.

extern void mux_fpga(uint32_t spi);
extern void mux_adc03(uint32_t spi);
extern void mux_w25(uint32_t spi);
extern void mux_dac(uint32_t spi);
extern void mux_adc(uint32_t spi);



extern void reg_set( uint32_t spi, uint8_t r, uint8_t v);
extern void reg_clear( uint32_t spi, uint8_t r, uint8_t v);
extern void reg_write( uint32_t spi, uint8_t r, uint8_t v);
extern void reg_toggle( uint32_t spi, uint8_t r, uint8_t v);

extern void reg_write_mask( uint32_t spi, uint8_t r, uint8_t mask, uint8_t v);


