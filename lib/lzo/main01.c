






#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


#include "unlzo.h"





void myerror(char *x)
{
  printf("my error: %s\n", x);
}


int main(int argc, char **argv)
{
  assert(argc == 2);

  FILE *f = fopen(argv[1], "r") ;
  assert(f);

  long seek_pos = fseek(f, 0, SEEK_END);
  assert(seek_pos == 0);

  long file_sz = ftell(f);
  printf("file_sz pos %lu\n", file_sz);

  // rewind(f); 
  fseek(f, 0L, SEEK_SET);

  char *buf = malloc( file_sz );
  assert(buf);

  size_t n = fread(buf , file_sz, 1, f); 
  assert(n == 1);

/*
int unlzo(unsigned char *inbuf, long len,
	long (*fill)(void*, unsigned long),
	long (*flush)(void*, unsigned long),
	unsigned char *output,
	long *pos,
	void(*error)(char *x));
*/


  char *buf_out = malloc( file_sz);
  assert(buf_out);

  int ret  = unlzo(buf, file_sz,
    NULL,
    NULL,
    buf_out,
    0,
    myerror 
  );

  printf("ret is %u\n", ret );
 
  // overflows. 
  printf("%s\n", buf_out );
}





