

#include <stdint.h> // uint16_t etc

#include "Adafruit-GFX-Library/gfxfont.h"
#include "Adafruit-GFX-Library/glcdfont.c"


#include "context.h"
#include "gfx.h"


#define UNUSED(x) (void)(x)

#if 0
void Adafruit_GFX::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                            uint16_t color) {
  startWrite();
  for (int16_t i = x; i < x + w; i++) {
    writeFastVLine(i, y, h, color);
  }
  endWrite();
}
#endif



void fillRect(Context *ctx, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{

  ILI9341_DrawRectangle(ctx, x, y, w, h, color);
#if 0
  startWrite();
  for (int16_t i = x; i < x + w; i++) {
    writeFastVLine(i, y, h, color);
  }
  endWrite();
#endif
}

void fillScreen(Context *ctx, uint16_t color)
{
  fillRect(ctx, 0, 0, ctx->width, ctx->height, color);
}

/*
    appears to use a buffer????
void writePixel(Context *ctx, int16_t x, int16_t y, uint16_t color)
{
  drawPixel(x, y, color);
}
*/

void writeFillRect(Context *ctx, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    // same...
  fillRect(ctx, x, y, w, h, color);
}


void writeFastHLine(Context *ctx, int16_t x, int16_t y, int16_t w, uint16_t color)
{
  // Overwrite in subclasses if startWrite is defined!
  // Example: writeLine(x, y, x+w-1, y, color);
  // or writeFillRect(x, y, w, 1, color);
  //drawFastHLine(x, y, w, color);

  writeFillRect(ctx, x, y, w, 1, color);

}

void writeFastVLine(Context *ctx, int16_t x, int16_t y, int16_t h, uint16_t color)
{
  // Overwrite in subclasses if startWrite is defined!
  // Can be just writeLine(x, y, x, y+h-1, color);
  // or writeFillRect(x, y, 1, h, color);
  // drawFastVLine(x, y, h, color);

  writeFillRect(ctx, x, y, 1, h, color);
}



#if 0
void Adafruit_SPITFT::drawPixel(int16_t x, int16_t y, uint16_t color) {
  // Clip first...
  if ((x >= 0) && (x < _width) && (y >= 0) && (y < _height)) {
    // THEN set up transaction (if needed) and draw...
    startWrite();
    setAddrWindow(x, y, 1, 1);
    SPI_WRITE16(color);
    endWrite();
  }
}
#endif

static void drawPixel(Context *ctx, int16_t x, int16_t y, uint16_t color) {
  // Clip first...
  if ((x >= 0) && (x < ctx->width) && (y >= 0) && (y < ctx->height)) {
    // THEN set up transaction (if needed) and draw...


    //  can improve later
    ILI9341_DrawRectangle(ctx, x, y, 1 , 1, color);

  }
}


void writePixel(Context *ctx, int16_t x, int16_t y, uint16_t color) {
  drawPixel(ctx, x, y, color);
}

#include <stdlib.h> // abs


#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef _swap_int16_t
#define _swap_int16_t(a, b)                                                    \
  {                                                                            \
    int16_t t = a;                                                             \
    a = b;                                                                     \
    b = t;                                                                     \
  }
#endif



void writeLine(Context *ctx, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
#if defined(ESP8266)
  yield();
#endif
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    _swap_int16_t(x0, y0);
    _swap_int16_t(x1, y1);
  }

  if (x0 > x1) {
    _swap_int16_t(x0, x1);
    _swap_int16_t(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0 <= x1; x0++) {
    if (steep) {
      writePixel(ctx, y0, x0, color);
    } else {
      writePixel(ctx, x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}





void drawCircle(Context *ctx, int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
#if defined(ESP8266)
  yield();
#endif
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  startWrite(ctx);
  writePixel(ctx, x0, y0 + r, color);
  writePixel(ctx, x0, y0 - r, color);
  writePixel(ctx, x0 + r, y0, color);
  writePixel(ctx, x0 - r, y0, color);

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    writePixel(ctx, x0 + x, y0 + y, color);
    writePixel(ctx, x0 - x, y0 + y, color);
    writePixel(ctx, x0 + x, y0 - y, color);
    writePixel(ctx, x0 - x, y0 - y, color);
    writePixel(ctx, x0 + y, y0 + x, color);
    writePixel(ctx, x0 - y, y0 + x, color);
    writePixel(ctx, x0 + y, y0 - x, color);
    writePixel(ctx, x0 - y, y0 - x, color);
  }
  endWrite(ctx);
}




static inline uint8_t pgm_read_byte(const uint8_t *addr) {
  return *addr;
}



// Draw a character
/**************************************************************************/
/*!
   @brief   Draw a single character
    @param    x   Bottom left corner x coordinate
    @param    y   Bottom left corner y coordinate
    @param    c   The 8-bit font-indexed character (likely ascii)
    @param    color 16-bit 5-6-5 Color to draw chraracter with
    @param    bg 16-bit 5-6-5 Color to fill background with (if same as color,
   no background)
    @param    size_x  Font magnification level in X-axis, 1 is 'original' size
    @param    size_y  Font magnification level in Y-axis, 1 is 'original' size
*/
/**************************************************************************/
void drawChar(
    Context *ctx,
    int16_t x, int16_t y, unsigned char c,
    uint16_t color, uint16_t bg,
    uint8_t size_x, uint8_t size_y
) {

  if (!ctx->gfxFont) { // 'Classic' built-in font

    if ((x >= ctx->width) ||              // Clip right
        (y >= ctx->height) ||             // Clip bottom
        ((x + 6 * size_x - 1) < 0) || // Clip left
        ((y + 8 * size_y - 1) < 0))   // Clip top
      return;

    if (!ctx->cp437 && (c >= 176))
      c++; // Handle 'classic' charset behavior

    startWrite(ctx);
    for (int8_t i = 0; i < 5; i++) { // Char bitmap = 5 columns
      uint8_t line = pgm_read_byte(&font[c * 5 + i]);
      for (int8_t j = 0; j < 8; j++, line >>= 1) {
        if (line & 1) {
          if (size_x == 1 && size_y == 1)
            writePixel(ctx, x + i, y + j, color);
          else
            writeFillRect(ctx, x + i * size_x, y + j * size_y, size_x, size_y, color);
        } else if (bg != color) {
          if (size_x == 1 && size_y == 1)
            writePixel(ctx, x + i, y + j, bg);
          else
            writeFillRect(ctx, x + i * size_x, y + j * size_y, size_x, size_y, bg);
        }
      }
    }
    if (bg != color) { // If opaque, draw vertical line for last column
      if (size_x == 1 && size_y == 1)
        writeFastVLine(ctx, x + 5, y, 8, bg);
      else
        writeFillRect(ctx, x + 5 * size_x, y, size_x, 8 * size_y, bg);
    }
    endWrite(ctx);

  } else { // Custom font

#if 0
    // Character is assumed previously filtered by write() to eliminate
    // newlines, returns, non-printable characters, etc.  Calling
    // drawChar() directly with 'bad' characters of font may cause mayhem!

    c -= (uint8_t)pgm_read_byte(&gfxFont->first);
    GFXglyph *glyph = pgm_read_glyph_ptr(gfxFont, c);
    uint8_t *bitmap = pgm_read_bitmap_ptr(gfxFont);

    uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
    uint8_t w = pgm_read_byte(&glyph->width), h = pgm_read_byte(&glyph->height);
    int8_t xo = pgm_read_byte(&glyph->xOffset),
           yo = pgm_read_byte(&glyph->yOffset);
    uint8_t xx, yy, bits = 0, bit = 0;
    int16_t xo16 = 0, yo16 = 0;

    if (size_x > 1 || size_y > 1) {
      xo16 = xo;
      yo16 = yo;
    }

    // Todo: Add character clipping here

    // NOTE: THERE IS NO 'BACKGROUND' COLOR OPTION ON CUSTOM FONTS.
    // THIS IS ON PURPOSE AND BY DESIGN.  The background color feature
    // has typically been used with the 'classic' font to overwrite old
    // screen contents with new data.  This ONLY works because the
    // characters are a uniform size; it's not a sensible thing to do with
    // proportionally-spaced fonts with glyphs of varying sizes (and that
    // may overlap).  To replace previously-drawn text when using a custom
    // font, use the getTextBounds() function to determine the smallest
    // rectangle encompassing a string, erase the area with fillRect(),
    // then draw new text.  This WILL infortunately 'blink' the text, but
    // is unavoidable.  Drawing 'background' pixels will NOT fix this,
    // only creates a new set of problems.  Have an idea to work around
    // this (a canvas object type for MCUs that can afford the RAM and
    // displays supporting setAddrWindow() and pushColors()), but haven't
    // implemented this yet.

    startWrite();
    for (yy = 0; yy < h; yy++) {
      for (xx = 0; xx < w; xx++) {
        if (!(bit++ & 7)) {
          bits = pgm_read_byte(&bitmap[bo++]);
        }
        if (bits & 0x80) {
          if (size_x == 1 && size_y == 1) {
            writePixel(x + xo + xx, y + yo + yy, color);
          } else {
            writeFillRect(x + (xo16 + xx) * size_x, y + (yo16 + yy) * size_y,
                          size_x, size_y, color);
          }
        }
        bits <<= 1;
      }
    }
    endWrite();

#endif

  } // End classic vs custom font
}




size_t write(Context *ctx, uint8_t c)
{
  if (!ctx->gfxFont) { // 'Classic' built-in font

    if (c == '\n') {              // Newline?
      ctx->cursor_x = 0;               // Reset x to zero,
      ctx->cursor_y += ctx->textsize_y * 8; // advance y one line
    } else if (c != '\r') {       // Ignore carriage returns
      if (ctx->wrap && ((ctx->cursor_x + ctx->textsize_x * 6) > ctx->width)) { // Off right?
        ctx->cursor_x = 0;                                       // Reset x to zero,
        ctx->cursor_y += ctx->textsize_y * 8; // advance y one line
      }
      drawChar(ctx, ctx->cursor_x, ctx->cursor_y, c, ctx->textcolor, ctx->textbgcolor, ctx->textsize_x, ctx->textsize_y);
      ctx->cursor_x += ctx->textsize_x * 6; // Advance x one char
    }

  } else { // Custom font

#if 0
    if (c == '\n') {
      cursor_x = 0;
      cursor_y +=
          (int16_t)textsize_y * (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
    } else if (c != '\r') {
      uint8_t first = pgm_read_byte(&gfxFont->first);
      if ((c >= first) && (c <= (uint8_t)pgm_read_byte(&gfxFont->last))) {
        GFXglyph *glyph = pgm_read_glyph_ptr(gfxFont, c - first);
        uint8_t w = pgm_read_byte(&glyph->width),
                h = pgm_read_byte(&glyph->height);
        if ((w > 0) && (h > 0)) { // Is there an associated bitmap?
          int16_t xo = (int8_t)pgm_read_byte(&glyph->xOffset); // sic
          if (wrap && ((cursor_x + textsize_x * (xo + w)) > _width)) {
            cursor_x = 0;
            cursor_y += (int16_t)textsize_y *
                        (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
          }
          drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize_x,
                   textsize_y);
        }
        cursor_x +=
            (uint8_t)pgm_read_byte(&glyph->xAdvance) * (int16_t)textsize_x;
      }
    }
#endif
  }
  return 1;
}




void setCursor(Context *ctx, int16_t x, int16_t y) 
{
    ctx->cursor_x = x;
    ctx->cursor_y = y;
  }

void setTextColor(Context *ctx, uint16_t c) 
{ 
  ctx->textcolor = ctx->textbgcolor = c; 
}


void setTextSize(Context *ctx, uint8_t s_x, uint8_t s_y) 
{
  ctx->textsize_x = (s_x > 0) ? s_x : 1;
  ctx->textsize_y = (s_y > 0) ? s_y : 1;
}








