/*
  simple curses like char output
*/

#include <stdio.h>
#include <string.h>
#include <stdio.h> // snprintf


#include "agg.h"


#include "fsmc.h" // TFT_
#include "assert.h"
#include "util.h"
#include "format.h"

#include "fonts.h"

#include "agg_rounded_rect.h"
#include "agg_ellipse.h"


#include "agg_scanline_p.h"
#include "agg_rasterizer_scanline_aa.h"

#include "agg_renderer_scanline.h"


/*
  - we have the highlight now. eg. inverted.
    so we could try hittesting.
    or a focus.

  - should try to draw larger chars on another grid
  - draw a border / divider.
  - if do a numeric keypad? - then can enter values.
  - if add digital pot - then button can toggle - shift focus/ shift value.  
      toggling the unit. not so fun.
  ---------

  // having a switch statement for focus navigation
  // eg. if x has focus now and get down key etc.
  // i think would be easier. issue of individual digits. having focus versus word.
 /////////////////
  // ok. now move a couple more chars right. and a number



*/



/*
  ---
  like vt100, mcurses, terminal
  - cursor notional grid space.
    - for diff/delta optimisation
    - for hittesting
    - for knowing which font size to use.
    - for focus.
  ------
  possible a button - could be drawn as similar effect. eg. like background color. but rounded corners.
  a larger grid in a different view - for numeric keypad.
  -------------

  need highlight effect. can then test focus/hit test. in order to change set values.
  ----------

  possible outline fonts will be fast enough.
    because most values will only change in the last one or two digits..
    eg. so maybe 3 digits need to be redrawn. for V,A,W.
  --------------

  need to try to draw border. very important.   using hor. and vert. and corner glyphs.
    but not with anti antialiased - lines - want fixed pixel.
    might be able to do this easily without glyphs. by just using.  still need
    - either font/overloaded glyph.   or another array.

  -------------------

  ----- HMMMMM.....
    we cannot just render small text area. unless can correctly clear that area with background color.
      - problem of rb.clear. means need to redraw everything. to get the white background under the text.
      - if use porportionate spacing. then we would need to clear the old first.  to avoid the clear/redraw everything.
      --------------------
      EXTR. no it's easy./ EXCELLENT>
        ******** if a char changes - then we use the spans and render in the background color ********
        - this will work for proportionate fonts.

  is there a character that takes the entire space. then can
  no.   might need pixel level drawing underneath.

  - we need an effect. to do focus.
  - if each font was determined by two colors. ...  then could just invert the drawing.


  -------------------
  // we should do fg/bg - even if don't use it finally.
  // Also special glphys.
*/


/*
  generating a list o...
    - is much same effec

  setOrigin( )
  setCursorDxDy( 10, 10).  -- eg. for easier font. positioning. even if use proportional width.
  down()
  right(5)
  color()
  effect( blink);
  text( "whoot"); // advance  (argument doesn't propagate to affect others ) .

  - this kind of thing should be much better - for drawing buttons/ .

  - when something changes - then everything after changes.
  - this enables us to serialize() - and compare - to create deltas.
  - it is basically only text values that will change. and effects like blinking.



*/

#define MAXCELLS (33 * 17)

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

  // no concept of font size here.

  /////////////////////////////////////
  // character - dominant. only check other flags.
  // should be refreshed between draws.
  uint16_t character[ MAXCELLS ];

  // font to use. 0. for special glyph drawing actions
  const FontSpans *font[ MAXCELLS ];

  /*
    EXTR.
    having fg/bg colors are very powerful concept.  means can do,
        - inverted fg/bg for focus/ emphasis. blink.
        - alpha blending/sub-pixel accuracy - without having to read the LCD hardware screen memory / faster. less complicated.
        - delta change drawing - by drawing spans in bg color to clear them - to avoid full screen clear() /and redraw everything.
  */
  // so this thing is memory intensive
  // and needs constructor
  agg::rgba color[ MAXCELLS ];
  agg::rgba color_bg[ MAXCELLS ];

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

  // draw color
  agg::rgba cursor_color;
  agg::rgba cursor_color_bg;

  uint16_t cursor_effect;

  // rather than copy all these individually. might have a cell structure


};







// order?????
int index( A &a, int x, int y )
{
  int i = (y * a.stride ) + x   ;
  assert(i < MAXCELLS);
  // return (y * a.stride ) + x   ;
  return i;
}





void init( A & a)
{
  // change name init().
/*
  // dims
  a.stride = 33;
  a.ny = 17;
  a.pdx = 14;
  a.pdy = 16;
*/
  // defaults/current state
  a.cursor_font = & arial_span_18;
  a.cursor_color = agg::rgba(0,0,1);
  a.cursor_x = 0;
  a.cursor_y = 0;

  usart_printf("stride=%u, ny=%u\n" , a.stride, a.ny);

  for(unsigned y = 0; y < a.ny; ++y)
  for(unsigned x = 0; x < a.stride; ++x) {

      int i = index(a, x, y);

      a.character[i] = 0;
      a.font[i] = 0;
      a.effect[i] = 0;

  }
}







