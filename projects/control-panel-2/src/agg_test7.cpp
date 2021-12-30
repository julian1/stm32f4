
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
  like vt100, mcurses, terminal
  - cursor notional grid space.
    - for diff/delta optimisation
    - for hittesting
    - for knowing which font size to use.
    - for focus.
  ------
  possible a button - could be drawn as similar effect. eg. like background color. but rounded corners.
  a larger grid in a different view - for numeric keypad.

*/

struct A
{

  uint16_t ny; 
  uint16_t stride; // x dim


  // these positions are determined by fontsize, and our layout which may not be cartesian
  // can be pointers. and allocate later.
  uint16_t notional_x[ 50* 20 ];
  uint16_t notional_y[ 50* 20 ];

  // required in order to draw
  uint8_t fontsize[ 50 * 20 ] ; 

  /////////////////////////////////////
  // character
  uint16_t character[ 50* 20 ];
  uint16_t color[ 50* 20 ];
  // effect. effects - like background color are going to be hard.
  uint16_t effect[ 50* 20 ];

  // void (*pf[50 * 20 ] )(void )     callback

  // where text was actually placed, due to proportionate font spacing.
  // uint16_t text_x[ 50* 20 ]; // text actual position.
  // uint16_t text_y[ 50* 20 ];

  // NO. we diff compare - to evaluate what needs to be redrawn.
  // bool changed [ 50 * 20 ] ;


  // draw cursor
  uint16_t cursor_x;
  uint16_t cursor_y;


};


/*
  EXTR. if using double buffering. then any write - would be done on BOTH structures.
  we want cursor_to () rather than text... to enable chaining.

  this would mean we have 4 of these structures. which is ok. eg. current/new for each screen flip.
*/

void cursor_to( A &a, int x, int y)
{
  // use this instead of set_notional
  a.cursor_x = x;
  a.cursor_y = y;
}

void cursor_down( A &a) 
{
  // move next line down

  ++ a.cursor_y ;
}


void set_notional(A &a, int nx, int ny )
{
  int i = a.cursor_x + a.cursor_y * a.stride;
  a.notional_x[ i] = nx;
  a.notional_y[ i] = ny;
}


#if 0
void get_notional(const A &a, int *x, int *y)
{
  int i = a.cursor_x + a.cursor_y * a.stride;
  *x = a.notional_x[ i]
  *y = a.notional_y[ i]
}
#endif


// if the effect is blinking. or inverse.
// inverse ok. with proportional fonts.   we know the glyph, and can calculate the ....
// or inverse... is just for an effect per char.
// just draw a square under the text.

// compute actual positions of everything. first.


void text( A &a, const char *s)
{
  // eg. text expand right.

  int i = a.cursor_x + a.cursor_y * a.stride;

  while(*s) {

      // if(x > a.stride) {
      // could clip
      // }
      a.character[i] = *s;

      // we can actually wrap here.... kkkkk

      // should also set current color. and current effect.
      i++;
      s++;
  }
}


void text_indentright( A &a, const char *s)
{
  // eg. expand left.

}


void init_cursor_coordinates( A & a)
{
  a.stride = 33;
  a.ny = 19;

  int y_accum = 0;

  for(unsigned y = 0; y < 20; ++y )  {

    int x_accum = 0;
    for(unsigned x = 0; x < a.stride; ++x )  {

      cursor_to(a, x, y);
      set_notional(a, x_accum, y_accum );
      /*
      std::cout <<  x << ", " << y << "    ";
      get_notional(a);
      std::cout << "\n";
      */

      // the font is 18...
      x_accum +=  14;
    }
    y_accum += 14;
  }

}


// hmmmm....


void set_callback( A &a, void *func, void *arg)
{

}



void render( A &a, rb_t &rb )
{

  for( unsigned i = 0; i < a.ny * a.stride ; ++i ) {
  
    // eg. 0,0 
    int x = a.notional_x[ i];
    int y = a.notional_y[ i];

    // need a single character drawer also
    drawSpanText(rb,  arial_span_18, x, y, agg::rgba(0,0,1), "2" ); // 
  }


}

// so we want the full alphabet for 18 font. see how the squares fit.
// actually drawing the alphabet would be interesting.




extern "C" int agg_test7()
{

  static A a;
  static bool first = true;
  if(first) { 
    
    init_cursor_coordinates( a);

    first = false;
  }


  // persist the page that we need to draw
  static int page = 0; // page to use
  page = ! page;




  // set up our buffer
  pixfmt_t  pixf(  page *  272 );
  rb_t    rb(pixf);


  rb.clear(agg::rgba(1,1,1));     // white .

  uint32_t start = system_millis;
  // ok, this works. so maybe there is memory corruption somewhere.


  render( a , rb );


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

