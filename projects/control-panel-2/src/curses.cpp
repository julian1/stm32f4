/*
  what we have might be enough to progress. if can set values.
  ---
  - look at 2450. etc. most things could be done with a double cursor coordinate approach.
  - simple rgba color mapping of cursor gridding still results in a 40k data structure.
  ----------

  So. focus navigation.  could be done per character.

  Cursesctually - we could cursor map the focus naviation. (or use switch statement).
  eg.    right[ ] ->  pos; // new x,y position
  and then auto fill or fill as we call text.
  -------
  No. switch is better. can be modal - from highlighting entire row, to word, to individual text. with a click.
  simple curses like char output
  character mapped display.
  ------------

  EXTR.
    - having
      focus map grid      - for moving between items.
      then another one      - for moving inside items.  eg characters in set value.
      then we edit the value.
      - switch between by clicking centre knob.

      EXTR. advantage - is that can localize the maps with the draw code.
        eg. static. and

      EXTR
        might need to be sparse.  eg. to handle a word. with multiple cells.
      EXTR
        no i think a switch might be easiest.   or 4. for each direction.
        and use an id for words.   or else scan the text.




    - then doulbe click button. to shift between them.
      nice.
    - we would need to set the invert flag for the cell also for drawing.
  ---------------

  rather than use a separate grid - for large text.  why not use free placement. and with proportionate spacing.
  as test.
  but still have
  ----
  button colors / gradients . should perhaps be pixmaps. not rendered.

*/

#include <stdio.h>
//#include <string.h>
#include <stdio.h> // snprintf



#include "assert.h"
#include "curses.h"


#include "streams.h"


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
    eg. so maybe 3 digits need to be redrawn. for V,Curses,W.
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
  // Curseslso special glphys.
*/


/*
  EXTR.

  draw sequenece - can use similar deta draw strategy
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
  ----------------------

  EXTR. hittesting.
    - we don not need an intermediate bounded rectable structure - structure for hittesting.
    - just play through the draw instructions - and calculate and test the bounds. as to what gets hit.
    - can play through - to test a hit point.



*/






// order?????
int index( Curses &a, int x, int y )
{
  int i = (y * a.stride ) + x   ;
  assert(i < MCursesXCELLS);
  // return (y * a.stride ) + x   ;
  return i;
}





void init( Curses & a)
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
  // a.cursor_color = agg::rgba(0,0,1);


  a.cursor_color_pair_idx = 0;  // blue white

  a.cursor_x = 0;
  a.cursor_y = 0;

#if 0
  // set up color pairs
  a.color_fg[ 0 ] = agg::rgba( 0,0,1); // blue
  a.color_bg[ 0 ] = agg::rgba( 1,1,1); // white

  a.color_fg[ 1 ] = agg::rgba( 1,0,0); // red
  a.color_bg[ 1]  = agg::rgba( 1,1,1); // white
#endif

#if 1
  // set up color pairs
  //a.color_fg[ 0 ] = agg::rgba( 1,1,0); // yellow
  a.color_fg[ 0 ] = agg::rgba( 1,0.90,0); // yellowy orange
  a.color_bg[ 0 ] = agg::rgba( 0,0,0); // black

  a.color_fg[ 1 ] = agg::rgba( 1,0,0); // red
  a.color_bg[ 1]  = agg::rgba( 0,0,0); // black
#endif



  /////////

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

void to( Curses &a, int x, int y)
{
  // use this instead of set_notional
  a.cursor_x = x;
  a.cursor_y = y;
}



void right( Curses &a, int dx)
{
  // chage name horiz() or horiz_to( ); etc
  a.cursor_x += dx;
}

void down( Curses &a, int dy)
{
  // TODO change name vert()
  // can have +/- args
  a.cursor_y += dy;
}

void color_pair_idx( Curses &a, uint8_t color_pair_idx )
{
  // usart_printf("setting cursor_color_pair %u\n", color_pair_idx );
  a.cursor_color_pair_idx = color_pair_idx;
}



