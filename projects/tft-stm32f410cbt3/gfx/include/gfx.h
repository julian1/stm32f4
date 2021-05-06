
/*
    see overrides in

    Adafruit-GFX-Library/Adafruit_SPITFT.cpp

    
    void Adafruit_SPITFT::startWrite(void) {
    void Adafruit_SPITFT::endWrite(void) {
    void Adafruit_SPITFT::writePixel(int16_t x, int16_t y, uint16_t color) {

*/

#include <stdint.h> // uint16_t
#include <stddef.h> // size_t


#ifdef __cplusplus
extern "C" {
#endif



// prefix with gfx_

void fillRect(Context *ctx, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) ;

void fillScreen(Context *ctx, uint16_t color) ;


void writeFillRect(Context *ctx, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) ;

// possibly should be moved to context.
void writeFastHLine(Context *ctx, int16_t x, int16_t y, int16_t w, uint16_t color); 
void writeFastVLine(Context *ctx, int16_t x, int16_t y, int16_t h, uint16_t color);

// not sure if should be exposed.
// void drawPixel(Context *ctx, int16_t x, int16_t y, uint16_t color) ;


void writePixel(Context *ctx, int16_t x, int16_t y, uint16_t color);

void writeLine(Context *ctx, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);


void drawCircle(Context *ctx, int16_t x0, int16_t y0, int16_t r, uint16_t color);

/////////

void drawChar(
    Context *ctx,
    int16_t x, int16_t y, unsigned char c,
    uint16_t color, uint16_t bg, 
    uint8_t size_x, uint8_t size_y); 


/*
    our printf function is goinig to have to take the function pointer, and context
    as arguments... which is a bit messy.
    eg. gfx_printf(&ctx, gfx_write, "%s %d", "hi", 123);
*/
size_t write(Context *ctx, uint8_t c);


void /*Adafruit_GFX::*/ charBounds(Context *ctx, unsigned char c, int16_t *x, int16_t *y,
                              int16_t *minx, int16_t *miny, int16_t *maxx,
                              int16_t *maxy) ;

void /*Adafruit_GFX::*/getTextBounds(Context *ctx, const char *str, int16_t x, int16_t y,
                                 int16_t *x1, int16_t *y1, uint16_t *w,
                                 uint16_t *h) ;



void setCursor(Context *ctx, int16_t x, int16_t y) ;
void setTextColor(Context *ctx, uint16_t c) ;
//void setTextColor2(uint16_t c, uint16_t bg) ;

void setTextBGColor(Context *ctx, uint16_t c );

void setTextSize(Context *ctx, uint8_t s_x, uint8_t s_y) ;


#ifdef __cplusplus
}
#endif


