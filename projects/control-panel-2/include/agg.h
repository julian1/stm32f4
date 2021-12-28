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



template<class Blender,  class RenBuf> class pixfmt_alpha_blend_rgb_packed
{

  /*
  // TODO 
  // remove the template arguments that we don't use at all. 
  // and fix/set the row_data and color_type explicitly.
  */

  // eg. from agg_pixfmt_rgb_packed.h
public:

    // for consumers
    typedef RenBuf   rbuf_type;
    typedef typename rbuf_type::row_data row_data;

    typedef Blender  blender_type;
    typedef typename blender_type::color_type color_type;
/*
    typedef typename blender_type::pixel_type pixel_type;
    typedef int                               order_type; // A fake one
    typedef typename color_type::value_type   value_type;
    typedef typename color_type::calc_type    calc_type;
    enum base_scale_e
    {
        base_shift = color_type::base_shift,
        base_scale = color_type::base_scale,
        base_mask  = color_type::base_mask,
        pix_width  = sizeof(pixel_type),
    };

*/

private:
    int scroll_start;
public:

    explicit pixfmt_alpha_blend_rgb_packed( int scroll_start_ )
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


// packed rgb565
typedef ::pixfmt_alpha_blend_rgb_packed<agg::blender_rgb565, agg::rendering_buffer> pixfmt_t;


// how does render_base delegate to the pixel buf???
typedef agg::renderer_base<pixfmt_t>   rb_t ;




void drawText(rb_t & rb, agg::trans_affine &mtx, const agg::rgba &color, const char *s);




void drawSpans( rb_t & rb,  const agg::rgba &color,  const uint8_t *spans ) ;



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

