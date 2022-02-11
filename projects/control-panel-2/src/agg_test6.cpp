
#include <stdio.h>
#include <string.h>
#include <stdio.h> // snprintf


#include "agg.h"


#include "fsmc.h" // TFT_
#include "assert.h"
#include "util.h"
#include "streams.h"

#include "format.h"

#include "fonts.h"

#include "agg_rounded_rect.h"
#include "agg_ellipse.h"


#include "agg_scanline_p.h"
#include "agg_rasterizer_scanline_aa.h"

#include "agg_renderer_scanline.h"

extern "C" int agg_test6()
{

  // persist the page that we need to draw
  static int page = 0; // page to use
  page = ! page;




  // set up our buffer
  pixfmt_t  pixf(  page *  272 );
  rb_t    rb(pixf);


  rb.clear(agg::rgba(1,1,1));     // white .

  uint32_t start = system_millis;
  // ok, this works. so maybe there is memory corruption somewhere.


  {
    agg::rasterizer_scanline_aa<> ras;
    agg::scanline_p8 sl;
    agg::ellipse e;
    e.init(120, 20 ,    50, 20, 16);
    ras.add_path(e);
    agg::render_scanlines_aa_solid(ras, sl, rb, agg::rgba( 1  ,0,0));
  }


  {
    agg::rounded_rect rect( 10, 10  , 100, 40 , 10);
    rect.normalize_radius();
    agg::rasterizer_scanline_aa<> ras;
    agg::scanline_p8 sl;
    ras.add_path(rect);
    agg::render_scanlines_aa_solid(ras, sl, rb, agg::rgba(0,0, 1 ));
  }


  // Hmmmm. so just referencing the structure kills it.
  // too much stuff loaded into ram?
  // justk


  // usart_printf("addr of arial_span_72 %p\n" , & arial_span_72 );
  // but if we try to do the large text it fails.
  drawSpanText(rb,  arial_span_72,    50, 100 , agg::rgba(0,0,1), "7.14159" );


  drawSpanText(rb,  arial_span_18,    50, 150 , agg::rgba(0,1,0), "7.000V" );


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

