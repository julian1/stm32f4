

void spi_dac_setup( uint32_t spi);

extern uint32_t dac_write_register1(uint32_t spi, uint32_t r);

extern void dac_write_register(uint32_t spi, uint8_t r, uint16_t v);
extern uint32_t dac_read_register(uint32_t, uint8_t r);



