

#pragma once

#ifdef __cplusplus
extern "C" {
#endif



#include <stdint.h>
#include <stdbool.h>


#define TFT_MAGIC  82342351


typedef struct tft_t tft_t;

typedef struct tft_t
{
  uint32_t  magic;

  uint32_t  fmc_addr;     // FMC_MY_BASE |  FMC_A19
  uint32_t  fmc_cd;       // command/data bit. FMC_A16.


  void (*tft_gpio_setup)( tft_t *);
  bool (*tft_getTear)( tft_t *);                  // renae
  void (*tft_reset)( tft_t *, bool val );

/*
  // remmember the bits p
  // in pix
  uint32_t  width;          // pix
  uint32_t  height_bytes;   // hieght in bytes is / 8.

  //
  bool page;
*/



} tft_t;





inline void tft_write_cmd( tft_t *tft, uint16_t cmd)
{

  *((volatile uint16_t *)  (tft->fmc_addr )) = cmd;
}

inline void tft_write_data( tft_t *tft, uint16_t data)
{
  *((volatile uint16_t *)  (tft->fmc_addr | tft->fmc_cd)) = data;
}

inline uint16_t tft_read_data( tft_t *tft)
{
  return *((volatile uint16_t *)  (tft->fmc_addr | tft->fmc_cd));
}



inline bool tft_get_tear( tft_t *tft)
{
  assert( tft);
  return tft->tft_getTear( tft);
}








/////////////


void LCD_Init( tft_t *tft, volatile uint32_t *system_millis);


void setScrollStart( tft_t *tft, uint16_t y);


// TODO prefix with LCD
void setXY( tft_t *tft,  uint16_t x1,  uint16_t y1,uint16_t x2,  uint16_t y2 );


//
// cannot interleave to switch on the fly. since governs 1963 memory to screen. not blt operations.
void setOriginTopLeft(  tft_t *tft);    // conventional
void setOriginBottomLeft( tft_t *tft ); // cartesion/ fonts/ postscript



uint16_t getTearEffectStatus( tft_t *tft);

void LCD_SetTearOn( tft_t *tft);


uint16_t packRGB565(  uint16_t r, uint16_t g, uint16_t b);

void LCD_fillRect( tft_t *tft,  uint16_t x1,  uint16_t y1,uint16_t x2,  uint16_t y2, uint16_t c );





void  LCD_Read_DDB( tft_t *tft);

void  LCD_TestFill( tft_t *tft);



#ifdef __cplusplus
}
#endif



/*
  antigrain font loading. appears to use flip_y on load
  https://coconut2015.github.io/agg-tutorial/tutorial__font__1_8cpp_source.htm

  i think we can use top-left. ok.
*/


