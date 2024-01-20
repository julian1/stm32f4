
/*
  Ideally we should be modifying platform support...

  that way - examples should work.
*/

#include "agg_basics.h"
// #include "agg_rendering_buffer.h"

#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"

#include "agg_scanline_p.h"
#include "agg_renderer_scanline.h"


#include "context.h"
#include "gfx.h"
#include "Adafruit_ILI9341.h"

// avoid including platform_support.h
namespace agg
{
     enum pix_format_e
    {
        pix_format_undefined = 0,  // By default. No conversions are applied
        pix_format_bw,             // 1 bit per color B/W
        pix_format_gray8,          // Simple 256 level grayscale
        pix_format_sgray8,         // Simple 256 level grayscale (sRGB)
        pix_format_gray16,         // Simple 65535 level grayscale
        pix_format_gray32,         // Grayscale, one 32-bit float per pixel
        pix_format_rgb555,         // 15 bit rgb. Depends on the byte ordering!
        pix_format_rgb565,         // 16 bit rgb. Depends on the byte ordering!
        pix_format_rgbAAA,         // 30 bit rgb. Depends on the byte ordering!
        pix_format_rgbBBA,         // 32 bit rgb. Depends on the byte ordering!
        pix_format_bgrAAA,         // 30 bit bgr. Depends on the byte ordering!
        pix_format_bgrABB,         // 32 bit bgr. Depends on the byte ordering!
        pix_format_rgb24,          // R-G-B, one byte per color component
        pix_format_srgb24,         // R-G-B, one byte per color component (sRGB)
        pix_format_bgr24,          // B-G-R, one byte per color component
        pix_format_sbgr24,         // B-G-R, native win32 BMP format (sRGB)
        pix_format_rgba32,         // R-G-B-A, one byte per color component
        pix_format_srgba32,        // R-G-B-A, one byte per color component (sRGB)
        pix_format_argb32,         // A-R-G-B, native MAC format
        pix_format_sargb32,        // A-R-G-B, native MAC format (sRGB)
        pix_format_abgr32,         // A-B-G-R, one byte per color component
        pix_format_sabgr32,        // A-B-G-R, one byte per color component (sRGB)
        pix_format_bgra32,         // B-G-R-A, native win32 BMP format
        pix_format_sbgra32,        // B-G-R-A, native win32 BMP format (sRGB)
        pix_format_rgb48,          // R-G-B, 16 bits per color component
        pix_format_bgr48,          // B-G-R, native win32 BMP format.
        pix_format_rgb96,          // R-G-B, one 32-bit float per color component
        pix_format_bgr96,          // B-G-R, one 32-bit float per color component
        pix_format_rgba64,         // R-G-B-A, 16 bits byte per color component
        pix_format_argb64,         // A-R-G-B, native MAC format
        pix_format_abgr64,         // A-B-G-R, one byte per color component
        pix_format_bgra64,         // B-G-R-A, native win32 BMP format
        pix_format_rgba128,        // R-G-B-A, one 32-bit float per color component
        pix_format_argb128,        // A-R-G-B, one 32-bit float per color component
        pix_format_abgr128,        // A-B-G-R, one 32-bit float per color component
        pix_format_bgra128,        // B-G-R-A, one 32-bit float per color component

        end_of_pix_formats
    };

};

// #define AGG_BGR24
//#define AGG_RGB24
//#define AGG_BGR96
//#define AGG_BGRA32
//#define AGG_RGBA32
//#define AGG_ARGB32
//#define AGG_ABGR32
//#define AGG_BGRA128
#define AGG_RGB565        // JA 16 bit
//#define AGG_RGB555
// #include "pixel_formats.h"
#include "../examples/pixel_formats.h"



/*
#include "agg_conv_transform.h"
#include "agg_bspline.h"
#include "agg_ellipse.h"
#include "agg_gsv_text.h"

#include "ctrl/agg_slider_ctrl.h"
#include "ctrl/agg_scale_ctrl.h"
#include "platform/agg_platform_support.h"

*/


extern "C" void agg_test(void);


/*
  Or. ew
  - strategy is still to have the screen
  - remember most gui items (buttons, text) will be much smaller than the screen.
      - and would be blitted. independently.  eg. active layer.
  - it's only the background... that would be a bit static
  - each character...
  - BUT. having character vectors loaded into memory is going to consume a lot of memory also.

  but a chart or graph - is more complicated...
*/

void agg_test(void)
{

  Context   ctx;

  // low level
  initialize(&ctx);

  ILI9341_setRotation(&ctx, 3); // 0 == trhs, 1 == brhs, 2 == blhs,  3 == tlhs


#if 1
    // 320*200*2 / 1000 = 128k...  and
    // stm32f410 only has 32k.
    // stm32f411  has 128k.
    // stm32f405/7 192+4 Kbytes of SRAM. but 64k is CCM (core coupled memory) data RAM. so maybe not contiguous?
    // STM32F412RE   has 256k ram.

    // stm32f407 board with extra ram chip, https://de.aliexpress.com/item/32689262341.html
    // needs FMC controller.
    // https://github.com/knielsen/pcb_stm32f4_sram/blob/master/pcb_sram.pdf
    // http://stm32f4-discovery.net/2014/05/library-14-working-with-sdram-on-stm32f429-discovery/

    // https://github.com/MaJerle/stm32f429/blob/master/14-STM32F429_SDRAM/User/main.c

    // libopencm3 - has sdram example for stm32f429 discovery. not particlarly simple.

    // So. cannot write to shared buffer - for antialiasing.
    // unless assume the pixel to blend is a background color. and there's no draw overlap.
    // OK. the renderer_base is the high-level interface.

    // libopencm3-examples/examples/stm32/f4/stm32f429i-discovery/sdram/sdram.c

    // sram (versus sdram) with stm32f429,
    // simpler interface.   uses fpga. but not stm32 fmc.
    // https://andybrown.me.uk/2014/06/01/ase/

    /*
      IMPORTANT
      - have an extent capture element in the rendering pipeline.
      - can then use those extents to determine what needs to be blitted.
        - eg. render some text. extract the extents. then blit to the display.
      ---------
      OR. just use bitmaps.  for text chars. and other elements, buttons.
        fastest, quickest path to strong visual environment.
        need external flash. - pretty important. 
      OR use VFD. because vastly simpler. graphically.
    */
    // 150*100 / 1000 = 15k
    int frame_width = 150; // 320;
    int frame_height = 80;
    // int frame_height = 90 or 100, draws ok, but also freezes blinky.
    unsigned char buffer[ frame_width * frame_height * 2];
#endif


#if 1

    agg::rendering_buffer rbuf(buffer,
                               frame_width,
                               frame_height,
                               frame_width * 2);

    agg::rasterizer_scanline_aa<> pf;
    agg::scanline_p8 sl;

    typedef agg::renderer_base<pixfmt> renderer_base;

    //typedef agg::pixfmt_rgb565 pixfmt;
    pixfmt pixf(rbuf);

    renderer_base rb(pixf);


    // rb.clear(agg::rgba(0,0,1));     // blue
    // rb.clear(agg::rgba(0.8,0.8,0.8));     // green.
    rb.clear(agg::rgba(1,1,0.8));     // green.

    // hmmmmm....   
    // maybe it expects an actual bitmap...

#endif
  ///////////////////////////


  // works
  ILI9341_DrawBuffer(&ctx , 0, 0, frame_width, frame_height, buffer );


  // works
  fillRect(&ctx, 20, 20, 40, 20, ILI9341_GREEN ); // works...

}





