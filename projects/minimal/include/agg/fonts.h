
#ifndef FONTS_H
#define FONTS_H
  /*
    unreferenced 72 point arial,

       text    data     bss     dec     hex filename
     109032   29736   19956  158724   26c04 main.elf

    referenced in code 

       text    data     bss     dec     hex filename
     112136   58272   19956  190364   2e79c main.elf


    after compiling and making const and static.
       text    data     bss     dec     hex filename
     140664   29736   19956  190356   2e794 main.elf


    ok. so 29k to 58k data.
    and larger text.
  */



#include <stdint.h>

#include "agg_path_storage_integer.h"

typedef agg::serialized_integer_path_adaptor<short int, 6>  font_path_type;


struct FontOutline
{
  font_path_type *glyph_outline[256]; // cannot be const. because c++ constructed object
  const int glyph_advance_x[256];
  const int glphy_advance_y[256];
};


struct FontSpans
{
  const uint8_t *glyph_spans[256];
  const int glyph_advance_x[256];
  const int glphy_advance_y[256];
};


// problem with c or cpp linkage?


extern const FontOutline arial_outline;


extern const FontSpans arial_span_72;
extern const FontSpans arial_span_18;


#endif // FONTS_H
