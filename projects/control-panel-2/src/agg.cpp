
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
#include "util.h"



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
  // not even sure if it makes sense to factor this


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


