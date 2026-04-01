
/*
  fonts.

  peripheral vfd code. not instance specific
  but pretty general code, and probably does not belong in peripheral

*/

#include <stdio.h>    // printf, scanf
#include <string.h>   // strcmp, memset
#include <stdint.h>
#include <assert.h>


// for vfd commands
#include <peripheral/vfd.h>

#include <peripheral/vfd-font-small.h>

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

#include <fonts/apple2_hgr_font.h>



// char inArray[8];
// char outArray[8];

static void rotate_and_reverse (  /*char inArray[8]*/ const uint8_t *inArray , uint8_t outArray[8])
{
  // EXTR. should be precomputed. and cached for speed.
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




static void vfd_write_char( vfd_t *vfd, uint8_t ch, uint8_t xpix, uint8_t ychar)
{
  // const char FONT[] = { 0x1C, 0x22, 0x2A, 0x3A, 0x1A, 0x02, 0x3C, 0x00, } ;   // @
  // const char FONT[] = { 0x08, 0x14, 0x22, 0x22, 0x3E, 0x22, 0x22, 0x00,  } ;  // A
  // with an 8pix vertical height.  the direct drawing (without buffer) is quite neat.
    // printf("@ is char %lu\n", (uint32_t) '@'  );    // 64decimal == 40hex. it's correct.

  // issue is that with vertical - we need to rotate 90degs.
  // this is expensive and should be removed from here
  uint8_t letter[ 8];
  rotate_and_reverse(  & FONT[ (( uint32_t) ch ) * 8 ] , letter ) ;

  vfd_setincx( vfd);
  vfd_setx( vfd, xpix );
  vfd_sety( vfd, ychar );

  for(unsigned i = 0; i < 8; ++i)
    // vfd_write_data( FONT[ (( uint32_t) 'A' ) * 8 +  i ] );     // OK. this *is* writing to the ramp. but in a funny position.
    vfd_write_data( vfd, letter[ i ]  );
}




// static void vfd_write_string_special( vfd_t *vfd, const char *s, size_t n, uint8_t xpix, uint8_t ychar)

void vfd_font_small_write( vfd_t *vfd, const char *s, uint8_t xpix, uint8_t ychar)
{
  const size_t n = strlen( s);

  // note that 0, sential is a character which is quite nice.
  for(unsigned i = 0; i < n; ++i) {

    vfd_write_char( vfd, s[ i], xpix, ychar);
    xpix += 7;
  }
}





void vfd_font_small_write_special( vfd_t *vfd, const char *s, uint8_t xpix, uint8_t ychar)
{

  const size_t n = strlen( s);

  // note that 0, sential is a character which is quite nice.
  for(unsigned i = 0; i < n; ++i) {

    vfd_write_char( vfd, s[ i], xpix, ychar);

    if( s[i] == ',' || s[i] == '.')
    {
      // simple kerning, reduce advance
      xpix += 4;
    }
    else {
      // ordinary monospace
      xpix += 7;
    }
  }
}







#if 0
void vfd_font_small_write( vfd_t *vfd, const char *s, uint8_t xpix, uint8_t ychar)
{

  vfd_write_string( vfd, s, strlen(s), xpix, ychar);
}
#endif


