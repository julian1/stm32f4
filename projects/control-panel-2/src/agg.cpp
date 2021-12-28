
#include <stdio.h>
#include <string.h>
#include <stdio.h> // snprintf
// #include "agg_pixfmt_rgb24.h"
// #include "agg_pixfmt_rgb.h"

#include "agg_conv_curve.h"
#include "agg_conv_contour.h"


// #include "agg_basics.h"
// #include "agg_rendering_buffer.h"

#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_p.h"
#include "agg_renderer_scanline.h"

// for fonts
#include "agg_path_storage.h"
#include "agg_path_storage_integer.h"



#include "agg.h"

#include "assert.h"
// #include "util.h"



/*
  TODO
  prototypes should be put in arial.h
  And then also pass the glyph structures.
*/

typedef agg::serialized_integer_path_adaptor<short int, 6>  font_path_type;

extern font_path_type *arial_glyph[256];
extern int arial_glyph_advance_x[256] ;
extern int arial_glyph_advance_y[256] ;






void drawText(rb_t & rb, agg::trans_affine &mtx, const agg::rgba &color, const char *s)
{
  // TODO change namem drawOutlineText...
  // not even sure if it makes sense to factor this
  // rb.copy_hline(0, 0, 0, color );


  // we can move these out of loop if desired...
  agg::rasterizer_scanline_aa<> ras;
  agg::scanline_p8 sl;


  int x  = 0;

 // uint32_t start = system_millis;

  for( const char *p = s; *p; ++p)  {

      char ch = *p;

      // whoot it links
      font_path_type *path = arial_glyph[ ch ];
/*
      if(!path) {
        x += 10;
        continue;
      }
*/
      assert( path );

      // TODO add a translate() method... or add an adapter
      // should add a method translate( dx, dy );
      path->m_dx = x;

      x += arial_glyph_advance_x[ ch ];
      // y += arial_glyph_advance_y[ ch ];  ie. if lang uses vertical glyphs


      agg::conv_transform<font_path_type> trans( *path, mtx);
      agg::conv_curve<agg::conv_transform<  font_path_type > > curve(trans);

      ras.reset();
      ras.add_path( curve );
      // agg::render_scanlines_aa_solid(ras, sl, rb, agg::rgba(0,0,1));
      agg::render_scanlines_aa_solid(ras, sl, rb, color );

  }

  // usart_printf("agg text draw %ums\n", system_millis - start );
  // nothing is displayed????

}


/*
  - OK. we need to support an x,y translation offset here...
  to be able to draw text.

  - don't use mtx. because spans cannot be affine.

*/


void drawSpans( rb_t & rb, int dx, int dy,  const agg::rgba &color,  const uint8_t *spans )
{
  // TODO change name drawSpanText
  // actually no, since it will work on any span data.
  // dx,dy
  // rb.copy_hline(0, 0, 0, color );

  const uint8_t *p = spans;

  bool done = false;

  while(!done)
    switch(*p) {

      case (0x01 << 7):   {
        // usart_printf("got blend_solid_hspan\n");

        // void blend_solid_hspan(int x, int y, unsigned len, const color_type& c, const int8u* covers)
        p++;
        int x = (int8_t) *p++;
        int y = (int8_t) *p++;     // y at least needs to be interpreted as signed.
        int len = *p++;
        const uint8_t *covers = p;    // no array copying nice/fast versus other solutions
        p += len;

        x += dx;
        y += dy;

        rb.blend_solid_hspan(x, y, len, color, covers);
        break;
      }

      case (0x01 << 6):   {
        // usart_printf("got blend_hline\n");
        // void blend_hline(int x, int y, unsigned len, const color_type& c, int8u cover)
        p++;
        int x = (int8_t)*p++;
        int y = (int8_t) *p++;     // y at least needs to be interpreted as signed.
        int len = *p++;
        const uint8_t cover = *p++;    // no array copying nice/fast versus other solutions

        x += dx;
        y += dy;

        // careful  .blend_hline() interface for x is different for rendering_base versus pxfmt
        rb.blend_hline( x, y, x + len, color, cover ); // x1, x2
        break;
      }

      case (0x01 << 5): { 
        // sentinel, we are done.
        done = true;
        break;
      }

      default:
        usart_printf("unknwon char in drawSpans()\n");
        assert(0);

    }
}



// TODO fixme
// arial-span-1.8
extern uint8_t *glyph[256] ;
extern int glyph_advance_x[256] ;


void drawSpanText(rb_t & rb, int x1, int y1, const agg::rgba &color, const char *s)
{
  int x  = x1;
  int y  = y1;

 // uint32_t start = system_millis;

  for( const char *p = s; *p; ++p)  {

      char ch = *p;


      uint8_t *spans = glyph[ ch ];
      assert(spans);

      // usart_printf("before drawSpans() \n");
      drawSpans( rb, x, y,  agg::rgba(0,0,1),  spans );

      x += glyph_advance_x[ ch ];

  }


}




