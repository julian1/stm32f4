/*
  draw direct / without paging/ double buffering
*/

#include <stdio.h>
#include <string.h>


#include "assert.h"
#include "util.h"

#include "agg.h"
#include "fonts.h"


extern "C" int agg_test2()
{


  // set up our buffer
  pixfmt_t  pixf(  0  );
  // rb_t    rb(pixf);
  agg::renderer_base<pixfmt_t>   rb(pixf);

  while( ! getTear() ); // wait for tear to go high...
  // usart_printf("t_irq %u\n", getTear()  );

  uint32_t start;

  start = system_millis;
  rb.clear(agg::rgba(1,1,1));     // white .
  usart_printf("rb.clear() %ums\n", system_millis - start );


  // EXTR. this is a clear/fillRect that is not subpixel, and simple.
  // see, agg_renderer_base.h.
  start = system_millis;
  rb.copy_bar(20, 20, 100, 200, agg::rgba(1,0,0));
  usart_printf("copy_bar()  %ums\n", system_millis - start );

  // EXTR. IMPORTANT confirm we have floating point enabled.
  // looks ok.


  // http://agg.sourceforge.net/antigrain.com/demo/conv_contour.cpp.html
  // https://coconut2015.github.io/agg-tutorial/agg__trans__affine_8h_source.html
  agg::trans_affine mtx;
  // mtx.flip_y();
  // mtx *= agg::trans_affine_scaling(1.0, -1); // this inverts/flips the glyph, relative to origin. but not in place.
  mtx *= agg::trans_affine_translation(50, 50);   // this moves from above origin, back into the screen.
  mtx *= agg::trans_affine_rotation(10.0 * 3.1415926 / 180.0);
  mtx *= agg::trans_affine_scaling(2.0); // now scale it


  const char *s = "hello123";
  // drawText(rb , mtx, agg::rgba(0,0,1), s );
  drawOutlineText(rb, arial_outline, mtx, agg::rgba(0,0,1), s);


  // non anti aliased.
  // agg::renderer_scanline_bin_solid(


  // setOriginBottomLeft(); // cartesion/ fonts/ postscript

  // agg::path_storage            m_path2;
#if 0
    m_path.remove_all();
    m_path.move_to(10, 10);
    m_path.line_to(50, 10);
    m_path.line_to(50, 50);
    m_path.line_to(10, 50);
    m_path.line_to(10, 10);
    m_path.close_polygon();//agg::path_flags_cw);

  ras.reset();
  ras.add_path( m_path );
  agg::render_scanlines_aa_solid(ras, sl, /*renb*/ rb, agg::rgba(0,1,0));

#endif

  return 0;
}












/*
  same as main5.cpp except for packed rgb565
  pixfmt_rgb24 defined like this

  arduing fastish
    "Because CLEAR screen is going to be either black or white. You load the data bus once. Pulse WR line 240x320x2 times."

    OK. the first step here - would be to ignore alpha blending. and just try to write something


  think there's an example here. without aa.

    http://agg.sourceforge.net/antigrain.com/demo/rasterizers.cpp.html
      used to compare speed.

    http://agg.sourceforge.net/antigrain.com/demo/rasterizers2.cpp.html
  ---------------------

  rb.clear() 22ms
  agg text draw 21ms <- large

  reduced text size scale transform - from 2 to 1. and get half render speed.  good.
  rb.clear() 22ms
  agg text draw 13ms  <- small
  ----------

  - dumb screen client - mcu can be spi slave
    - dedicated to just rendering.  not supervisory.
    eg. board cpu can just send simple spi rendering commands. and receive pot and other inputs over spi (screen mcu raises interupt).
    - simple primitives. drawBar(pos); darwText(font, sisze, pos, text);  etc.  drawButton(font, pos, text);
    - and simple reverse inputs  screenprimitives
    - what about hit-testing. touchscreen / and mapping buttons/controls back.
    ------
    like xwindows / queue. just some simple serialized screen primitives.
    then put in a queue.  <- GOOD.    to match the data generation and the screen display.
    ------
    screen display has to manage paging timinng/ tear timing ..
    ------------

    we can test this functionality fairly simply. by replaying commands ... which is good.
    ---------
    EXTR. the spi send.   does not need to wait for the send. just put in a buffer - and use interupts to process the next byte.
      so there is no waiting on the spi output.  good.
    - like using a queue to communicate info between two separate threads.


  - use coroutine. on the draw. and/or perhaps queue of drawing operations.

  screen mcu v hardware mcu.
  ------
  - or if only need to respond - when reading the dac (every 1/10th sec etc). then it's ok.
    ranging depends on dac update.
    so everything really depends on if we can draw faster than the dac udpate rate.

  -------
  or use coroutines. in the drawing loop...


  -
  rate
*/




