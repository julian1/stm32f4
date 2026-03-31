
/*

*/


#include <string.h>     // memcpy
#include <assert.h>


#include <agg/agg.h>
#include <agg/fonts.h>
#include <agg/tft-display.h>


#include <data/data.h>
#include <data/range.h>
#include <data/buffer.h>





static void tft_display_update_data_( tft_display_t *tft_display, data_t *data)
{
  assert( tft_display && tft_display->magic == TFT_DISPLAY_MAGIC);
  assert( data && data->magic == DATA_MAGIC);


  volatile uint32_t *system_millis = tft_display->system_millis;

  // persist the page that we need to draw
  // static int page = 0; // page to use
  tft_display->page = ! tft_display->page;


  // set up our buffer
  pixfmt_t  pixf( tft_display->tft, tft_display->page *  272);
  agg::renderer_base<pixfmt_t>   rb(pixf);

// EXTR. if looks like if draw too fast. then flipping the scrollstart can introduce flicker
  uint32_t start = *system_millis;

  // consider - if the last rendered page has not updated, then skip
  // if( tft_display->new_page)
  //  return;


  // no valid measurement reading
  if( !data->valid) {

    return;
  }




  rb.clear(agg::rgba(1,1,1));     // white .
  // rb.clear(agg::rgba(1,1,1));     // white .
  // usart_printf("rb.clear() %ums\n", system_millis - start );

  // EXTR. this is a clear/fillRect that is not subpixel, and simple.
  // see, agg_renderer_base.h.
  rb.copy_bar(20, 20, 100, 200, agg::rgba(1,0,0));



  // http://agg.sourceforge.net/antigrain.com/demo/conv_contour.cpp.html
  // https://coconut2015.github.io/agg-tutorial/agg__trans__affine_8h_source.html
  agg::trans_affine mtx;
  // mtx.flip_y();
  // mtx *= agg::trans_affine_scaling(1.0, -1); // this inverts/flips the glyph, relative to origin. but not in place.
  mtx *= agg::trans_affine_translation(50, 50);   // this moves from above origin, back into the screen.
  mtx *= agg::trans_affine_rotation(10.0 * 3.1415926 / 180.0);
  mtx *= agg::trans_affine_scaling(1.0); // now scale it



  // only have range if data is valid
  const range_t *range = data->range;
  assert( range && range->magic == RANGE_MAGIC);


  /*
    extr. we could do the formatting once, at the time of decode.
  */


  format_val_t  val;
  range->range_reading_format( range, &val, 8, data->reading);



  drawOutlineText(rb, arial_outline, mtx, agg::rgba(0,0,1), val.all );



  /*
    Extr.  move this page flip code into the normal update() call
    to avoid blocking here.

  */

  /*
  // OK. this fixes flicker... but opposite of what we thought...
  // we flip the page when it is drawing. rather than when it's blanking.
  IMPORTANT.
    this is a blocking function and can block all display function
  */
  while( tft_get_tear( tft_display->tft)) {
  // while( getTear() ) {
    // usart_printf("tear hi\n" );
  };

  // flip the newly drawn page in
  setScrollStart( tft_display->tft, tft_display->page *  272 );

}









void tft_display_update( tft_display_t *tft_display)
{
  assert( tft_display && tft_display->magic == TFT_DISPLAY_MAGIC);


  /*
    Extr.  move the tear test, and page flip code
    to avoid blocking, and so that control is yielded.

    if( new_page && tear ) {
        new_page = false;

        setScrollStart( tft_display->tft, tft_display->page *  272 );
    }

    if( page != new_page && ! tft_get_tear())
      // then flip and set page == new_page

  */


}


void tft_display_update_500ms( tft_display_t *tft_display)
{
  assert( tft_display && tft_display->magic == TFT_DISPLAY_MAGIC);

}


void tft_display_update_data( tft_display_t *tft_display, data_t *data)
{
  assert( tft_display && tft_display->magic == TFT_DISPLAY_MAGIC);

  if( tft_display->update_data)
    tft_display->update_data( tft_display, data);
}




bool tft_display_repl_statement( tft_display_t *tft_display,  const char *cmd)
{
  assert( tft_display && tft_display->magic == TFT_DISPLAY_MAGIC);

  if(strcmp(cmd, "tft_test2") == 0) {
    tft_display->update_data = NULL;
    // tft_display->update = tft_test2;
  }

  else if(strcmp(cmd, "tft_test3") == 0) {
    tft_display->update_data = NULL;
    // tft_display->update = tft_test3;
  }

  else if(strcmp(cmd, "tft_none") == 0)
    tft_display->update_data = NULL;


  else
    return 0;

  return 1;

}


void tft_display_init( tft_display_t *tft_display, tft_t *tft, volatile uint32_t *system_millis)
{

  memset( tft_display, 0, sizeof( tft_display_t));

  tft_display->magic = TFT_DISPLAY_MAGIC;
  tft_display->tft = tft;
  tft_display->system_millis = system_millis;

  // tft_display->update = tft_display_update_;    // set to non

  tft_display->update_data =  tft_display_update_data_;

}

/*
  // check if font has a space char...
  // missing the '7' char/ aparently.

  static int count = 0;
  char buf[100];
  snprintf(buf, 100, "whoot %u", count++);
*/

