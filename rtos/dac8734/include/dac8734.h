
/*
void dac_write_register_spi(uint32_t r);
void dac_write_register_bitbash(uint32_t v);
void dac_write_register1(uint32_t r);
*/


void dac_setup_spi( void );
void dac_setup_bitbash( void );


void dac_write_register(uint8_t r, uint16_t v);

// ahhh --- call this
void dac_test(void );

