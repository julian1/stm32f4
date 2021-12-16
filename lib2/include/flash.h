
#ifdef __cplusplus
extern "C" {
#endif



// TODO. rename this. eg. not mpsse
void mpsse_xfer_spi(uint32_t spi, uint8_t *data, size_t n);

void flash_reset( uint32_t spi);

void flash_power_up(uint32_t spi);

uint8_t flash_read_status( uint32_t spi);

void flash_write_enable( uint32_t spi);

void flash_print_status( uint32_t spi);

void flash_read_id( uint32_t spi);


#ifdef __cplusplus
}
#endif



