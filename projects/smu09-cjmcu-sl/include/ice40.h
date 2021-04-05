

extern void spi_ice40_setup(uint32_t spi);

extern void spi_ice40_reg_set( uint32_t spi, uint8_t r, uint8_t v);
extern void spi_ice40_reg_clear( uint32_t spi, uint8_t r, uint8_t v);
extern void spi_ice40_reg_write( uint32_t spi, uint8_t r, uint8_t v);
// extern void spi_ice40_reg_toggle( uint32_t spi, uint8_t r, uint8_t v);

// here?
#define SPI_ICE40       SPI1






