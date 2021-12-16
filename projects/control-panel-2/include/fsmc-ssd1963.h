
#ifdef __cplusplus
extern "C" {
#endif


void fsmc_gpio_setup(void);
void fsmc_setup(uint8_t divider);

void tft_reset(void );


void LCD_WriteCommand(uint16_t cmd) ;
void LCD_WriteData(uint16_t data) ;
uint16_t LCD_ReadData(void) ;
 
#ifdef __cplusplus
}
#endif


