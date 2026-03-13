

#include <stdio.h>    // printf, scanf
#include <string.h>   // strcmp, memset
#include <stdint.h>
#include <assert.h>


#include <peripheral/vfd.h>

#include <lib2/util.h>      // UNUSED, ARRAY_SIZE


/*
  eg. bits are horiz. bytes are vertical.
  letter A.   { 0x08, 0x14, 0x22, 0x22, 0x3E, 0x22, 0x22, 0x00,  } ;  // A
  0x08 =   "001000"
  0x14 =   "010100"
  0x22 =   "100010"
  0x22 =   "100010"
  0x3E =   "111110"
  0x22 =   "100010"
  0x22 =   "100010"
  0x00 =   "000000"
*/

#include <fonts/font.h>



// char inArray[8];
// char outArray[8];

static void rotate_and_reverse (  /*char inArray[8]*/ const uint8_t *inArray , uint8_t outArray[8])
{
  // https://forum.arduino.cc/t/rotating-a-2d-matrix/287097/2

  int i, j, val;

  for (i = 0; i < 8; i++) {
    outArray[i] = 0;
  }

  //rotate 90* clockwise
  for (i = 0; i < 8; i++) {
    for (j = 0; j < 8; j++) {
      val = ((inArray[i] >> j) & 1); //extract the j-th bit of the i-th element

      // outArray[7-j] |= (val << i); //set the newJ-th bit of the newI-th element
      // reverse.
      outArray[j] |= (val << i); //set the newJ-th bit of the newI-th element
    }
  }
}



static void setx( uint8_t xpix )
{
    vfd_write_cmd( 0b01100100);          // write setx
    vfd_write_cmd( xpix );
}

static void sety( uint8_t ychar )
{
    vfd_write_cmd( 0b01100000  );          // data write sety
    vfd_write_cmd( ychar  );
}


static void setincx( void )
{

    vfd_write_cmd( 0b10000100 );          // set igx  to increment x,  bits are vertical. odd.
}


static void vfd_write_char( uint8_t ch, uint8_t xpix, uint8_t ychar )
{
  // const char FONT[] = { 0x1C, 0x22, 0x2A, 0x3A, 0x1A, 0x02, 0x3C, 0x00, } ;   // @
  // const char FONT[] = { 0x08, 0x14, 0x22, 0x22, 0x3E, 0x22, 0x22, 0x00,  } ;  // A
  // with an 8pix vertical height.  the direct drawing (without buffer) is quite neat.
    // printf("@ is char %lu\n", (uint32_t) '@'  );    // 64decimal == 40hex. it's correct.

  // issue is that with vertical - we need to rotate 90degs.
  // this is expensive and should be removed from here
  uint8_t letter[ 8];
  rotate_and_reverse(  & FONT[ (( uint32_t) ch ) * 8 ] , letter ) ;

  setincx();
  setx( xpix );
  sety( ychar );

  for(unsigned i = 0; i < 8; ++i)
    // vfd_write_data( FONT[ (( uint32_t) 'A' ) * 8 +  i ] );     // OK. this *is* writing to the ramp. but in a funny position.
    vfd_write_data( letter[ i ]  );
}


static void vfd_write_string( const char *s, size_t n, uint8_t xpix, uint8_t ychar )
{
  /* feb 2026.  have a func like this in the device.
    so ammotize the overhead of a virtual call.     
  */

  // note that 0, sential is a character which is quite nice.
  for(unsigned i = 0; i < n; ++i) {
    vfd_write_char( s[ i], xpix + (i * 7), ychar );
  }
}

void vfd_write_string2( const char *s, uint8_t xpix, uint8_t ychar )
{

  vfd_write_string( s, strlen(s), xpix, ychar );
}






/////////////////////////////////////////////////////////







// TODO consider change 'write' to 'blit'.


// looks good.
//     BMH-fonts/bmh_fonts/bmh_char/Bitstream Vera Sans/Bitstream Vera Sans_64.png
// https://github.com/jdmorise/BMH-fonts/blob/master/bmh_fonts/bmh_char/Bitstream%20Vera%20Sans/Bitstream%20Vera%20Sans_24.h
// actually use the mono
//     BMH-fonts/bmh_fonts/bmh_char/Bitstream Vera Sans Mono
// /Bitstream Vera Sans Mono_64.png
// https://github.com/jdmorise/BMH-fonts/blob/master/bmh_fonts/bmh_char/Bitstream%20Vera%20Sans%20Mono/Bitstream%20Vera%20Sans%20Mono_64.png

#include <fonts/Bitstream_Vera_Sans_Mono_24.h>


static void vfd_write_bitmap_char( uint8_t xpix, uint8_t ychar, const char *bitmap, uint8_t width )
{
  // important - note that it didn't need to be rotated.
  // need to try another letter.  eg. 3.

  // bitmap_33.  width is 9.  3x3     '!'
  // bitmap 34   width is 21  7x3
  // bitmap 48   width is 36  12x3
  // ??


  setincx();


  for(unsigned k = 0; k < 3; ++k )  {

    setx( xpix );
    sety( ychar  + k );

    for(unsigned i = 0; i < width ; ++i)              // how far horizontally
      vfd_write_data( bitmap[ (k * width) + i ]  );   // could just use a pointer.
  }
}



static void vfd_write_bitmap_string( const char *s, size_t n, uint8_t xpix, uint8_t ychar )
{
  // uses proporitional spacing.
  // can also have fixed width.

  /*
    - the pixel wide space added between chars - needs to be cleared.
    - or use monospace.
    - possibly want a monospace format version .
    - avoid - a separate blanking pass, that would need to be synchronized with device scan.

  */

  for(unsigned i = 0; i < n; ++i) {

    uint32_t char_idx = s[i] - '!';
    assert(char_idx < ARRAY_SIZE( char_addr));

    vfd_write_bitmap_char( xpix , /*0*/ ychar, char_addr[ char_idx ], char_width[ char_idx ] );
    xpix += char_width[ char_idx ] + 1;
  }
}



void vfd_write_bitmap_string2( const char *s, uint8_t xpix, uint8_t ychar )
{
  vfd_write_bitmap_string( s, strlen(s), xpix, ychar );
}






void vfd_do_something(void)
{
  // display clear - it is part of sequence in s8. so may be required
  // vfd_write_cmd( 0x5f ); // need to turn on again.
  // vfd_write_cmd( 0x5f | (1 << 2) );

  printf("vfd_do_something()\n");

  vfd_write_string2( "hello", 0, 3 );
  vfd_write_string2( "WORLD", 0, 4 );
  vfd_write_string2( "123467890", 0, 5 );

  // vfd_write_bitmap_string2( "apple", 0 , 0 );
  vfd_write_bitmap_string2( "7.168,259,0", 0 , 0 );

}




#if 0

static void vfd_do_something_old(void)
{

  printf("vfd_do_something()\n");

  // vfd_write_cmd( 0b10000100 );          // set igx  to increment x,  bits are vertical. odd.
  vfd_write_cmd( 0b10000010 );          // set igy  to increment y,     bits are vertical. odd.

  vfd_write_cmd( 0b01100100);          // write setx
  vfd_write_cmd( 3 );                  // 30pixels to the right.

  vfd_write_cmd( 0b01100000  );          // data write sety
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
  vfd_write_bitmap_char( x , 0, char_addr[ char_idx ], char_width[ char_idx ] );

  x += char_width[ char_idx ] + 2;

  char_idx = 'b' - '!';
  vfd_write_bitmap_char( x , 0, char_addr[ char_idx ], char_width[ char_idx ] );

*/