/*
  EXTR. if using double buffering. then any write - would be done on BOTH structures.
  we want to () rather than text... to enable chaining.

  this would mean we have 4 of these structures. which is ok. eg. current/new for each screen flip.
*/

void to( A &a, int x, int y)
{
  // use this instead of set_notional
  a.cursor_x = x;
  a.cursor_y = y;
}



void right( A &a, int dx)
{
  // chage name horiz() or horiz_to( ); etc
  a.cursor_x += dx;
}

void down( A &a, int dy)
{
  // TODO change name vert()
  // can have +/- args
  a.cursor_y += dy;
}

void color( A &a, const agg::rgba &color)
{
  a.cursor_color = color;
}

void color_bg( A &a, const agg::rgba &color)
{
  a.cursor_color_bg = color;
}

void font( A &a, const FontSpans *font)
{
  a.cursor_font = font;
  // const FontSpans *font[ 50* 20 ];
}

void effect( A &a, uint16_t v )
{
  a.cursor_effect = v;
}








//////////////////////


// if the effect is blinking. or inverse.
// inverse ok. with proportional fonts.   we know the glyph, and can calculate the ....
// or inverse... is just for an effect per char.
// just draw a square under the text.

// compute actual positions of everything. first.


void text( A &a, const char *s, int dir)
{
  assert(dir == 1 || dir == -1);

  // usart_printf("char before '%c'   ", *s );
  unsigned len = strlen(s);
  if(dir < 0) {
    // position at end
    s += len - 1; // what the fucking fuck.
  }

  for(unsigned j = 0; j < len; ++j)
  {
      // could clip
      // if(x > a.stride) ...

      int i = index( a, a.cursor_x , a.cursor_y);
      assert(i < 50 * 20);

      // set state
      a.character[i] = *s;
      a.color[i] =    a.cursor_color;
      a.color_bg[i] = a.cursor_color_bg;
      a.font[i] =     a.cursor_font;
      a.effect[i] =   a.cursor_effect;

      // we can pass the stride to use... as argument.
      a.cursor_x += dir;
      s += dir;
  }
}







void set_callback( A &a, void *func, void *arg)
{

}


void render_test_charset( A &a, rb_t &rb )
{
  // fill screen with chars
  // useful for test, check sizing, see what chars exist in a fontface
  int i = 0;
  for(unsigned y = 0; y < a.ny; ++y)
  for(unsigned x = 0; x < a.stride; ++x) {


    int x1 = x * a.pdx;
    int y1 = y * a.pdy;

    drawSpanChar(rb,  arial_span_18, x1, y1, agg::rgba(0,0,1), i++ % 0xff);
  }
}



void render( A &a, rb_t &rb, bool blink )
{
  // change name renderChars?

  for(unsigned y = 0; y < a.ny; ++y)
  for(unsigned x = 0; x < a.stride; ++x) {

    int i = index(a, x, y);
    assert(i < 50 * 20);

    uint16_t ch = a.character[ i ];
    const agg::rgba & color = a.color[ i ];
    const agg::rgba & color_bg = a.color_bg[ i ];
    const FontSpans *font = a.font[ i];
    uint16_t effect = a.effect[ i];

    if(ch != 0) {

      assert(font);

      int x1 = x * a.pdx;
      int y1 = y * a.pdy;

      // usart_printf( "x=%u y=%u\n", x1, y1);

      // blink effect on on and blink set
      if( (effect & (0x01 << 2)) == 0
        || (effect & (0x01 << 2) && blink)
      ) {


        if(( effect & 0x01) == 0 ) {
          // not inverted flag off
          // draw background
          rb.copy_bar(x1, y1, (x1 + 14) - 1, (y1 - 16) + 1,   color_bg );
          // draw char
          drawSpanChar(rb, *font, x1, y1 - 1, color, ch  );
        }
        else if ( effect & 0x01) {
          // draw inverted
          rb.copy_bar(x1, y1, (x1 + 14) - 1, (y1 - 16) + 1,   color );
          drawSpanChar(rb, *font, x1, y1 - 1, color_bg, ch  );
        }

      }

    }
  }
}



