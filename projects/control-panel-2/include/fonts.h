
#include <stdint.h>

#include "agg_path_storage_integer.h"

typedef agg::serialized_integer_path_adaptor<short int, 6>  font_path_type;


struct FontOutline
{
  font_path_type *glyph_outline[256];
  int glyph_advance_x[256];
  int glphy_advance_y[256];
};


struct FontSpans
{
  uint8_t *glyph_spans[256];
  int glyph_advance_x[256];
  int glphy_advance_y[256];
};





extern FontOutline arial_outline;


extern FontSpans arial_span_72;
extern FontSpans arial_span_18;


