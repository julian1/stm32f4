

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









static void draw_keypad_test(Curses &a )
{
  // draw a kepad
  color_pair_idx(a, 0); // blue/white
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

static void draw_test2(Curses &a )
{
  // grid spacing for text is quite different than for keypad button spacing.
  color_pair_idx(a, 0); // blue/white
  //effect(a, 0x01);        // invert
  effect(a, 0x00);        // normal
  font(a, &arial_span_72 ); // large font
  to(a, 0, 4);
  text(a, "+23.456mV", 1);

  effect(a, 0x00);        // normal
  to(a, 1, 5);
  text(a, "3.4mCurses", 1);


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

extern "C" int agg_test7()
{

  static Curses a( 33, 17, 14, 16 );
  // static Curses b(6,5, 60, 60  );
  static Curses b(10,6, 45, 50  );
/*
  sizeof(Curses) 40488 40k. hmmm.
  sizeof(agg::rgba) 32

  18k is the rgb data.

  sizeof(Curses) 5584  now 5.5k. after fixing color space. good.
  -------

  struct a only
  0x2001ffa4   130980

  struct a , b
  0x2001ff9c   130972

  so don't think it's working. or we already hit limit?
*/

  static bool first = true;
  if(first) {
    // move these to constructor?

    // trying to init both of these hangs...

    usart_printf("sizeof(agg::rgba) %u\n", sizeof(agg::rgba));
    usart_printf("sizeof(Curses) %u\n", sizeof(Curses));


    init( a);
    init( b);   // hangs with no output. weird. run out of sram?
                    // because static initialization constructor runs before - we have usart configured?
                    // by itself we can initialize
    // UNUSED(b);
                    // but
    first = false;
  }


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
  draw_test1(a );
  draw_test2(b );

  // draw_test3(a );

  int blink = (system_millis / 500) % 2;
  // usart_printf("blink %u\n", blink );


  render( a , rb,  blink );
  render( b , rb,  blink );

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



