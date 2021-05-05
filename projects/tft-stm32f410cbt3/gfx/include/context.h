
#include <stdint.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif



#if 0

Adafruit_GFX::Adafruit_GFX(int16_t w, int16_t h) : WIDTH(w), HEIGHT(h) {
  _width = WIDTH;
  _height = HEIGHT;
  rotation = 0;
  cursor_y = cursor_x = 0;
  textsize_x = textsize_y = 1;
  textcolor = textbgcolor = 0xFFFF;
  wrap = true;
  _cp437 = false;
  gfxFont = NULL;
}
#endif


typedef struct Context
{
    // could almost include opaque spi structure here, if needed

    uint8_t rotation  ;
    uint16_t width;
    uint16_t height;

#if 0
  int16_t cursor_x;     ///< x location to start print()ing text
  int16_t cursor_y;     ///< y location to start print()ing text
  uint16_t textcolor;   ///< 16-bit background color for print()
  uint16_t textbgcolor; ///< 16-bit text color for print()
  uint8_t textsize_x;   ///< Desired magnification in X-axis of text to print()
  uint8_t textsize_y;   ///< Desired magnification in Y-axis of text to print()
  uint8_t rotation;     ///< Display rotation (0 thru 3)
  bool wrap;            ///< If set, 'wrap' text at right edge of display
  bool _cp437;          ///< If set, use correct CP437 charset (default is off)
  GFXfont *gfxFont;     ///< Pointer to special font
#endif


  int16_t cursor_x;     ///< x location to start print()ing text
  int16_t cursor_y;     ///< y location to start print()ing text
  uint16_t textcolor;   ///< 16-bit background color for print()
  uint16_t textbgcolor; ///< 16-bit text color for print()
  uint8_t textsize_x;   ///< Desired magnification in X-axis of text to print()
  uint8_t textsize_y;   ///< Desired magnification in Y-axis of text to print()

  bool wrap;            ///< If set, 'wrap' text at right edge of display
  bool cp437;          ///< If set, use correct CP437 charset (default is off)

  void *gfxFont;

} Context;


// prefix with context_?

void startWrite(Context *ctx) ;

void endWrite(Context *ctx);


void delay( uint16_t x );

void initialize(Context *ctx);

void ILI9341_setRotation(Context *ctx, uint8_t m) ;

void ILI9341_SetAddressWindow(Context *ctx, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

void ILI9341_DrawRectangle(Context *ctx, uint16_t x, uint16_t y, uint16_t x_off, uint16_t y_off, uint16_t color);

// supports rgb565 byte order for agg.
void ILI9341_DrawBuffer(Context *ctx, uint16_t x, uint16_t y, uint16_t x_off, uint16_t y_off, const uint8_t *dataBytes);

#ifdef __cplusplus
}
#endif




