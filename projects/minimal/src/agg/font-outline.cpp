


#include <stdio.h>
#include <assert.h>


// agg stuff
#include <agg_conv_curve.h>
// #include <agg_conv_contour.h>

#include <agg_rasterizer_scanline_aa.h>
#include <agg_scanline_p.h>
#include <agg_renderer_scanline.h>

// for fonts
#include <agg_path_storage.h>




#include <agg/font-outline.h>








void rb_font_outline_write(rb_t & rb, const font_outline_t &font_outline, agg::trans_affine &mtx, const agg::rgba &color, const char *s)
{
  // should probably return x,y so can continue text

  /*
    if font is too large. determined by mtx. then we get crash.
    because of path->m_dx ?
    - not enough memory for the path / curve interpolation.  uses std::allocate. i think

  */
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


