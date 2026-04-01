


#include <stdio.h>    // printf, scanf
#include <string.h>   // strcmp, memset
#include <stdint.h>
#include <assert.h>
#include <ctype.h>    // isdigit()



#include <peripheral/vfd.h>
#include <peripheral/vfd-font-large.h>


#include <lib2/util.h>      // UNUSED, ARRAY_SIZE





// TODO rename 'write' to 'blit'.


// looks good.
//     BMH-fonts/bmh_fonts/bmh_char/Bitstream Vera Sans/Bitstream Vera Sans_64.png
// https://github.com/jdmorise/BMH-fonts/blob/master/bmh_fonts/bmh_char/Bitstream%20Vera%20Sans/Bitstream%20Vera%20Sans_24.h
// actually use the mono
//     BMH-fonts/bmh_fonts/bmh_char/Bitstream Vera Sans Mono
// /Bitstream Vera Sans Mono_64.png
// https://github.com/jdmorise/BMH-fonts/blob/master/bmh_fonts/bmh_char/Bitstream%20Vera%20Sans%20Mono/Bitstream%20Vera%20Sans%20Mono_64.png


#include <fonts/Bitstream_Vera_Sans_Mono_24.h>



static void vfd_font_large_write_char( vfd_t *vfd, uint8_t xpix, uint8_t ychar, const char *bitmap, uint8_t width )
{
  assert( vfd && vfd->magic == VFD_MAGIC);
  // important - note that it didn't need to be rotated.
  // need to try another letter.  eg. 3.

  // bitmap_33.  width is 9.  3x3     '!'
  // bitmap 34   width is 21  7x3
  // bitmap 48   width is 36  12x3
  // ??


  vfd_setincx( vfd);


  for(unsigned k = 0; k < 3; ++k )  {

    vfd_setx( vfd, xpix );
    vfd_sety( vfd, ychar  + k );

    for(unsigned i = 0; i < width ; ++i)              // how far horizontally
      vfd_write_data( vfd, bitmap[ (k * width) + i ]  );   // could just use a pointer.
  }
}



void vfd_font_large_write_proportional( vfd_t *vfd, const char *s, uint8_t xpix, uint8_t ychar )
{
  // proportional spacing
  assert( vfd && vfd->magic == VFD_MAGIC);

  const size_t n = strlen( s);

  for(unsigned i = 0; i < n; ++i) {

    uint32_t char_idx = s[i] - '!';
    assert(char_idx < ARRAY_SIZE( char_addr));

    vfd_font_large_write_char( vfd, xpix , /*0*/ ychar, char_addr[ char_idx ], char_width[ char_idx ] );
    xpix += char_width[ char_idx ] + 1;
  }
}




void vfd_font_large_write_special( vfd_t *vfd, const char *s, uint8_t xpix, uint8_t ychar )
{
  // mono spacing.
  assert( vfd && vfd->magic == VFD_MAGIC);

  const size_t n = strlen( s);

  for(unsigned i = 0; i < n; ++i) {


    uint32_t char_idx = s[i] - '!';
    char ch = s[ i] ;

    assert(char_idx < ARRAY_SIZE( char_addr));

    vfd_font_large_write_char( vfd, xpix , /*0*/ ychar, char_addr[ char_idx ], char_width[ char_idx ] );


    if( isdigit( (unsigned char) ch)
      || ch == '+' || ch == '-')
    {
      // for digits - use mono-space advance. reference'0'
      xpix += char_width[ '0' - '!' ] + 1;
    }
    else {
      // for anything else including punctuation, use proportional advance
      xpix += char_width[ char_idx] + 1;
    }

  }
}




void vfd_clear( vfd_t *vfd)
{
  assert( vfd && vfd->magic == VFD_MAGIC);

  assert( vfd->height_bytes == 8);
  assert( vfd->width == 128);

  vfd_setincx( vfd);
  for( unsigned y = 0; y < vfd->height_bytes; ++y ) {

    vfd_setx( vfd, 0 );
    vfd_sety( vfd, y );

    for( unsigned x = 0; x < vfd->width; ++x) {

      vfd_write_data( vfd, 0x00 );
    }
  }

}



// move or at least rename test.

void vfd_test( vfd_t *vfd)
{
  assert( vfd && vfd->magic == VFD_MAGIC);

#if 0
  // display clear - it is part of sequence in s8. so may be required
  // vfd_write_cmd( 0x5f ); // need to turn on again.
  // vfd_write_cmd( 0x5f | (1 << 2) );

  printf("vfd test()\n");

  vfd_font_small_write( vfd, "hello", 0, 3 );
  vfd_font_small_write( vfd, "WORLD", 0, 4 );
  vfd_font_small_write( vfd, "123467890", 0, 5 );
#endif

  // vfd_font_large_write2( vfd, "apple", 0 , 0 );
  // vfd_font_large_write2( vfd, "7.168,259,0", 0 , 0 );
  vfd_font_large_write_proportional( vfd, "7.168,259,0", 0 , 0 );

}




#if 0

static void vfd_do_something_old(void)
{

  printf("vfd_do_something()\n");

  // vfd_write_cmd( 0b10000100 );          // set igx  to increment x,  bits are vertical. odd.
  vfd_write_cmd( 0b10000010 );          // set igy  to increment y,     bits are vertical. odd.

  vfd_write_cmd( 0b01100100);          // write vfd_setx
  vfd_write_cmd( 3 );                  // 30pixels to the right.

  vfd_write_cmd( 0b01100000  );          // data write vfd_sety
  vfd_write_cmd( 3 );                     // eg. moves down 3x8=24 bits.

  for(unsigned i = 0; i < 4; ++i)
    vfd_write_data( 0b10101010 );     // OK. this *is* writing to the ramp. but in a funny position.

  // it is incrementing x.  and then drawing y as 8 bits vertically.
  // but it is not at 10pixels.  actually maybe it is at 80pixels.
  // ok. it's very weird there is one 3 bit
}

#endif


/*

  ///////////
  // letter 0. char 48
  // https://github.com/jdmorise/BMH-fonts/blob/master/bmh_fonts/bmh_digits/Arimo%20Regular/Arimo%20Regular_24.h
  // const char bitmap_48[] = {128,224,240,56,24,24,24,24,48,240,224,0,255,255,129,0,0,0,0,0,0,129,255,255,0,7,15,12,24,24,24,24,12,15,7,0};

  // const char bitmap_51[] = {192,224,240,56,24,24,24,24,56,240,224,0,0,0,0,0,24,24,24,24,60,247,227,192,3,15,14,28,24,24,24,24,28,15,15,3};
  // assert(sizeof(bitmap_51) == 36);


  // bitmap_33.  width is 9.  3x3     '!'
  // bitmap 34   width is 21  7x3
  // bitmap 48   width is 36  12x3
  // ??


  uint32_t char_idx = 'a' - '!';

  uint32_t x = 0;

  // vfd_write_big_zero
  vfd_font_large_write_char( x , 0, char_addr[ char_idx ], char_width[ char_idx ] );

  x += char_width[ char_idx ] + 2;

  char_idx = 'b' - '!';
  vfd_font_large_write_char( x , 0, char_addr[ char_idx ], char_width[ char_idx ] );

*/


