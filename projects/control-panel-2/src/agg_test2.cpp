/*
  same as main5.cpp except for packed rgb565
  pixfmt_rgb24 defined like this

  arduing fastish
    "Because CLEAR screen is going to be either black or white. You load the data bus once. Pulse WR line 240x320x2 times."

    OK. the first step here - would be to ignore alpha blending. and just try to write something


  think there's an example here. without aa.

    http://agg.sourceforge.net/antigrain.com/demo/rasterizers.cpp.html
      used to compare speed.

    http://agg.sourceforge.net/antigrain.com/demo/rasterizers2.cpp.html
*/



#include <stdio.h>
#include <string.h>
// #include "agg_pixfmt_rgb24.h"
// #include "agg_pixfmt_rgb.h"
#include "agg_pixfmt_rgb_packed.h"  // rgb565

#include "agg_conv_curve.h"
#include "agg_conv_contour.h"


#include "agg_basics.h"
// #include "agg_rendering_buffer.h"

#include "agg_rendering_buffer.h"

#include "agg_rasterizer_scanline_aa.h"

#include "agg_scanline_p.h"
#include "agg_renderer_scanline.h"


#include "agg_path_storage.h"
// #include "agg_basics.h"

#include "agg_path_storage_integer.h"


// #include <iostream>

#include "agg_test2.h"



#include "assert.h"
#include "fsmc.h"
#include "ssd1963.h"




typedef agg::serialized_integer_path_adaptor<short int, 6>  path_type; 

extern path_type *arial_glyph[256];
extern int arial_glyph_advance_x[256] ;
extern int arial_glyph_advance_y[256] ;



using namespace agg;


    // eg. from agg_pixfmt_rgb_packed.h

    //===========================================pixfmt_alpha_blend_rgb_packed
    template<class Blender,  class RenBuf> class pixfmt_alpha_blend_rgb_packed
    {
    public:
        typedef RenBuf   rbuf_type;
        typedef typename rbuf_type::row_data row_data;
        typedef Blender  blender_type;
        typedef typename blender_type::color_type color_type;
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




public:
      /*
        EXTR. IMPORTANT
        - So. we could actually do a memory read of the pixel data. for the line.
        - to do proper blending.
        - in order that we do not have to pass an additional fake background color.
        - this is in fact a real advantage of using a bitmap without fill operations.
        ------
        alternatively we could do a gamma threshold of the coverage/ for non aa.
      */

        /*
          EXTR.
            need to fill a rect. with agg.
            it may be better to affine rotate the character... maybe on load.

          - need to handle vertical reversed . how is this done normally with agg. with the render buffer?
          - not sure. if we used bottom left. then everything would be nice.
          arduino gfx.
            The coordinate system places the origin (0,0) at the top left corner, with positive X increasing to the right and positive Y increasing downward.
          EXTR.
            i think that always rendering is equivalent to gamma==1.
          EXTR.
            need to invert the char geometry.

        */

        explicit pixfmt_alpha_blend_rgb_packed(/* rbuf_type& rb */) //:
            // m_rbuf(&rb)
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
          setXY(x, y, x + len, y + 1);   // y + 1 ????
          for( int i = 0; i < len; ++i ) {
            LCD_WriteData(   packRGB565( c.r, c.g, c.b)  ) ;
          }

        }



        void blend_solid_hspan(int x, int y,
                               unsigned len,
                               const color_type& c,
                               const int8u* covers)
        {
          /*
          std::cout << "blend_solid_hspan x " << x << " y " << y << " len " << len << " (r " << int(c.r) << " g " << int(c.g) << " b " << int(c.b) << ")"  ;
          std::cout << " covers ";
          for(unsigned i = 0; i < len; ++i ) {
            std::cout << int(covers[i]) << ", ";
          }
          std::cout << std::endl;
          */

          setXY(x, y, x + len, y + 1);   // how is this working without y + 1 ????
          for( int i = 0; i < len; ++i ) {
            LCD_WriteData(   packRGB565( c.r, c.g, c.b)  ) ;
          }


        }

        void blend_hline(int x, int y,
                         unsigned len,
                         const color_type& c,
                         int8u cover)
        {
        /*
          std::cout << "blend_hline       x " << x << " y " << y << " len " << len << " (r " << int(c.r) << " g " << int(c.g) << " b " << int(c.b) << ")"  << " cover " << int(cover) << std::endl;
        */
          setXY(x, y, x + len, y + 1);   // how is this working without y + 1 ????
          for( int i = 0; i < len; ++i ) {
            LCD_WriteData(   packRGB565( c.r, c.g, c.b)  ) ;
          }

        }

    };






