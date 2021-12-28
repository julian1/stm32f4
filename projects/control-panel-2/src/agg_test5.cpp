
#include <stdio.h>
#include <string.h>
#include <stdio.h> // snprintf


#include "agg.h"


#include "fsmc.h" // TFT_
#include "assert.h"
#include "util.h"
#include "format.h"

#include "fonts.h"



extern "C" int agg_test5()
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









/*
  /////////////////////
  mtx.reset();
  mtx *= agg::trans_affine_translation(50, 120);
  snprintf(buf, 100, "%ums", system_millis - start);
  drawText(rb, mtx, agg::rgba(0,0,1), buf );
  usart_printf("time %u\n", system_millis - start);
*/








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

#if 0

  // EXTR. if looks like if draw too fast. then flipping the scrollstart can introduce flicker

  uint32_t start = system_millis;

  rb.clear(agg::rgba(1,1,1));     // white .
  usart_printf("rb.clear() time %u\n", system_millis - start);

  char buf[100];

  /////////////////////
  agg::trans_affine mtx;
  mtx *= agg::trans_affine_scaling(1.8);
  mtx *= agg::trans_affine_translation(30, 70);
  static double volts = 7.159884 + (rand() % 3);
  snprintf(buf, 100, "%gV", volts);
  // const char *s = "hello123";
  drawText(rb, mtx, agg::rgba(0,0,1), buf );


  /////////////////////
  mtx.reset();
  mtx *= agg::trans_affine_scaling(1.8);
  mtx *= agg::trans_affine_translation(30, 140);
  snprintf(buf, 100, "%gmA", 3.0194 );
  drawText(rb, mtx, agg::rgba(0,0,1), buf );


  /////////////////////
  mtx.reset();
  mtx *= agg::trans_affine_translation(50, 180);
  snprintf(buf, 100, "%ums", system_millis - start);
  drawText(rb, mtx, agg::rgba(0,0,1), buf );
  usart_printf("time %u\n", system_millis - start);

#endif

