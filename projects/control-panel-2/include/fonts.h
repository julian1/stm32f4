
#include <stdint.h>

#include "agg_path_storage_integer.h"

typedef agg::serialized_integer_path_adaptor<short int, 6>  path_type;


struct FontSpans
{
  uint8_t *glyph_spans[256];
  int glyph_advance_x[256];
  int glphy_advance_y[256];
};



struct FontOutline
{
  path_type *glyph[256];
  int glyph_advance_x[256];
  int glphy_advance_y[256];
};





extern FontSpans arial_span_1p8 ;

extern FontOutline arial_outline;
