
#ifndef CURSES_H
#define CURSES_H

#include "agg.h"
#include "fonts.h"

/*
  - could use a sparse array.

*/


// 33 * 17 == 561
#define MAXCELLS (33 * 17)

#define MAXCOLORPAIRS 8

// we could template the maxcells argument. when want to instantiate for different sizes.

/*
  - this could be sparse and work just as well.
  and would work as well for proportional font draw sequences.
  - remember - fg/bg glyph sizes can probably work to do focused rederaw.
  ////////////////////////////////////////////

  UhGhhh..  think cannot include in main.c because agg::rgba() is templated?
    unless compile with cpp...
*/

struct A
{

  explicit A(
    uint16_t stride_,
    uint16_t ny_,
    uint16_t pdx_,
    uint16_t pdy_ )
    :
    stride(stride_),
    ny(ny_),
    pdx(pdx_),
    pdy(pdy_)
  {  }


  uint16_t stride; // nx
  uint16_t ny;

  // pixel dx,dy.
  uint16_t pdx;
  uint16_t pdy;

  // no concept of font size here, only font.

  /*
    EXTR.
      structure of arrays has lower/better memory needs - than array of structs, due to field padding/alignment.
  */
  /*
    EXTR.
    having fg/bg colors are very powerful concept.  means can do,
        - inverted fg/bg for focus/ emphasis. blink.
        - alpha blending/sub-pixel accuracy - without having to read the LCD hardware screen memory / faster. less complicated.
        - delta change drawing - by drawing spans in bg color to clear them - to avoid full screen clear() /and redraw everything.
  */
  agg::rgba color_fg[ MAXCOLORPAIRS ];  // agg::rgba == 32 bytes. 8 * 32 = 256 bytes.
  agg::rgba color_bg[ MAXCOLORPAIRS ];


  /////////////////////////////////////
  // character - dominant. only check other flags.
  // should be refreshed between draws.
  uint16_t character[ MAXCELLS ];

  // font to use. 0. for special glyph drawing actions
  const FontSpans *font[ MAXCELLS ];

  uint8_t  color_pair_idx[ MAXCELLS ];

  // effect. effects
  // invert fg/bg == 0x01   blink == 0x10
  uint16_t effect[ MAXCELLS ];

  // uint16_t callback_id[ 50 * 20];  // id might be easier than function ptr.
  // void (*callback[50 * 20 ] )(void )     not sure if desirable. or if should be by word.

  //////////////////////////////
  // cursor == current attributes to draw with - should use separate structure.
  // draw cursor
  uint16_t cursor_x;
  uint16_t cursor_y;

  const FontSpans *cursor_font;

  uint8_t cursor_color_pair_idx;

  uint16_t cursor_effect;

  // rather than copy all these individually. might have a cell structure


};




// treat non-virtual functions as functions.

void init( A & a);

void to( A &a, int x, int y);

void right( A &a, int dx);

void down( A &a, int dy);

void color_pair_idx( A &a, uint8_t color_pair_idx );

void font( A &a, const FontSpans *font);

void effect( A &a, uint16_t v );

void textch( A &a, uint16_t ch);

void text( A &a, const char *s, int dir);

void render( A &a, rb_t &rb, bool blink );

void draw_test1(A &a );
void draw_test3(A &a );


#endif // CURSES_H
