

#include <stdio.h>
#include <assert.h>
#include <ctype.h>



#include <agg/font-span.h>







void drawSpans( rb_t & rb, int dx, int dy,  const agg::rgba &color,  const uint8_t *spans )
{
  const uint8_t *p = spans;
  bool done = false;

  while(!done)
    switch(*p) {

      case (0x01 << 7):   {
        // printf("got blend_solid_hspan\n");

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
        // printf("got blend_hline\n");
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
        printf("bad command in drawSpans()\n");
        assert(0);

    }
}




void rb_font_span_write_char(rb_t & rb, const font_span_t &font_spans, int x, int y, const agg::rgba &color, const char ch)
{

  const uint8_t *spans = font_spans.glyph_spans[ ch ];
  assert(spans);

  drawSpans( rb, x, y, color , spans);

}



void rb_font_span_write(rb_t & rb, const font_span_t &font_spans, int x, int y, const agg::rgba &color, const char *s)
{
  // proportionaly spaced

  for( const char *p = s; *p; ++p)  {

    char ch = *p;

    const uint8_t *spans = font_spans.glyph_spans[ ch ];
    assert(spans);

    drawSpans( rb, x, y, color , spans);

    x += font_spans.glyph_advance_x[ ch ];
  }
}


void rb_font_span_write_special(rb_t & rb, const font_span_t &font_spans, int x, int y, const agg::rgba &color, const char *s)
{
  // proportionaly spaced, but monospace digits

  for( const char *p = s; *p; ++p)  {

    char ch = *p;

    const uint8_t *spans = font_spans.glyph_spans[ ch ];
    assert(spans);

    drawSpans( rb, x, y, color , spans);


    if( isdigit( (unsigned char) ch)
      || ch == '+' || ch == '-')
    {
      // mono-space advance. reference'0'
      x += font_spans.glyph_advance_x[ '0' ];
    }
    else {
      // proportional advance for everything else
      x += font_spans.glyph_advance_x[ ch ];
    }


  }
}