/*
  if rgb565 doesn't work.  ssd1963 can do 24 bit color. have conversions...
  should try to compile this and see if can implement

  actually how the fill operations workk - separate from the bitmap blting.
  possible exactly the same.
  --------------

  hmmmm...
  it supports bliting bitmap.   but has no actual fill function?.

  ssd1936 (not ssd1963)  has filled rectangle draw.
  ook.  but then how does Andy do it?   check code.

  well we could still test it. I suppose.
  -----------

  on an arduino - one just asserts the color on the bus. and then pulse the wr pin. which is pretty fast. maybe faster than bus mode.
    https://www.avrfreaks.net/forum/slow-working-lcd-ssd1963-controller-board
  ---------

  EXTR. it might

  ------------
  stm32 100MHz / 5 (bus data setup ) = 20MHz.
  480 * 272 * 2 =  261120 = 260000.
  So write speed should be 77fps.
  ------------


  EXTR. should just do all drawing with transforms... and using native agg primitives.
      - avoid a sizing/layout engine.
      - if have different screens then just draw them independently.
      -------
      eg. document html/ ps type model.

      - inject a struct with needed data - into the view render code.
      - use local switch - to flip tabs etc.
      - use update flags - indicating if have to rerender

  ------------

*/

int agg_test2()
{

    // packed rgb565
    typedef ::pixfmt_alpha_blend_rgb_packed<agg::blender_rgb565, agg::rendering_buffer> pixfmt_t;


    pixfmt_t  pixf;


     agg::renderer_base<pixfmt_t>   rb(pixf);

     rb.clear(agg::rgba(1,1,1));     // white .


    // http://agg.sourceforge.net/antigrain.com/demo/conv_contour.cpp.html
    // https://coconut2015.github.io/agg-tutorial/agg__trans__affine_8h_source.html
    agg::trans_affine mtx;
    // mtx.flip_y();
    // mtx *= agg::trans_affine_scaling(1.0, -1); // this inverts/flips the glyph, relative to origin. but not in place.
    mtx *= agg::trans_affine_translation(50, 50);   // this moves from above origin, back into the screen.
    mtx *= agg::trans_affine_scaling(3.0); // now scale it

    // we need other letters 'j' with hanging to

    // EXTR. IMPORTANT confirm we have floating point enabled.
    
    // const char *s = "hello";

    agg::path_storage m_path;


    // how do we do the advance
    // we can render all in one go..

    // OK. so each glyph kind of wants to be locally transformed first
    // ok. this


    int x  = 0;
    for( char *p = "hello"; *p; ++p)  {

      // whoot it links
      path_type *path = arial_glyph[ *p ];
      assert( path ); 

      // no there is a local translate on the serialization structure...
      // path->translate( 10, 10);


      // ok. this sets the structure before we copy the path... 
      path->m_dx = x;
      
      m_path.join_path( *path ); 

      x += arial_glyph_advance_x[ *p ];

    } 

    typedef agg::path_storage path_type2;

      agg::conv_transform<path_type2> trans(m_path, mtx);
      agg::conv_curve<agg::conv_transform<  path_type2 > > curve(trans);

    /*
      OK. it's possible we may want the origin at bottom left.  if it makes handling fonts easier.
      we can probably flip o
      what does postscript do.
      Note that we can flip the origin on the fly.
    */

    agg::rasterizer_scanline_aa<> ras;
    agg::scanline_p8 sl;

    ras.add_path( curve );
    agg::render_scanlines_aa_solid(ras, sl, rb, agg::rgba(0,0,1));

    // non anti aliased.
    // agg::renderer_scanline_bin_solid(


    // setOriginBottomLeft(); // cartesion/ fonts/ postscript

    // agg::path_storage            m_path2;
#if 0
      m_path.remove_all();
      m_path.move_to(10, 10);
      m_path.line_to(50, 10);
      m_path.line_to(50, 50);
      m_path.line_to(10, 50);
      m_path.line_to(10, 10);
      m_path.close_polygon();//agg::path_flags_cw);

    ras.reset();
    ras.add_path( m_path );
    agg::render_scanlines_aa_solid(ras, sl, /*renb*/ rb, agg::rgba(0,1,0));

#endif

    return 0;
}

