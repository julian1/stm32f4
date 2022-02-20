
#pragma once

#include "agg.h"
#include "fonts.h"
#include "assert.h"

/*
  - use a sparse array.

*/


// 33 * 17 == 561
#define MCursesCellsMax (33 * 17)
// #define MCursesCellsMax (10 * 10)

#define MCursesColorsMax 8




/*
  TODO
    - OK. we need another controller. 
      that selects the menu list to display. eg. page_controller.
      then we can move between less common readings (like stddev) and  settings 

    - then we need another curses for larger
*/



#if 0
void pfalloc( void * ) ; 



template< int SZ>
struct CursesMem
{
  bool      changed[ SZ ];
  uint16_t  character[ SZ ];
  const     FontSpans *font[ SZ];
  uint8_t   color_pair_idx[ SZ ];
  uint16_t  effect[ SZ ];
};

#endif


struct Curses
{
  explicit Curses(

    // change name stride back to nx_
    uint16_t stride_,
    uint16_t ny_,
    uint16_t pdx_,
    uint16_t pdy_ );
 

    // change name stride back to nx_
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
  agg::rgba color_fg[ MCursesColorsMax ];  // agg::rgba == 32 bytes. 8 * 32 = 256 bytes.
  agg::rgba color_bg[ MCursesColorsMax ];


  // whether item needs to be redrawn
  bool changed[ MCursesCellsMax ];
  

  /////////////////////////////////////
  // character - dominant. only check other flags.
  // should be refreshed between draws.
  uint16_t character[ MCursesCellsMax ];

  // font to use. 0. for special glyph drawing actions
  const FontSpans *font[ MCursesCellsMax ];

  uint8_t  color_pair_idx[ MCursesCellsMax ];


  // IMPORTANT - blinking at different speeds is easy
  // just add an uint_16 array for.    eg. (ms / 500 %2 == 0) or (ms %200 == 0) 
  // eg. pass systemmillis instead of bool.

  // effect. effects
  // invert fg/bg == 0x01   blink == 0x10
  uint16_t effect[ MCursesCellsMax ];

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

// TODO if this is using c++. then would be better to get rid of init().
// void init( Curses & a);

void clear( Curses & a);

void to( Curses &a, int x, int y);

void right( Curses &a, int dx);

void down( Curses &a, int dy);

void color_pair_idx( Curses &a, uint8_t color_pair_idx );

void font( Curses &a, const FontSpans *font);

void effect( Curses &a, uint16_t v );

void ch_text( Curses &a, uint16_t ch);    // change character
void ch_effect( Curses &a);  // change the effect only at position.

// void text( Curses &a, const char *s, int dir);
void text( Curses &a, const char *s);

void render( Curses &a, rb_t &rb, bool blink );

// void draw_test1(Curses &a );
// void draw_test3(Curses &a );


