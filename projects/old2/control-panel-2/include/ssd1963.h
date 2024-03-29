
#ifndef SSD1963_H
#define SSD1963_H

#ifdef __cplusplus
extern "C" {
#endif



void LCD_Init(void);



void LCD_fillRect(uint16_t x1,  uint16_t y1,uint16_t x2,  uint16_t y2, uint16_t c );


void setScrollStart(uint16_t y);


// TODO prefix with LCD
void setXY(uint16_t x1,  uint16_t y1,uint16_t x2,  uint16_t y2 );

uint16_t packRGB565( uint16_t r, uint16_t g, uint16_t b);

//
// cannot interleave to switch on the fly. since governs 1963 memory to screen. not blt operations. 
void setOriginTopLeft(void);    // conventional 
void setOriginBottomLeft(void); // cartesion/ fonts/ postscript 



uint16_t getTearEffectStatus(void );

void LCD_SetTearOn(void );

void  LCD_TestFill(void);

void  LCD_Read_DDB(void);
/* 
  antigrain font loading. appears to use flip_y on load
  https://coconut2015.github.io/agg-tutorial/tutorial__font__1_8cpp_source.htm

  i think we can use top-left. ok. 
*/


#ifdef __cplusplus
}
#endif

#endif


