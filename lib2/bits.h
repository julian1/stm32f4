

// bit manipulation


#define MASK(width)                           ((1<<(width)) - 1)

#define SETFIELD(data, width, offset, val)    (((data) & ~(MASK(width) << (offset))) | (((val) & MASK(width)) << (offset)))

#define GETFIELD(data, width, offset)         ((data) >> (offset)) & MASK(width)



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

