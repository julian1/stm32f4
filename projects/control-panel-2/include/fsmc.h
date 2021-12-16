
#ifndef FSMC_H
#define FSMC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>


void fsmc_gpio_setup(void);
void fsmc_setup(uint8_t divider);

void tft_reset(void ); // TODO maybe move to ssd1963

bool getTear(void);

////////////////////////////////

// JA required to be volatile
#define __IO volatile

 typedef struct
 {
  __IO uint16_t LCD_REG;
  __IO uint16_t LCD_RAM;
 } LCD_TypeDef;

// #define LCD_BASE    ((uint32_t)(0x60000000 | 0x00020000 -2 ) )
// JA
#define LCD_BASE    ((uint32_t)(0x60000000 | (0x00020000 -2) ) )

/* JA
  >  (0x60000000 | (0x00020000 -2)).toString(16)
  "6001fffe"
  > (0x60000000 | (0x00020000 )).toString(16)
  "60020000"
*/

/*
When you access A16 becomes zero in the LCD-> LCD_REG. (Address 0x6001FFFE)
When you access A16 is 1 in LCD-> LCD_RAM. (Address 0x60020000)
*/

#define LCD         ((LCD_TypeDef *) LCD_BASE)


inline void LCD_WriteCommand(uint16_t cmd) {
  /* Write cmd */
  // LCD_REG = cmd;
  LCD->LCD_REG = cmd;
}

inline void LCD_WriteData(uint16_t data) {
  /* Write 16-bit data */
  // LCD_RAM = data;
  LCD->LCD_RAM = data;
}

inline uint16_t LCD_ReadData(void) {
  /* Read 16-bit data */
  // return LCD_RAM;
  return (LCD->LCD_RAM);
}





#ifdef __cplusplus
}
#endif

#endif
