




void fsmc_gpio_setup(void);
void fsmc_setup(uint8_t divider);


void tft_reset(void );

uint16_t LCD_ReadReg(uint8_t LCD_Reg);

void LCD_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue);

///////

void LCD_SetAddr(uint8_t LCD_Reg);
uint16_t LCD_ReadData( void);

