

void dac_setup_spi( void );
void dac_setup_bitbash( void );


void dac_reset(void);
void dac_write_register(uint8_t r, uint16_t v);

uint32_t dac_read_register(uint8_t r);


// shouldn't be here... high-level
#define DAC_VSET_REGISTER 0x04 
#define DAC_ISET_REGISTER 0x05

// ahhh --- call this
// void dac_test(void );

