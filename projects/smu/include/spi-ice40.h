

extern void spi_ice40_setup(uint32_t spi);

// these don't have spi- prefix,  but it makes dealing with ice40 registers more clear.
// should possibly move to separate file

extern void ice40_reg_set( uint32_t spi, uint8_t r, uint8_t v);
extern void ice40_reg_clear( uint32_t spi, uint8_t r, uint8_t v);
extern void ice40_reg_write( uint32_t spi, uint8_t r, uint8_t v);
extern void ice40_reg_toggle( uint32_t spi, uint8_t r, uint8_t v);
extern void ice40_reg_write_mask( uint32_t spi, uint8_t r, uint8_t mask, uint8_t v);





