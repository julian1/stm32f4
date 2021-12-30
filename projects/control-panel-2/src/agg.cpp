
#include <stdio.h>
#include <string.h>
#include <stdio.h> // snprintf

#include "agg_conv_curve.h"
#include "agg_conv_contour.h"


#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_p.h"
#include "agg_renderer_scanline.h"

// for fonts
#include "agg_path_storage.h"
#include "agg_path_storage_integer.h"


#include "assert.h"
#include "util.h"

#include "agg.h"
#include "fonts.h"








void drawOutlineText(rb_t & rb, const FontOutline &font_outline, agg::trans_affine &mtx, const agg::rgba &color, const char *s)
{
  // should probably return x,y so can continue text

  agg::rasterizer_scanline_aa<> ras;
  agg::scanline_p8 sl;

  int x  = 0;


  for( const char *p = s; *p; ++p)  {

      char ch = *p;

      font_path_type *path = font_outline.glyph_outline[ ch ];
      assert( path );

      // TODO add a translate() method... or add an adapter
      // should add a method translate( dx, dy );
      path->m_dx = x;

      // advance for next time
      x += font_outline.glyph_advance_x[ ch ];

      agg::conv_transform<font_path_type> trans( *path, mtx);
      agg::conv_curve<agg::conv_transform<  font_path_type > > curve(trans);

      ras.reset();
      ras.add_path( curve );
      // agg::render_scanlines_aa_solid(ras, sl, rb, agg::rgba(0,0,1));
      agg::render_scanlines_aa_solid(ras, sl, rb, color );
  }
}





void drawSpans( rb_t & rb, int dx, int dy,  const agg::rgba &color,  const uint8_t *spans )
{
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
        usart_printf("bad command in drawSpans()\n");
        assert(0);

    }
}




void drawSpanChar(rb_t & rb, const FontSpans &font_spans, int x, int y, const agg::rgba &color, const char ch)
{

  const uint8_t *spans = font_spans.glyph_spans[ ch ];
  assert(spans);

  drawSpans( rb, x, y, color , spans);
  
}



void drawSpanText(rb_t & rb, const FontSpans &font_spans, int x, int y, const agg::rgba &color, const char *s)
{
  
  // should probably return x,y so can continue text

  // int x  = x1;
  // int y  = y1;


  for( const char *p = s; *p; ++p)  {

    char ch = *p;

    const uint8_t *spans = font_spans.glyph_spans[ ch ];
    assert(spans);

    drawSpans( rb, x, y, color , spans);

    x += font_spans.glyph_advance_x[ ch ];
  }
}