void font( Curses &a, const FontSpans *font)
{
  a.cursor_font = font;
}

void effect( Curses &a, uint16_t v )
{
  a.cursor_effect = v;
}





//////////////////////


// if the effect is blinking. or inverse.
// inverse ok. with proportional fonts.   we know the glyph, and can calculate the ....
// or inverse... is just for an effect per char.
// just draw a square under the text.

// compute actual positions of everything. first.


void ch_effect( Curses &a)
{
  // change effect at current cur position, without changing character
  // does not increment cursor.
  int i = index( a, a.cursor_x , a.cursor_y);

  // set state
  a.effect[i] =         a.cursor_effect;
}




void ch_text( Curses &a, uint16_t ch)
{
  // change character, at current cur position

  // does not increment cursor.
  int i = index( a, a.cursor_x , a.cursor_y);

  // set state
  a.character[i] =      ch;
  a.font[i] =           a.cursor_font;
  a.effect[i] =         a.cursor_effect;
  a.color_pair_idx[i] = a.cursor_color_pair_idx;
}




void text( Curses &a, const char *s, int dir)
{
  // increments the cursor as it writes

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



      ch_text( a, *s);
/*
      int i = index( a, a.cursor_x , a.cursor_y);

      // set state
      a.character[i] =      *s;
      a.font[i] =           a.cursor_font;
      a.effect[i] =         a.cursor_effect;
      a.color_pair_idx[i] = a.cursor_color_pair_idx;
*/

      // we can pass the stride to use... as argument.
      a.cursor_x += dir;
      s += dir;
  }
}





#if 0

void set_callback( Curses &a, void *func, void *arg)
{

}
#endif


void draw_test_charset( Curses &a, rb_t &rb )
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



void render( Curses &a, rb_t &rb, bool blink )
{
  // change name renderChars?

  for(unsigned y = 0; y < a.ny; ++y)
  for(unsigned x = 0; x < a.stride; ++x) {

    int i       = index(a, x, y);
    uint16_t ch = a.character[ i ];

    if(ch != 0) {

      uint8_t color_pair_idx = a.color_pair_idx[i];
      assert(color_pair_idx < MCursesXCOLORPCursesIRS);

      const agg::rgba & color_fg = a.color_fg[ color_pair_idx ];
      const agg::rgba & color_bg = a.color_bg[ color_pair_idx ];

      const FontSpans *font = a.font[ i];
      uint16_t effect = a.effect[ i];


      assert(font);

      int x1 = x * a.pdx;
      int y1 = y * a.pdy;


      // flags
      // 0x01 invert.
      // 0x10 blink.

      if( effect & 0x01) {
          // invert effect flag, we always draw / never blank

          // blink effect off . so draw
          if( (effect & 0x10) == 0
          // or blink effect on and blink flag on. so draw
            || ((effect & 0x10 ) && blink)
          ) {
              // draw background
              rb.copy_bar(x1, y1, (x1 + a.pdx) - 1, (y1 - a.pdy) + 1,   color_bg );
              // draw char
              drawSpanChar(rb, *font, x1, y1 - 1, color_fg, ch  );
          }
          else {
              // draw inverted
              rb.copy_bar(x1, y1, (x1 + a.pdx) - 1, (y1 - a.pdy) + 1,   color_fg );
              drawSpanChar(rb, *font, x1, y1 - 1, color_bg, ch  );
            }
      }
      else

      // invert flag off.

      // blink effect off, and normal draw - in which case always draw
      if( (effect & 0x10) == 0
      // or blink effect on and blink flag on - then draw
        || ((effect & 0x10 ) && blink)
      ) {
          // draw background
          rb.copy_bar(x1, y1, (x1 + a.pdx) - 1, (y1 - a.pdy) + 1,   color_bg );
          // draw char
          drawSpanChar(rb, *font, x1, y1 - 1, color_fg, ch  );
        }

      // don't draw... should perhaps force draw the background



    }
  }
}





