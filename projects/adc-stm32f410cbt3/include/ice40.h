

extern void spi_ice40_setup(uint32_t spi);

// these are all 4 bit
extern void spi_ice40_reg_set( uint32_t spi, uint8_t r, uint8_t v);
extern void spi_ice40_reg_clear( uint32_t spi, uint8_t r, uint8_t v);
extern uint16_t spi_ice40_reg_write( uint32_t spi, uint8_t r, uint8_t v);
extern void spi_ice40_reg_toggle( uint32_t spi, uint8_t r, uint8_t v);
extern void spi_ice40_reg_write_mask( uint32_t spi, uint8_t r, uint8_t mask, uint8_t v);


uint32_t spi_reg_xfer_24(uint32_t spi, uint8_t reg, uint32_t val);
uint32_t spi_reg_read_24(uint32_t spi, uint8_t reg);





