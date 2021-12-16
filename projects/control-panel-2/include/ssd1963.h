


#ifdef __cplusplus
extern "C" {
#endif



void LCD_Init(void);

void setXY(uint16_t x1,  uint16_t y1,uint16_t x2,  uint16_t y2 );


void LCD_fillRect(uint16_t x1,  uint16_t y1,uint16_t x2,  uint16_t y2, uint16_t c );

uint16_t packRGB565( uint16_t r, uint16_t g, uint16_t b);



#ifdef __cplusplus
}
#endif



