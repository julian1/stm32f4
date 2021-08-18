

int voltage_to_dac( float x);

void spi_dac_setup( uint32_t spi  );

// extern uint32_t spi_dac_xfer_register(uint32_t spi, uint32_t r);
extern void spi_dac_write_register(uint32_t spi, uint8_t r, uint16_t v);

extern uint32_t spi_dac_read_register(uint32_t spi, uint8_t r);


extern int dac_init(uint32_t spi, uint8_t reg);  // bad name?

#define DAC_CMD_REG       0x0 

// keep registers names specific to dac8734, not application
#define DAC_VOUT0_REGISTER 0x04 
#define DAC_VOUT1_REGISTER 0x05
#define DAC_VOUT2_REGISTER 0x06 
#define DAC_VOUT3_REGISTER 0x07


#define DAC_GPIO0 (1 << 8)
#define DAC_GPIO1 (1 << 9)


// These are CMD gain. 2x or 4x.  set by hardware. not register values.
#define DAC_GAIN_BIT0 (1 << 2)  // DB2-DB5
#define DAC_GAIN_BIT1 (1 << 3)
#define DAC_GAIN_BIT3 (1 << 4)
#define DAC_GAIN_BIT4 (1 << 5)


#define DAC_MON_REGISTER 0x01   // DB11, DB12-DB15
#define DAC_MON_AIN   (1 << 11)
#define DAC_MON_MDAC0 (1 << 12)
#define DAC_MON_MDAC1 (1 << 13)
#define DAC_MON_MDAC2 (1 << 14)
#define DAC_MON_MDAC3 (1 << 13)





