

#pragma once


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



#include <agg_path_storage_integer.h>

typedef agg::serialized_integer_path_adaptor<short int, 6>  font_path_type;


struct font_outline_t
{
  font_path_type *glyph_outline[256]; // cannot be const. because c++ constructed object
  const int glyph_advance_x[256];
  const int glphy_advance_y[256];
};



extern const font_outline_t arial_outline;



#include <agg/pixfmt-tft.h>
#include <agg_renderer_base.h>
#include <agg_trans_affine.h>



typedef pixfmt_tft_writer               pixfmt_t;

typedef agg::renderer_base< pixfmt_t>   rb_t ;


// use function template on first argument type, for polymorphic render base.

void rb_font_outline_write( rb_t & rb, const font_outline_t &font_outline, agg::trans_affine &mtx, const agg::rgba &color, const char *s);

// vfd_font_small_write_special()



