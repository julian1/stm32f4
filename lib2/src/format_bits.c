
#include "format_bits.h"


// input needs to be width + 1 for sentinel

char * format_bits(char *buf, size_t width, uint32_t value)
{
  // passing the buf, means can use more than once in printf expression. using separate bufs
  char *s = buf;

  for(int i = width - 1; i >= 0; --i) {
    *s++ = value & (1 << i) ? '1' : '0';
  }

  *s = 0;
  return buf;
}


/*
int main()
{
  char buf[1000];


  uint32_t data  = 0xff;

  printf(  "%s\n" , uint_to_bits(buf, 16, SETFIELD(data, 3 , 1 , 0b101 ) ));

  // can also use like this, passing 0 as argument.
  printf(  "%s\n" , uint_to_bits(buf, 16, SETFIELD(0, 3, 1, 0b101 ) ));
  printf(  "%s\n" , uint_to_bits(buf, 16, SETFIELD(0, 4, 4, 0b1111 ) ));
  printf(  "-------\n");
  printf(  "%s\n" , uint_to_bits(buf, 16, GETFIELD( 0b11110000, 4, 2)  ));
  printf(  "%s\n" , uint_to_bits(buf, 16, GETFIELD( 0b00001111, 4, 2)  ));
}

*/

