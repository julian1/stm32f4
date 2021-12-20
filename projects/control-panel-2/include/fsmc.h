
#ifndef FSMC_H
#define FSMC_H

#ifdef __cplusplus
extern "C" {
#endif


void fsmc_gpio_setup(void);
void fsmc_setup(uint8_t divider);

void tft_reset(void ); // TODO maybe move to ssd1963

/*
  It might make sense to expose the data structure here. 
  and then inline these...
*/

void LCD_WriteCommand(uint16_t cmd) ;
void LCD_WriteData(uint16_t data) ;
uint16_t LCD_ReadData(void) ;
 
#ifdef __cplusplus
}
#endif

#endif
