

// TODO. maybe change 'fpga' to 'ice40'

extern void spi_fpga_reg_setup(uint32_t spi);
extern void spi_fpga_reg_set( uint32_t spi, uint8_t r, uint8_t v);
extern void spi_fpga_reg_clear( uint32_t spi, uint8_t r, uint8_t v);
extern void spi_fpga_reg_write( uint32_t spi, uint8_t r, uint8_t v);

// THIS SHOULD PROBABLY BE SET IN THE HEADER
#define SPI_ICE40       SPI1






