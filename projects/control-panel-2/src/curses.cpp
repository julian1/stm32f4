/*
  what we have might be enough to progress. if can set values.
  ---
  - look at 2450. etc. most things could be done with a double cursor coordinate approach.
  - simple rgba color mapping of cursor gridding still results in a 40k data structure.
  ----------

  So. focus navigation.  could be done per character.

  Actually - we could cursor map the focus naviation. (or use switch statement).
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

void color_pair_idx( A &a, uint8_t color_pair_idx )
{
  // usart_printf("setting cursor_color_pair %u\n", color_pair_idx );
  a.cursor_color_pair_idx = color_pair_idx;
}



void font( A &a, const FontSpans *font)
{
  a.cursor_font = font;
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



void textch( A &a, uint16_t ch)
{
  // does not increment cursor.
  int i = index( a, a.cursor_x , a.cursor_y);

  // set state
  a.character[i] =      ch;
  a.font[i] =           a.cursor_font;
  a.effect[i] =         a.cursor_effect;
  a.color_pair_idx[i] = a.cursor_color_pair_idx;
}




void text( A &a, const char *s, int dir)
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



      textch( a, *s);
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

void set_callback( A &a, void *func, void *arg)
{

}
#endif


void draw_test_charset( A &a, rb_t &rb )
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

    int i       = index(a, x, y);
    uint16_t ch = a.character[ i ];

    if(ch != 0) {

      uint8_t color_pair_idx = a.color_pair_idx[i];
      assert(color_pair_idx < MAXCOLORPAIRS);

      const agg::rgba & color_fg = a.color_fg[ color_pair_idx ];
      const agg::rgba & color_bg = a.color_bg[ color_pair_idx ];

      const FontSpans *font = a.font[ i];
      uint16_t effect = a.effect[ i];


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
          // rb.copy_bar(x1, y1, (x1 + 14) - 1, (y1 - 16) + 1,   color_bg );
          rb.copy_bar(x1, y1, (x1 + a.pdx) - 1, (y1 - a.pdy) + 1,   color_bg );
          // draw char
          drawSpanChar(rb, *font, x1, y1 - 1, color_fg, ch  );
        }
        else if ( effect & 0x01) {
          // draw inverted
          // rb.copy_bar(x1, y1, (x1 + 14) - 1, (y1 - 16) + 1,   color_fg );
          rb.copy_bar(x1, y1, (x1 + a.pdx) - 1, (y1 - a.pdy) + 1,   color_fg );
          drawSpanChar(rb, *font, x1, y1 - 1, color_bg, ch  );
        }

      }

    }
  }
}





static uint32_t last_draw_time = 0;

void draw_test1(A &a )
{
  /*
    should switch the page state.
    to determine which page we are drawing.
    --
    So. we should be passing the state as an argument.
  */

  // terminfo codes,
  // https://invisible-island.net/ncurses/man/terminfo.5.html
  // fonts for vt100..  has vert/horz. and corners.
  // https://blog.adafruit.com/2019/03/29/raster-crt-typography-the-glyphs-drawn-by-dec-vt100-and-vt220-terminals-typeography-dec-vintagecomputing-fonts/


  // int rotary = timer_get_counter(TIM1);
  int focus = 3 ; // (rotary / 4 ) %  5;

  char buf[100];

  // usart_printf("rotary %d   ", rotary  );    // weird always 0...
  // usart_printf("rotary mod 16 %d\n", (rotary / 4 ) %  4  );



  // so we can diff the structure with the last structure to see if anything changed.
  font(a, &arial_span_18 );
  color_pair_idx(a, 0 );  // blue,white

  //////////
  to(a, 5, 4);
  (focus == 0) ? effect(a, 0x01) : effect(a, 0x00);
  text(a, "     settings     ", 1);

  to(a, 5, 5);
  (focus  == 1) ? effect(a, 0x01) : effect(a, 0x00);
  text(a, "whootj", 1);
  to(a, 18, 5);
  text(a, "123.49", -1);


  //////////
  to(a, 5, 6);
  color_pair_idx(a, 1); // red/white
  (focus == 2) ? effect(a, 0x01) : effect(a, 0x00);
  text(a, "foobar", 1);

  to(a, 18, 6);
  effect(a, 0x01 << 2);   // blink
  text(a, "678mV", -1);


  //////////
  to(a, 5, 7);
  color_pair_idx(a, 0); // blue/white
  (focus  == 3) ? effect(a, 0x01) : effect(a, 0x00);
  text(a, "drawtime", 1);
  // effect(a, 0x00);
  to(a, 18, 7);
  snprintf(buf, 100, "%ums", last_draw_time);
  text(a, buf, -1);
  // effect(a, 0x00);



  to(a, 5, 8);
  color_pair_idx(a, 0); // blue/white
  (focus  == 4) ? effect(a, 0x01) : effect(a, 0x00);
  text(a, "rotary", 1);
  to(a, 18, 8);
  // snprintf(buf, 100, "%d %d", rotary, focus );
  text(a, "whoot", -1);


