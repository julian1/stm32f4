

extern void spi_ice40_setup(uint32_t spi);

// these don't have spi- prefix,  but it makes dealing with ice40 registers more clear.
// should possibly move to separate file

#if 0

extern uint8_t ice40_reg_read( uint32_t spi, uint8_t r);

extern void ice40_reg_set( uint32_t spi, uint8_t r, uint8_t v);
extern void ice40_reg_clear( uint32_t spi, uint8_t r, uint8_t v);
extern void ice40_reg_write( uint32_t spi, uint8_t r, uint8_t v);
extern void ice40_reg_toggle( uint32_t spi, uint8_t r, uint8_t v);
extern void ice40_reg_write_mask( uint32_t spi, uint8_t r, uint8_t mask, uint8_t v);

//////////

uint32_t spi_ice40_reg_read(uint32_t spi, uint8_t reg);
uint32_t spi_ice40_reg_write(uint32_t spi, uint8_t reg, uint32_t val);

#endif


///////////////
uint32_t spi_ice40_reg_write32(uint32_t spi, uint8_t reg, uint32_t val);
uint32_t spi_ice40_reg_read32(uint32_t spi, uint8_t reg);