/*
  if rgb565 doesn't work.  ssd1963 can do 24 bit color. have conversions...
  should try to compile this and see if can implement

  actually how the fill operations workk - separate from the bitmap blting.
  possible exactly the same.
  --------------

  hmmmm...
  it supports bliting bitmap.   but has no actual fill function?.

  ssd1936 (not ssd1963)  has filled rectangle draw.
  ook.  but then how does Andy do it?   check code.

  well we could still test it. I suppose.
  -----------

  on an arduino - one just asserts the color on the bus. and then pulse the wr pin. which is pretty fast. maybe faster than bus mode.
    https://www.avrfreaks.net/forum/slow-working-lcd-ssd1963-controller-board
  ---------

  EXTR. it might

  ------------
  stm32 100MHz / 5 (bus data setup ) = 20MHz.
  480 * 272 * 2 =  261120 = 260000.
  So write speed should be 77fps.
  ------------


  EXTR. should just do all drawing with transforms... and using native agg primitives.
      - avoid a sizing/layout engine.
      - if have different screens then just draw them independently.
      -------
      eg. document html/ ps type model.

      - inject a struct with needed data - into the view render code.
      - use local switch - to flip tabs etc.
      - use update flags - indicating if have to rerender
  ------------
*/

/*
  TODO. could add a footprint square - to erase the character for redrawing.
        eg. just hold min/max dimensions.
        bit complicated because depends on mtx.
*/



/*
  What should we pass around????? the pixfmt???
  perhaps yes. so if we ever want to store state on it we can.
*/




    /*
      OK. it's possible we may want the origin at bottom left.  if it makes handling fonts easier.
      we can probably flip o
      what does postscript do.
      Note that we can flip the origin on the fly.
    */


#if 0
// #include "agg_pixfmt_rgb24.h"
// #include "agg_pixfmt_rgb.h"
#include "agg_pixfmt_rgb_packed.h"  // rgb565

#include "agg_conv_curve.h"
#include "agg_conv_contour.h"


#include "agg_basics.h"
// #include "agg_rendering_buffer.h"

#include "agg_rendering_buffer.h"

#include "agg_rasterizer_scanline_aa.h"

#include "agg_scanline_p.h"
#include "agg_renderer_scanline.h"


#include "agg_path_storage.h"

#include "agg_path_storage_integer.h"
#endif



#if 0

typedef agg::serialized_integer_path_adaptor<short int, 6>  font_path_type;

extern font_path_type *arial_glyph[256];
extern int arial_glyph_advance_x[256] ;
extern int arial_glyph_advance_y[256] ;


/*
  so we would move this into an include file? ....
  and then pass it from main?
*/

// template< class T>
static void drawText(rb_t & rb , agg::trans_affine &mtx, const char *s)
{
  // not even sure if it makes sense to factor this



    // we can move these out of loop if desired...
    agg::rasterizer_scanline_aa<> ras;
    agg::scanline_p8 sl;


    int x  = 0;

   uint32_t start = system_millis;

    for( const char *p = s; *p; ++p)  {

        char ch = *p;

        // whoot it links
        font_path_type *path = arial_glyph[ ch ];
        assert( path );

        // TODO add a translate() method... or add an adapter
        // should add a method translate( dx, dy );
        path->m_dx = x;

        x += arial_glyph_advance_x[ ch ];


        agg::conv_transform<font_path_type> trans( *path, mtx);
        agg::conv_curve<agg::conv_transform<  font_path_type > > curve(trans);

        ras.reset();
        ras.add_path( curve );
        agg::render_scanlines_aa_solid(ras, sl, rb, agg::rgba(0,0,1));

    }


    usart_printf("agg text draw %ums\n", system_millis - start );


}



#endif