void render_test1(A &a )
{

  // terminfo codes, 
  // https://invisible-island.net/ncurses/man/terminfo.5.html
  // fonts for vt100..  has vert/horz. and corners. 
  // https://blog.adafruit.com/2019/03/29/raster-crt-typography-the-glyphs-drawn-by-dec-vt100-and-vt220-terminals-typeography-dec-vintagecomputing-fonts/

  // so we can diff the structure with the last structure to see if anything changed.
  font(a, &arial_span_18 );
  color(a, agg::rgba( 0,0,1));
  color_bg(a, agg::rgba( 1,1,1)); // white

  // 0x6a
  to(a, 5, 4);
  // text(a, "-------|-----------\x6a", 1);
  effect(a, 0x01);  // invert
  text(a, "     settings     ", 1);
  effect(a, 0x00);  // normal

  to(a, 5, 5);
  text(a, "whootj", 1);
  to(a, 18, 5);
  // right(a, 3);
  text(a, "123.49", -1);


  to(a, 5, 6);
  color(a, agg::rgba( 1,0,0));

  // focus...
  // if(x_has_focus)
  effect(a, 0x01);  // invert
  text(a, "foobar", 1);
  effect(a, 0x00);

  to(a, 18, 6);

  effect(a, 0x01 << 2);   // blink
  text(a, "678mV", -1);
  effect(a, 0x00);

  // larger font
  font(a, &arial_span_72 );
  color(a, agg::rgba( 0.7,0.7,1));
  to(a, 10, 10);
  text(a, "99", +1);

}



void render_test2(A &a )
{
  font(a, &arial_span_72 ); // large font
  to(a, 1, 1);
  text(a, "123", 1);
  to(a, 1, 2);
  text(a, "456m", 1);
  to(a, 1, 3);
  text(a, "789u", 1);
  to(a, 1, 4);
  text(a, "-0.", 1);

}


// 72 size text is much too spaced out.

extern "C" int agg_test7()
{

  static A a(33, 17, 14, 16  );
  static A b(6,5, 60, 60  );

/*
  a.stride = 33;
  a.ny = 17;
  a.pdx = 14;
  a.pdy = 16;
*/

  static bool first = true;
  if(first) {
    // move these to constructor?
//    init( a);

    init( b);   // hangs with no output. weird. run out of sram?
                    // because static initialization constructor runs before - we have usart configured?
                    // by itself we can initialize
    UNUSED(b);
                    // but 
    first = false;
  }


  // persist the page that we need to draw
  static int page = 0; // page to use
  page = ! page;




  // set up our buffer
  pixfmt_t  pixf(  page *  272 );
  rb_t    rb(pixf);


  rb.clear(agg::rgba(1,1,1));     // bg white .


  uint32_t start = system_millis;
  // ok, this works. so maybe there is memory corruption somewhere.

  ////////////////////////////////////

  // render_test1(a );
  render_test2(b );

  int blink = (system_millis / 500) % 2;
  // usart_printf("blink %u\n", blink );


  // render( a , rb,  blink );
  render( b , rb,  blink );


  // render_test_charset( a, rb );


  usart_printf("draw time  %u\n", system_millis - start);

  // usart_printf("done drawSpans() \n");


  // lcd synchronization, wait until not in vertical blanking mode
  while( getTear() ) {
    // usart_printf("tear hi\n" );
  };



  // flip the newly drawn page in
  setScrollStart( page *  272 );


    return 0;
}



/*

Using arm-none-eabi-gcc as the reference for an STM32 build, neither take any
flash space at all.

Global and static variables that are not declared const go either into the
.data section if they require startup initialisation or into .bss if they
don't. Both of those segments are placed into SRAM by your linker script. If
you're doing C++ then static C++ classes end up in .bss.

If you do declare them const then they'll be placed into the .rodata section
which, if you consult your linker script you should find being located into a
subsection of .text which is in flash. Flash is usually more plentiful than
SRAM so do make use of const where you can.

Finally, the optimizer can come along and totally rearrange anything it sees
fit, including the elimination of storage in favour of inlining.

*/








#if 0
/// none of this works.
  agg::rounded_rect rect( 10, 10, 100, 50, 5);
  rect.normalize_radius();
  // agg::trans_affine mtx;
  // mtx *= agg::trans_affine_scaling(1.f);

  agg::rasterizer_scanline_aa<> ras;
  agg::scanline_p8 sl;

  // Drawing as an outline
//  agg::conv_stroke<agg::rounded_rect> p(rect);
//  p.width(1.0);
  ras.add_path(rect);
  // ren.color(m_white_on_black.status() ? agg::rgba(1,1,1) : agg::rgba(0,0,0));
  // agg::render_scanlines(ras, sl, ren);
  // ras.add_path( curve /*trans*/ );
  // agg::render_scanlines_aa_solid(ras, sl, rb, agg::rgba(0,0,1));


//  agg::render_scanlines_aa_solid(ras, sl, rb, agg::rgba(1,0,0));

  // agg::render_scanlines(ras, sl, ren);

#endif


#if 0
  ///////////////

  {
  static int count = 715982;
  ++count;
  double volts = double(count) / 100000.f;
  char buf[100];
  format_float(buf, 100, 5, volts );

  //drawSpanText(rb,  arial_span_72,    50, 100 , agg::rgba(0,0,1), buf );
  drawSpanText(rb,  arial_span_18,    50, 150 , agg::rgba(0,0,1), "7.000V" );

  }
#endif

