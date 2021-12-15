

void tft_gpio_init(void);

void fsmc_setup(void);


uint16_t LCD_ReadReg(uint8_t LCD_Reg);

void LCD_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue);

///////

void LCD_SetAddr(uint8_t LCD_Reg);
uint16_t LCD_ReadData( void);

