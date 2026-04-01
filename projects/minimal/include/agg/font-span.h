

#pragma once



#include <stdint.h>


// rename font_span_t

struct font_span_t
{
  const uint8_t *glyph_spans[256];
  const int glyph_advance_x[256];
  const int glphy_advance_y[256];
};


extern const font_span_t arial_span_72;
extern const font_span_t arial_span_18;




#include <agg/pixfmt-tft.h>
#include <agg_renderer_base.h>
#include <agg_trans_affine.h>



/*
  apr. 2026. review. is the clip bounding box established here.setup.
  can improve render speed. by fast clipping geometry outside render base
*/

// packed rgb565
typedef pixfmt_tft_writer             pixfmt_t;

typedef agg::renderer_base< pixfmt_t>   rb_t ;



// use function template on first argument type, for polymorphic render base.

void rb_font_span_write_char( rb_t & rb, const font_span_t &font_spans, int x1, int y1, const agg::rgba &color, const char ch);

void rb_font_span_write( rb_t & rb, const font_span_t &font_spans, int x1, int y1, const agg::rgba &color, const char *s);

// really not sure if should not just pass a flag
void rb_font_span_write_special( rb_t & rb, const font_span_t &font_spans, int x1, int y1, const agg::rgba &color, const char *s);


