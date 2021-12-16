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


#include "agg_basics.h"
// #include "agg_rendering_buffer.h"

#include "agg_rendering_buffer.h"

#include "agg_rasterizer_scanline_aa.h"

#include "agg_scanline_p.h"
#include "agg_renderer_scanline.h"


#include "agg_path_storage.h"
// #include "agg_basics.h"


// #include <iostream>

#include "agg_test2.h"



#include "fsmc.h"
#include "ssd1963.h"








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

        explicit pixfmt_alpha_blend_rgb_packed(/* rbuf_type& rb */) //:
            // m_rbuf(&rb)
        {}


        AGG_INLINE unsigned width()  const { return 100;  }
        AGG_INLINE unsigned height() const { return 100; }
        // AGG_INLINE int      stride() const { return m_rbuf->stride(); }

        AGG_INLINE void copy_hline(int x, int y,
                                   unsigned len,
                                   const color_type& c)
        {
/*
          // std::cout << "copy_hline       x " << x << " y " << y << " len " << len << " (r " << int(c.r) << " g " << int(c.g) << " b " << int(c.b) << ")"  << std::endl;
*/
          setXY(x, y, x + len, y );   // y + 1 ????
          for( int i = 0; i < len; ++i ) {
           // rgb 565
            LCD_WriteData(    (c.r & 0x1f ) << 11 | (c.g & 0x3f) << 5 | (c.b & 0x1f)  ) ; 
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

          setXY(x, y, x + len, y );   // y + 1 ????
          for( int i = 0; i < len; ++i ) {
           // rgb 565
            LCD_WriteData(    (c.r & 0x1f ) << 11 | (c.g & 0x3f) << 5 | (c.b & 0x1f)  ) ; 
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
        }



#if 0
    private:
        //--------------------------------------------------------------------
        AGG_INLINE void copy_or_blend_pix(pixel_type* p, const color_type& c, unsigned cover)
        {
            if (c.a)
            {
                calc_type alpha = (calc_type(c.a) * (cover + 1)) >> 8;
                if(alpha == base_mask)
                {
                    *p = m_blender.make_pix(c.r, c.g, c.b);
                }
                else
                {
                    m_blender.blend_pix(p, c.r, c.g, c.b, alpha, cover);
                }
            }
        }

    public:
        //--------------------------------------------------------------------
        explicit pixfmt_alpha_blend_rgb_packed(rbuf_type& rb) : m_rbuf(&rb) {}
        void attach(rbuf_type& rb) { m_rbuf = &rb; }

        //--------------------------------------------------------------------
        template<class PixFmt>
        bool attach(PixFmt& pixf, int x1, int y1, int x2, int y2)
        {
            rect_i r(x1, y1, x2, y2);
            if(r.clip(rect_i(0, 0, pixf.width()-1, pixf.height()-1)))
            {
                int stride = pixf.stride();
                m_rbuf->attach(pixf.pix_ptr(r.x1, stride < 0 ? r.y2 : r.y1), 
                               (r.x2 - r.x1) + 1,
                               (r.y2 - r.y1) + 1,
                               stride);
                return true;
            }
            return false;
        }

        Blender& blender() { return m_blender; }

        //--------------------------------------------------------------------
        AGG_INLINE unsigned width()  const { return m_rbuf->width();  }
        AGG_INLINE unsigned height() const { return m_rbuf->height(); }
        AGG_INLINE int      stride() const { return m_rbuf->stride(); }

        //--------------------------------------------------------------------
        AGG_INLINE       int8u* row_ptr(int y)       { return m_rbuf->row_ptr(y); }
        AGG_INLINE const int8u* row_ptr(int y) const { return m_rbuf->row_ptr(y); }
        AGG_INLINE row_data     row(int y)     const { return m_rbuf->row(y); }

        //--------------------------------------------------------------------
        AGG_INLINE int8u* pix_ptr(int x, int y)
        {
            return m_rbuf->row_ptr(y) + x * pix_width;
        }

        AGG_INLINE const int8u* pix_ptr(int x, int y) const
        {
            return m_rbuf->row_ptr(y) + x * pix_width;
        }

        //--------------------------------------------------------------------
        AGG_INLINE void make_pix(int8u* p, const color_type& c)
        {
            *(pixel_type*)p = m_blender.make_pix(c.r, c.g, c.b);
        }

        //--------------------------------------------------------------------
        AGG_INLINE color_type pixel(int x, int y) const
        {
            return m_blender.make_color(((pixel_type*)m_rbuf->row_ptr(y))[x]);
        }

        //--------------------------------------------------------------------
        AGG_INLINE void copy_pixel(int x, int y, const color_type& c)
        {
            ((pixel_type*)
                m_rbuf->row_ptr(x, y, 1))[x] = 
                    m_blender.make_pix(c.r, c.g, c.b);
        }

        //--------------------------------------------------------------------
        AGG_INLINE void blend_pixel(int x, int y, const color_type& c, int8u cover)
        {
            copy_or_blend_pix((pixel_type*)m_rbuf->row_ptr(x, y, 1) + x, c, cover);
        }

        //--------------------------------------------------------------------
        AGG_INLINE void copy_hline(int x, int y, 
                                   unsigned len, 
                                   const color_type& c)
        {
            pixel_type* p = (pixel_type*)m_rbuf->row_ptr(x, y, len) + x;
            pixel_type v = m_blender.make_pix(c.r, c.g, c.b);
            do
            {
                *p++ = v;
            }
            while(--len);
        }

        //--------------------------------------------------------------------
        AGG_INLINE void copy_vline(int x, int y,
                                   unsigned len, 
                                   const color_type& c)
        {
            pixel_type v = m_blender.make_pix(c.r, c.g, c.b);
            do
            {
                pixel_type* p = (pixel_type*)m_rbuf->row_ptr(x, y++, 1) + x;
                *p = v;
            }
            while(--len);
        }

        //--------------------------------------------------------------------
        void blend_hline(int x, int y,
                         unsigned len, 
                         const color_type& c,
                         int8u cover)
        {
            if (c.a)
            {
                pixel_type* p = (pixel_type*)m_rbuf->row_ptr(x, y, len) + x;
                calc_type alpha = (calc_type(c.a) * (cover + 1)) >> 8;
                if(alpha == base_mask)
                {
                    pixel_type v = m_blender.make_pix(c.r, c.g, c.b);
                    do
                    {
                        *p++ = v;
                    }
                    while(--len);
                }
                else
                {
                    do
                    {
                        m_blender.blend_pix(p, c.r, c.g, c.b, alpha, cover);
                        ++p;
                    }
                    while(--len);
                }
            }
        }

        //--------------------------------------------------------------------
        void blend_vline(int x, int y,
                         unsigned len, 
                         const color_type& c,
                         int8u cover)
        {
            if (c.a)
            {
                calc_type alpha = (calc_type(c.a) * (cover + 1)) >> 8;
                if(alpha == base_mask)
                {
                    pixel_type v = m_blender.make_pix(c.r, c.g, c.b);
                    do
                    {
                        ((pixel_type*)m_rbuf->row_ptr(x, y++, 1))[x] = v;
                    }
                    while(--len);
                }
                else
                {
                    do
                    {
                        m_blender.blend_pix(
                            (pixel_type*)m_rbuf->row_ptr(x, y++, 1), 
                            c.r, c.g, c.b, alpha, cover);
                    }
                    while(--len);
                }
            }
        }

        //--------------------------------------------------------------------
        void blend_solid_hspan(int x, int y,
                               unsigned len, 
                               const color_type& c,
                               const int8u* covers)
        {
            pixel_type* p = (pixel_type*)m_rbuf->row_ptr(x, y, len) + x;
            do 
            {
                copy_or_blend_pix(p, c, *covers++);
                ++p;
            }
            while(--len);
        }

        //--------------------------------------------------------------------
        void blend_solid_vspan(int x, int y,
                               unsigned len, 
                               const color_type& c,
                               const int8u* covers)
        {
            do 
            {
                copy_or_blend_pix((pixel_type*)m_rbuf->row_ptr(x, y++, 1) + x, 
                                  c, *covers++);
            }
            while(--len);
        }

        //--------------------------------------------------------------------
        void copy_color_hspan(int x, int y,
                              unsigned len, 
                              const color_type* colors)
        {
            pixel_type* p = (pixel_type*)m_rbuf->row_ptr(x, y, len) + x;
            do 
            {
                *p++ = m_blender.make_pix(colors->r, colors->g, colors->b);
                ++colors;
            }
            while(--len);
        }

        //--------------------------------------------------------------------
        void copy_color_vspan(int x, int y,
                              unsigned len, 
                              const color_type* colors)
        {
            do 
            {
                pixel_type* p = (pixel_type*)m_rbuf->row_ptr(x, y++, 1) + x;
                *p = m_blender.make_pix(colors->r, colors->g, colors->b);
                ++colors;
            }
            while(--len);
        }

        //--------------------------------------------------------------------
        void blend_color_hspan(int x, int y,
                               unsigned len, 
                               const color_type* colors,
                               const int8u* covers,
                               int8u cover)
        {
            pixel_type* p = (pixel_type*)m_rbuf->row_ptr(x, y, len) + x;
            do 
            {
                copy_or_blend_pix(p++, *colors++, covers ? *covers++ : cover);
            }
            while(--len);
        }

        //--------------------------------------------------------------------
        void blend_color_vspan(int x, int y,
                               unsigned len, 
                               const color_type* colors,
                               const int8u* covers,
                               int8u cover)
        {
            do 
            {
                copy_or_blend_pix((pixel_type*)m_rbuf->row_ptr(x, y++, 1) + x, 
                                  *colors++, covers ? *covers++ : cover);
            }
            while(--len);
        }
        
        //--------------------------------------------------------------------
        template<class RenBuf2>
        void copy_from(const RenBuf2& from, 
                       int xdst, int ydst,
                       int xsrc, int ysrc,
                       unsigned len)
        {
            const int8u* p = from.row_ptr(ysrc);
            if(p)
            {
                std::memmove(m_rbuf->row_ptr(xdst, ydst, len) + xdst * pix_width, 
                        p + xsrc * pix_width, 
                        len * pix_width);
            }
        }

        //--------------------------------------------------------------------
        template<class SrcPixelFormatRenderer>
        void blend_from(const SrcPixelFormatRenderer& from, 
                        int xdst, int ydst,
                        int xsrc, int ysrc,
                        unsigned len,
                        int8u cover)
        {
            typedef typename SrcPixelFormatRenderer::order_type src_order;

            const value_type* psrc = (const value_type*)from.row_ptr(ysrc);
            if(psrc)
            {
                psrc += xsrc * 4;
                pixel_type* pdst = 
                    (pixel_type*)m_rbuf->row_ptr(xdst, ydst, len) + xdst;
                do 
                {
                    value_type alpha = psrc[src_order::A];
                    if(alpha)
                    {
                        if(alpha == base_mask && cover == 255)
                        {
                            *pdst = m_blender.make_pix(psrc[src_order::R], 
                                                       psrc[src_order::G],
                                                       psrc[src_order::B]);
                        }
                        else
                        {
                            m_blender.blend_pix(pdst, 
                                                psrc[src_order::R],
                                                psrc[src_order::G],
                                                psrc[src_order::B],
                                                alpha,
                                                cover);
                        }
                    }
                    psrc += 4;
                    ++pdst;
                }
                while(--len);
            }
        }

        //--------------------------------------------------------------------
        template<class SrcPixelFormatRenderer>
        void blend_from_color(const SrcPixelFormatRenderer& from, 
                              const color_type& color,
                              int xdst, int ydst,
                              int xsrc, int ysrc,
                              unsigned len,
                              int8u cover)
        {
            typedef typename SrcPixelFormatRenderer::value_type src_value_type;
            typedef typename SrcPixelFormatRenderer::color_type src_color_type;
            const src_value_type* psrc = (src_value_type*)from.row_ptr(ysrc);
            if(psrc)
            {
                psrc += xsrc * SrcPixelFormatRenderer::pix_step + SrcPixelFormatRenderer::pix_offset;
                pixel_type* pdst = 
                    (pixel_type*)m_rbuf->row_ptr(xdst, ydst, len) + xdst;

                do 
                {
                    m_blender.blend_pix(pdst, 
                                        color.r, color.g, color.b, color.a,
                                        cover);
                    psrc += SrcPixelFormatRenderer::pix_step;
                    ++pdst;
                }
                while(--len);
            }
        }

        //--------------------------------------------------------------------
        template<class SrcPixelFormatRenderer>
        void blend_from_lut(const SrcPixelFormatRenderer& from, 
                            const color_type* color_lut,
                            int xdst, int ydst,
                            int xsrc, int ysrc,
                            unsigned len,
                            int8u cover)
        {
            typedef typename SrcPixelFormatRenderer::value_type src_value_type;
            const src_value_type* psrc = (src_value_type*)from.row_ptr(ysrc);
            if(psrc)
            {
                psrc += xsrc * SrcPixelFormatRenderer::pix_step + SrcPixelFormatRenderer::pix_offset;
                pixel_type* pdst = 
                    (pixel_type*)m_rbuf->row_ptr(xdst, ydst, len) + xdst;

                do 
                {
                    const color_type& color = color_lut[*psrc];
                    m_blender.blend_pix(pdst, 
                                        color.r, color.g, color.b, color.a,
                                        cover);
                    psrc += SrcPixelFormatRenderer::pix_step;
                    ++pdst;
                }
                while(--len);
            }
        }



    private:
        rbuf_type* m_rbuf;
        Blender    m_blender;
#endif
    };






    void compose_path(  agg::path_storage         &   m_path )
    {
        unsigned flag = 0;
        // if (m_close.cur_item() == 1) flag = agg::path_flags_cw;
        // if (m_close.cur_item() == 2) flag = agg::path_flags_ccw;

        flag = agg::path_flags_cw;

        m_path.remove_all();
        m_path.move_to(28.47, 6.45);
        m_path.curve3(21.58, 1.12, 19.82, 0.29);
        m_path.curve3(17.19, -0.93, 14.21, -0.93);
        m_path.curve3(9.57, -0.93, 6.57, 2.25);
        m_path.curve3(3.56, 5.42, 3.56, 10.60);
        m_path.curve3(3.56, 13.87, 5.03, 16.26);
        m_path.curve3(7.03, 19.58, 11.99, 22.51);
        m_path.curve3(16.94, 25.44, 28.47, 29.64);
        m_path.line_to(28.47, 31.40);
        m_path.curve3(28.47, 38.09, 26.34, 40.58);
        m_path.curve3(24.22, 43.07, 20.17, 43.07);
        m_path.curve3(17.09, 43.07, 15.28, 41.41);
        m_path.curve3(13.43, 39.75, 13.43, 37.60);
        m_path.line_to(13.53, 34.77);
        m_path.curve3(13.53, 32.52, 12.38, 31.30);
        m_path.curve3(11.23, 30.08, 9.38, 30.08);
        m_path.curve3(7.57, 30.08, 6.42, 31.35);
        m_path.curve3(5.27, 32.62, 5.27, 34.81);
        m_path.curve3(5.27, 39.01, 9.57, 42.53);
        m_path.curve3(13.87, 46.04, 21.63, 46.04);
        m_path.curve3(27.59, 46.04, 31.40, 44.04);
        m_path.curve3(34.28, 42.53, 35.64, 39.31);
        m_path.curve3(36.52, 37.21, 36.52, 30.71);
        m_path.line_to(36.52, 15.53);
        m_path.curve3(36.52, 9.13, 36.77, 7.69);
        m_path.curve3(37.01, 6.25, 37.57, 5.76);
        m_path.curve3(38.13, 5.27, 38.87, 5.27);
        m_path.curve3(39.65, 5.27, 40.23, 5.62);
        m_path.curve3(41.26, 6.25, 44.19, 9.18);
        m_path.line_to(44.19, 6.45);
        m_path.curve3(38.72, -0.88, 33.74, -0.88);
        m_path.curve3(31.35, -0.88, 29.93, 0.78);
        m_path.curve3(28.52, 2.44, 28.47, 6.45);
        m_path.close_polygon(flag);

        m_path.move_to(28.47, 9.62);
        m_path.line_to(28.47, 26.66);
        m_path.curve3(21.09, 23.73, 18.95, 22.51);
        m_path.curve3(15.09, 20.36, 13.43, 18.02);
        m_path.curve3(11.77, 15.67, 11.77, 12.89);
        m_path.curve3(11.77, 9.38, 13.87, 7.06);
        m_path.curve3(15.97, 4.74, 18.70, 4.74);
        m_path.curve3(22.41, 4.74, 28.47, 9.62);
        m_path.close_polygon(flag);
    }


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

*/

int agg_test2()
{

    // packed rgb565 
    typedef ::pixfmt_alpha_blend_rgb_packed<agg::blender_rgb565, agg::rendering_buffer> pixfmt_t;


    pixfmt_t  pixf;


     agg::renderer_base<pixfmt_t>   rb(pixf);

     rb.clear(agg::rgba(0,0,1));     // blue.


    agg::path_storage            m_path;

    compose_path(     m_path );



    agg::rasterizer_scanline_aa<> ras;
    agg::scanline_p8 sl;


    ras.add_path( m_path );

    agg::render_scanlines_aa_solid(ras, sl, /*renb*/ rb, agg::rgba(1,0,0));

    // non anti aliased.
    // agg::renderer_scanline_bin_solid( 



    return 0;
}

