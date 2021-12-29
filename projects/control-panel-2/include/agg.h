/*

  rename,  this creates the generalized render structure.
    that we pass around

  prehaps use hpp suffix.

  OK. there's a issue
  To have a templated rendering buffer.
  We need c++ linkage.
  hmmmm.
*/



/*
  cannot use external storage with templates.
#ifdef __cplusplus
extern "C" {
#endif
*/


#include "agg_pixfmt_rgb_packed.h"  // rgb565
#include "agg_renderer_base.h"

#include "agg_trans_affine.h"   // for drawText() argument

#include "fsmc.h"       // LCD_WriteData()
#include "ssd1963.h"    // setXY
#include "util.h"    // usart_printf



using namespace agg;



class pixfmt_tft_writer
{

  // eg. from agg_pixfmt_rgb_packed.h
public:

    // used by the renderer_base
    typedef const_row_info<int8u>                     row_data;
    typedef typename agg::blender_rgb565::color_type  color_type;



  private:
    int scroll_start;
  public:

    explicit pixfmt_tft_writer( int scroll_start_ )
      : scroll_start( scroll_start_ )
    {}



    AGG_INLINE unsigned width()  const { return 480;  }
    AGG_INLINE unsigned height() const { return 272; }
    // AGG_INLINE int      stride() const { return m_rbuf->stride(); }

    AGG_INLINE void copy_hline(int x, int y,
                               unsigned len,
                               const color_type& c)
    {

    /*
      // std::cout << "copy_hline       x " << x << " y " << y << " len " << len << " (r " << int(c.r) << " g " << int(c.g) << " b " << int(c.b) << ")"  << std::endl;
    */

      // we could use y += page * height();

      y += scroll_start;
      setXY(x, y, x + len, y + 1);   // y + 1 ????
      for( unsigned i = 0; i < len; ++i ) {
        LCD_WriteData(   packRGB565( c.r, c.g, c.b)  ) ;
      }

    }


  // linking error???

    inline void blend_solid_hspan(int x, int y,
                           unsigned len,
                           const color_type& c,
                           const int8u* covers)
    {
      UNUSED(covers);

      // usart_printf("   blend_hline x=%u y=%u len=%u \n", x, y, len );

      /*
      std::cout << "blend_solid_hspan x " << x << " y " << y << " len " << len << " (r " << int(c.r) << " g " << int(c.g) << " b " << int(c.b) << ")"  ;
      std::cout << " covers ";
      for(unsigned i = 0; i < len; ++i ) {
        std::cout << int(covers[i]) << ", ";
      }
      std::cout << std::endl;
      */
      y += scroll_start;

      setXY(x, y, x + len, y + 1);
      for( unsigned i = 0; i < len; ++i ) {
        LCD_WriteData(   packRGB565( c.r, c.g, c.b)  ) ;
      }


    }

    inline void blend_hline(int x, int y,
                     unsigned len,
                     const color_type& c,
                     int8u cover)
    {
      UNUSED(cover);
      /*
        std::cout << "blend_hline       x " << x << " y " << y << " len " << len << " (r " << int(c.r) << " g " << int(c.g) << " b " << int(c.b) << ")"  << " cover " << int(cover) << std::endl;
      */
      // usart_printf( "x=%u, %y=%u, len=%u, cover=%u\n", x, y, len, cover);

      y += scroll_start;

      setXY(x, y, x + len, y + 1);
      for( unsigned i = 0; i < len; ++i ) {
        LCD_WriteData(   packRGB565( c.r, c.g, c.b)  ) ;
      }

    }

};




#if 0


template<class PixelFormat> class renderer_base_no_clip
{
/*
  renderer_base without a clipbox.

  no clip - supports negative coordinates needed for font  spans.
          - also should be faster, if we know drawing will not extend past bounds. 

  // speed for drawing text not a lot different 9-10ms. to 8-10ms.

*/
  // adapted from agg_renderer_base.h
public:
    typedef PixelFormat pixfmt_type;
    typedef typename pixfmt_type::color_type color_type;
    typedef typename pixfmt_type::row_data row_data;


    explicit renderer_base_no_clip(pixfmt_type& ren) :
        m_ren(&ren)
    {}

  ////////

    pixfmt_type* m_ren;


    void blend_solid_hspan(int x, int y, int len,
                           const color_type& c,
                           const cover_type* covers)
    { 
      // no clip 
      m_ren->blend_solid_hspan(x, y, len, c, covers);
    }


    void blend_hline(int x1, int y, int x2,
                     const color_type& c, cover_type cover)
    { 
      // no clip
      m_ren->blend_hline(x1, y, x2 - x1 + 1, c, cover);
    }


    unsigned width()  const { return m_ren->width();  }
    unsigned height() const { return m_ren->height(); }


    void clear(const color_type& c)
    {
        unsigned y;
        if(width())
        {
            for(y = 0; y < height(); y++)
            {
                m_ren->copy_hline(0, y, width(), c);
            }
        }
    }

    // void copy_bar() etc

};


#endif









// packed rgb565
typedef pixfmt_tft_writer             pixfmt_t;


// how does render_base delegate to the pixel buf???
typedef agg::renderer_base<pixfmt_t>   rb_t ;
// typedef renderer_base_no_clip<pixfmt_t>   rb_t ;



// front fonts.h
struct FontOutline;
struct FontSpans;

void drawOutlineText(rb_t & rb, const FontOutline &font_outline, agg::trans_affine &mtx, const agg::rgba &color, const char *s);


void drawSpanText(rb_t & rb, const FontSpans &font_spans, int x1, int y1, const agg::rgba &color, const char *s);




#if 0


//#include "agg_trans_affine.h"
#include "agg_conv_transform.h"
#include "agg_conv_curve.h"
#include "agg_conv_stroke.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_p.h"
#include "agg_renderer_scanline.h"

// change name drawCurvePath

template< class PathType >
void drawPath(rb_t & rb, PathType & path , agg::trans_affine &mtx, const agg::rgba &color)
{
  // should probably return x,y so can continue text

  agg::rasterizer_scanline_aa<> ras;
  agg::scanline_p8 sl;


  agg::conv_transform<PathType > trans( path, mtx);
  agg::conv_curve<agg::conv_transform<  PathType > > curve(trans);

  ras.reset();
  ras.add_path( curve /*trans*/ );
  // agg::render_scanlines_aa_solid(ras, sl, rb, agg::rgba(0,0,1));
  agg::render_scanlines_aa_solid(ras, sl, rb, color );

}

#endif




/*
  EXTR. IMPORTANT
  - So. we could actually do a memory read of the pixel data. for the line.
  - to do proper blending.
  - in order that we do not have to pass an additional fake background color.
  - this is in fact a real advantage of using a bitmap rather than blt fill operations.
  - most stuff is sold fill though - so reading shouldn't be too costly.
  ------
  alternatively we could do a gamma threshold of the coverage/ for non aa.
*/

  /*
    EXTR.
      need to fill a rect. with agg.
      it may be better to affine rotate the character... maybe on load.
      see rb.copy_bar()

    - need to handle vertical reversed . how is this done normally with agg. with the render buffer?
    - not sure. if we used bottom left. then everything would be nice.
    arduino gfx.
      The coordinate system places the origin (0,0) at the top left corner, with positive X increasing to the right and positive Y increasing downward.
    EXTR.
      i think that always rendering is equivalent to gamma==1.
    EXTR.
      need to invert the char geometry.

  */



/*
#ifdef __cplusplus
}
#endif
*/

