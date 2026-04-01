
/*

*/


#include <string.h>     // memcpy
#include <assert.h>

#include <lib2/util.h>  // ARRAY_SIZE

// #include <agg/font-outline.h>
#include <agg/font-span.h>
#include <agg/tft-display.h>


#include <support.h>         // format
#include <data/data.h>
#include <data/range.h>
#include <data/buffer.h>



/*
  if the page flip is moved from data_update() to update()
  then timing code, can be hoisted up and placed around the _update()  call.

*/






static void tft_display_update_data_( tft_display_t *display, data_t *data)
{
  assert( display && display->magic == TFT_DISPLAY_MAGIC);
  assert( data && data->magic == DATA_MAGIC);


  buffer_t *buffer = display->buffer;
  assert( buffer && buffer->magic == BUFFER_MAGIC);



  // ignore, if current rendered page has not been flipped to display yet
  if( display->page_ready)
    return;



  if( !data->valid) {

    // clear the star to indicate no measurement data
    char buf[101];
    snprintf( buf, 100, "%c", ' ');
    // vfd_font_small_write( vfd, buf, 0, 3 );
    // vfd_font_small_write( vfd, buf, 0, 7 );

    return;
  }


  // range only available if data valid
  const range_t *range = data->range;
  assert( range && range->magic == RANGE_MAGIC);


  // change the page
  display->page = ! display->page;

  // set up our buffer
  pixfmt_t  pixf( display->tft, display->page *  272 );
  rb_t    rb(pixf);

  rb.clear(agg::rgba( 1,1,1));     // white .

  uint32_t start = *display->system_millis;

  ///////////////

  format_val_t  val;
  range->range_reading_format( range, &val, 9, data->reading);

  rb_font_span_write_special(rb,  arial_span_72, 10, 100 , agg::rgba(0,0,1), val.all );

  ///////

  char buf[ 101];
  char buf2[ 101];

  snprintf( buf, 100, "stddev %sV", str_format_value_dynamic( buf2, 100, buffer->stddev, 4 ));

  rb_font_span_write(rb,  arial_span_18,    10, 150 , agg::rgba(0,0,1), buf );


  display->page_ready = true;
}









void tft_display_update( tft_display_t *display)
{
  assert( display && display->magic == TFT_DISPLAY_MAGIC);

#if 0
  tft_test6( display);
#endif

#if 1

  if( !display->page_ready)
    return;

  /*
  // OK. this fixes flicker... but opposite of what we thought...
  // we flip the page when it is drawing. rather than when it's blanking.
  IMPORTANT.
    this is a blocking function and can block all display function
  */

  // non blocking
  // if( tft_get_tear( display->tft))
  //  return ;

  // flip the newly drawn page in
  setScrollStart( display->tft, display->page *  272 );


  display->page_ready = false;


  // flip the display page
  display->page = ! display->page;


#endif
}


void tft_display_update_500ms( tft_display_t *display)
{
  assert( display && display->magic == TFT_DISPLAY_MAGIC);

}


void tft_display_update_data( tft_display_t *display, data_t *data)
{
  assert( display && display->magic == TFT_DISPLAY_MAGIC);

  if( display->update_data)
    display->update_data( display, data);
}




bool tft_display_repl_statement( tft_display_t *display,  const char *cmd)
{
  assert( display && display->magic == TFT_DISPLAY_MAGIC);

  if(strcmp(cmd, "tft_test2") == 0) {
    display->update_data = NULL;
    // display->update = tft_test2;
  }

  else if(strcmp(cmd, "tft_test3") == 0) {
    display->update_data = NULL;
    // display->update = tft_test3;
  }

  else if(strcmp(cmd, "tft_none") == 0)
    display->update_data = NULL;


  else
    return 0;

  return 1;

}


void tft_display_init( tft_display_t *display,  tft_t *tft, buffer_t *buffer, volatile uint32_t *system_millis)
{

  memset( display, 0, sizeof( tft_display_t));

  display->magic = TFT_DISPLAY_MAGIC;
  display->tft = tft;
  display->buffer = buffer;

  display->system_millis = system_millis;

  // display->update = display_update_;    // set to non

  display->update_data =  tft_display_update_data_;

}

/*
  // check if font has a space char...
  // missing the '7' char/ aparently.

  static int count = 0;
  char buf[100];
  snprintf(buf, 100, "whoot %u", count++);
*/


#if 0
static void tft_display_update_data_( tft_display_t *display, data_t *data)
{
  assert( display && display->magic == TFT_DISPLAY_MAGIC);
  assert( data && data->magic == DATA_MAGIC);


  // do nothing - if still waiting to display existin page.
  if( display->page_ready)
    return;


  volatile uint32_t *system_millis = display->system_millis;



  // set up our buffer
  pixfmt_t  pixf( display->tft, display->page *  272);
  agg::renderer_base<pixfmt_t>   rb(pixf);

  // EXTR. if looks like if draw too fast. then flipping the scrollstart can introduce flicker
  uint32_t start = *system_millis;

  // consider - if the last rendered page has not updated, then skip
  // if( display->new_page)
  //  return;


  // no valid measurement reading
  if( !data->valid) {

    // clear the measurement star

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
  range->range_reading_format( range, &val, 9, data->reading);



  rb_font_outline_write(rb, arial_outline, mtx, agg::rgba(0,0,1), val.all );


  display->page_ready = true;

}
#endif