/*
  // larger font
  font(a, &arial_span_72 );
  color_pair_idx( a, 0 );      // blue white
  to(a, 10, 10);
  text(a, "99", +1);
*/

}




////////////////////////
  // (x % 9)  + '0'    // to get digits.     actually we have to take the number.
                    // we actually edit the underlying value.
                    // or edit a textfield.  and only after edit, convert to digit.
                    // need a cancel.

                    // HAVE a SINGLE edit buffer. so that if item is selected. we copy value into that.
                    // then we edit it.
                    // rather than printing the value.
                    // will need to be static.
                    // need to test in isolation.

                    // doesn't even need to be a buffer. could be a value that we edit.

                    // copy once.
                    // fuck. it's getting complicated.
                    // integer or double?

                    // when we change the character then we want that
                    // we kind of have to have events. onfocus() onleavefocus() etc.

                    // OR.      we don't edit the copied value - instead a delta value (delta buf, or delta double/integer). and we just spin characters on that.
                    // then we don't need the events.

                    // not so easy. need to deal to rotate mV, uV, nV. etc.
                    // -----------------
                    // EXTR.  inject menu structure into controller / like pattern.


  // char delta_buf = '000.0000'
/*
  len = strlen(editbuf);
  for(unsigned i = 0; i < len; ++i ) {
    move_to( a, 5 + len, 8);
    ( i == focus ) ?  effect(a, 0x01) : effect(a, 0x00);

    // do we want a sinigle character?
    text(a, "rotary", 1);
  }
*/



void draw_test3(A &a )
{
  /*
      - needs to be separately editable buffer.  eg. edit then commit.
      which may require an enter button. and  a cancel button.
    - edit buf should be injected/ static
    - eg. we generically edit
    -------------------
    it's highly modal.   eg. dial adjusts menu item, then dial adjusts position in string. then dial adjusts value.
    suggests event state machine.
    -------------------

    3x separate controllers - for menu item.
        not nice. means three sepaate pointer structures.

    but an index is required for
      - 1. page, 2. menuitem, 3. digit/edit buf.

    events. eg. for switching mode. ef. finished edit - need ok or cancel.
      then shift from diti/edit buf.

    state machine is easier.
      - with index for page, menuitem, digit
      - events for - ok, cancel. shifting between modes.   (use buttons)
      ----------------

    potentially. need more than one level of nesting.

    This means a linked structure. that can navigate.

    menu level 1 - index 3
    menu level 2 - index 4
    editfield    - index 6.


  */

  //////////
  // mode  change edit position


  static char buf[100];
  bool first = true;
  if(first) {
    snprintf(buf, 100, "123456" );

    // if we try to write timer. it stops updating...
    /*
    timer_disable_counter(TIM1);
    timer_set_counter(TIM1, 3 * 4) ;
    timer_enable_counter(TIM1);
    */
    // but perhaps we could record a delta?

    first = false;
  }

  // try to edit
  // IMOPRTANT negative numbers are really not working well. because of modulo?
  int16_t rotary = 3; // timer_get_counter(TIM1) ;

  // usart_printf("%d %d %d\n", rotary, rotary_last , (rotary - rotary_last) );

  // negative numbers go modulo in negative direction....

  usart_printf("%d \n", (rotary / 4) % 10 );

  // directly coupling
  buf[ 3 ] = '0' + (rotary / 4) % 10;


  /////////////////////
  to(a, 5, 6);
  color_pair_idx(a, 0); // red/white

  unsigned len = strlen(buf);

  for(unsigned i = 0; i < len; ++i) {

    to(a, 5 + i, 6);

    // dial focus.
    if( i == 3)
      effect(a, 0x01) ;
    else
      effect(a, 0x00);

    // output the character
    textch( a, buf[ i] );
  }

  //////////////////////////
  // edit value. we will edit the buffer. need to make it static.

  // how do we couple/edit a value????    use a delta. No.

  // take counter_pos at time we enter.
  // it would be much easier if we could directly change the value.  by coupling.
  // use an interupt?

  // EASY.
  // adjust between, timer_counter and last_timer_counter  - update teh value.
  // eg. just do it every time.

}





