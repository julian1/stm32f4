

#include <stdio.h>
#include <string.h>
#include <assert.h>



#include <agg/font-outline.h>
#include <agg/tft-display.h>



extern "C" void tft_test4( tft_display_t *display)
// extern "C" int agg_test4()
{
  assert( display && display->magic == TFT_DISPLAY_MAGIC);


  // change the page
  display->page = ! display->page;


  // set up our buffer
  // pixfmt_t  pixf(  page *  272 );
  pixfmt_t  pixf( display->tft, display->page *  272 );
  agg::renderer_base<pixfmt_t>   rb(pixf);


  // EXTR. if looks like if draw too fast. then flipping the scrollstart can introduce flicker

  uint32_t start = *display->system_millis;

  rb.clear( agg::rgba(1,1,1));     // white .
  // usart_printf("rb.clear() time %u\n", system_millis - start);

  char buf[100];

/*
  if scaling too large. then get memory problems.

  1.8 was ok. before?
  1.5 bad.
  1.4 ok.  no. runs for a while then calls bad_alloc....
  1.3 ok.

  the path_storage gets too large?
  doesn't matter too much. if going to use the span text drawing.

*/

/*
   EXTR. apr. 2026.
  - scaling issue  - because not using a clipping box?0
*/

  /////////////////////
  agg::trans_affine mtx;
  mtx *= agg::trans_affine_scaling(1.3);
  mtx *= agg::trans_affine_translation(30, 70);
  static double volts = 7.159884 + (rand() % 3);
  snprintf(buf, 100, "%gV", volts);
  rb_font_outline_write(rb, arial_outline, mtx, agg::rgba(0,0,1), buf);



  /////////////////////
  mtx.reset();
  mtx *= agg::trans_affine_scaling(1.3);
  mtx *= agg::trans_affine_translation(30, 140);
  snprintf(buf, 100, "%gmA", 3.0194 );
  rb_font_outline_write(rb, arial_outline, mtx, agg::rgba(0,0,1), buf);


  /////////////////////
  mtx.reset();
  mtx *= agg::trans_affine_translation(50, 180);
  snprintf(buf, 100, "%lums", *display->system_millis - start);
  rb_font_outline_write(rb, arial_outline, mtx, agg::rgba(0,0,1), buf);


  // usart_printf("time %u\n", system_millis - start);


/*
  /////////////////////
  mtx.reset();
  mtx *= agg::trans_affine_translation(50, 120);
  snprintf(buf, 100, "%ums", system_millis - start);
  drawText(rb, mtx, agg::rgba(0,0,1), buf );
  usart_printf("time %u\n", system_millis - start);
*/


  /////////////


  /*
  // OK. this fixes flicker... but opposite of what we thought...
  // we flip the page when it is drawing. rather than when it's blanking.
  IMPORTANT.
    this is a blocking function and can block all display function
  */

  while( tft_get_tear( display->tft)) {
  // while( getTear() ) {
    // usart_printf("tear hi\n" );
  };



  // flip the newly drawn page in
  setScrollStart( display->tft, display->page *  272 );



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

}





