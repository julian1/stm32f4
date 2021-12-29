
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

  ///////////////
  static int count = 715982;
  ++count;
  double volts = double(count) / 100000.f;
  char buf[100];
  format_float(buf, 100, 5, volts );

  drawSpanText(rb,  arial_span_72,    50, 100 , agg::rgba(0,0,1), buf );
  // drawSpanText(rb,  arial_span_18,    50, 150 , agg::rgba(0,0,1), "7.000V" );



    {
    agg::rasterizer_scanline_aa<> ras;
    agg::scanline_p8 sl;


    agg::ellipse e;

    e.init(20 , 20, 3, 3, 16);

    // ok. adding the path to the rasterizer hangs.
    // ras.add_path(e);

    //agg::render_scanlines_aa_solid(ras, sl, rb, agg::rgba(1,0,0));

  }




#if 1
/// none of this works.
  agg::rounded_rect rect( 10, 10, 100, 50, 5); 
  rect.normalize_radius();

  // agg::trans_affine mtx;
  // mtx *= agg::trans_affine_scaling(1.f);
  
  agg::rasterizer_scanline_aa<> ras;
  agg::scanline_p8 sl;

  // OK. when we add the path to rasterizer it fails.
  // ras.add_path(rect);

// hangs even if don't call this
//  agg::render_scanlines_aa_solid(ras, sl, rb, agg::rgba(1,0,0));

  // agg::render_scanlines(ras, sl, ren);

#endif




// locks up...
  // drawPath(rb, rect, mtx , agg::rgba(1,0,0) );


  usart_printf("spans time  %u\n", system_millis - start);

  // usart_printf("done drawSpans() \n");


  // lcd synchronization, wait until not in vertical blanking mode
  while( getTear() ) {
    // usart_printf("tear hi\n" );
  };



  // flip the newly drawn page in
  setScrollStart( page *  272 );


    return 0;
}




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



