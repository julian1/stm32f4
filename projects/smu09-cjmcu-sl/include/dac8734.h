

int voltage_to_dac( float x);

void spi_dac_setup( uint32_t spi);

// extern uint32_t spi_dac_xfer_register(uint32_t spi, uint32_t r);
extern void spi_dac_write_register(uint32_t spi, uint8_t r, uint16_t v);

extern uint32_t spi_dac_read_register(uint32_t spi, uint8_t r);


#define DAC_VSET_REGISTER 0x04 
#define DAC_ISET_REGISTER 0x05

#define DAC_GPIO0 (1 << 8)
#define DAC_GPIO1 (1 << 9)


