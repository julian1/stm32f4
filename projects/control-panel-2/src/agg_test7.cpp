

#include "curses.h"
#include "streams.h"


/*
  - navigation,
    - page
    - menus
    - submenus (a different page)
    - editable fields

  - showing the navigation / and switching the menu item with focus.
  - eg. bitfields representing navitation paths  - jump between menu/ submenu or when scrolling menu entries
  -

*/


/*

Using arm-none-eabi-gcc as the reference for an STM32 build, neither take any
flash space at all.

Global and static variables that are not declared const go either into the
.data section if they require startup initialisation or into .bss if they
don't. Both of those segments are placed into SRCursesM by your linker script. If
you're doing C++ then static C++ classes end up in .bss.

If you do declare them const then they'll be placed into the .rodata section
which, if you consult your linker script you should find being located into a
subsection of .text which is in flash. Flash is usually more plentiful than
SRCursesM so do make use of const where you can.

Finally, the optimizer can come along and totally rearrange anything it sees
fit, including the elimination of storage in favour of inlining.

*/








static uint32_t last_draw_time = 0;

static void draw_test1(Curses &a )
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
  text(a, "     settings     ");

  to(a, 5, 5);
  (focus  == 1) ? effect(a, 0x01) : effect(a, 0x00);
  text(a, "whootj");
  to(a, 18, 5);
  text(a, "123.49");


  //////////
  to(a, 5, 6);
  color_pair_idx(a, 1); // red/white
  (focus == 2) ? effect(a, 0x01) : effect(a, 0x00);
  text(a, "foobar");

  to(a, 18, 6);
  effect(a, 0x01 << 2);   // blink
  text(a, "678mV");


  //////////
  to(a, 5, 7);
  color_pair_idx(a, 0); // blue/white
  (focus  == 3) ? effect(a, 0x01) : effect(a, 0x00);
  text(a, "drawtime");
  // effect(a, 0x00);
  to(a, 18, 7);
  snprintf(buf, 100, "%ums", last_draw_time);
  text(a, buf);
  // effect(a, 0x00);



  to(a, 5, 8);
  color_pair_idx(a, 0); // blue/white
  (focus  == 4) ? effect(a, 0x01) : effect(a, 0x00);
  text(a, "rotary");
  to(a, 18, 8);
  // snprintf(buf, 100, "%d %d", rotary, focus );
  text(a, "whoot");


/*
  // larger font
  font(a, &arial_span_72 );
  color_pair_idx( a, 0 );      // blue white
  to(a, 10, 10);
  text(a, "99");
*/

}






static void draw_test2(Curses &a )
{
  // grid spacing for text is quite different than for keypad button spacing.
  color_pair_idx(a, 0); // blue/white
  //effect(a, 0x01);        // invert
  effect(a, 0x00);        // normal
  // font(a, &arial_span_72 ); // large font
  font(a, &arial_span_18 ); // large font
  to(a, 0, 4);
  text(a, "+23.456mV");

  effect(a, 0x00);        // normal
  to(a, 1, 5);
  text(a, "3.4mCurses");   // remove the 1 argument.


}



static void draw_test3(Curses &a )
// void draw_test_charset( Curses &a, rb_t &rb )
{
  printf("draw_test3()\n");

  font(a, &arial_span_18 ); // large font

  // fill screen with chars
  // useful for test, check sizing, see what chars exist in a fontface
  int i = 0;
  for(unsigned y = 0; y < a.ny - 1; ++y)
  for(unsigned x = 0; x <  a.stride  - 1; ++x) {

    to(a, x, y);
    ch_text(a, i++ % 0xff );
    // ch_text(a, 'a' );

    // int x1 = x * a.pdx;
    // int y1 = y * a.pdy;
    // drawSpanChar(rb,  arial_span_18, x1, y1, agg::rgba(0,0,1), i++ % 0xff);
  }
}




static void draw_test4(Curses &a )
{
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
  // IMOPRTCursesNT negative numbers are really not working well. because of modulo?
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
    ch_text( a, buf[ i] );
  }

  //////////////////////////
  // edit value. we will edit the buffer. need to make it static.

  // how do we couple/edit a value????    use a delta. No.

  // take counter_pos at time we enter.
  // it would be much easier if we could directly change the value.  by coupling.
  // use an interupt?

  // ECursesSY.
  // adjust between, timer_counter and last_timer_counter  - update teh value.
  // eg. just do it every time.

}






static void draw_test5_keypad_test(Curses &a )
{
  // draw a kepad
  color_pair_idx(a, 0); // blue/white
  // font(a, &arial_span_72 ); // large font
  font(a, &arial_span_18 ); // large font
  to(a, 1, 1);
  text(a, "123");
  to(a, 1, 2);
  text(a, "456m");
  to(a, 1, 3);
  text(a, "789u");
  to(a, 1, 4);
  text(a, "-0.");

}



void print_stack_pointer()
{
  // https://stackoverflow.com/questions/20059673/print-out-value-of-stack-pointer
  // non-portable.
  void* p = NULL;
  usart_printf("%p   %d\n", (void*)&p,  ( (unsigned)(void*)&p)  - 0x20000000   );
  // return &p;
}




// 72 size text is much too spaced out.

extern "C" int agg_test7( Curses & a, int arg)
{


  // persist the page that we need to draw
  static int page = 0; // page to use
  page = ! page;



  // set up our buffer
  pixfmt_t  pixf(  page *  272 );
  rb_t    rb(pixf);


  // rb.clear(agg::rgba(1,1,1));     // bg white .
  rb.clear(agg::rgba(0,0,0));       // bg black


  uint32_t start = system_millis;
  // ok, this works. so maybe there is memory corruption somewhere.

  ////////////////////////////////////


  // print_stack_pointer();


  // need a better name. this is not render
  switch(arg) { 
    case 1: draw_test1(a ); break;
    case 2: draw_test2(a ); break;
    case 3: draw_test3(a ); break;
    case 4: draw_test4(a ); break;
    case 5: draw_test5_keypad_test(a ); break;
  }

  int blink = (system_millis / 500) % 2;
  // usart_printf("blink %u\n", blink );


  render( a , rb,  blink );
  // render( b , rb,  blink );

  // draw_test_charset( a, rb );


  // usart_printf("draw time  %u\n", system_millis - start);
  // last_draw_time = system_millis - start;

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





////////////////////////
  // (x % 9)  + '0'    // to get digits.     actually we have to take the number.
                    // we actually edit the underlying value.
                    // or edit a textfield.  and only after edit, convert to digit.
                    // need a cancel.

                    // HCursesVE a SINGLE edit buffer. so that if item is selected. we copy value into that.
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



