

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


#include <lzo/unlzo.h>

#include <lib2/util.h>    // msleep
#include <lib2/stream-flash.h>


// fix me
int flash_lzo_test(void);


static void myerror(char *x)
{
  printf("my error: %s\n", x);
}



static FILE *fill_file = NULL;

static long fill(void*p, unsigned long n)
{
  /*
    problem here is no callback ctx for communication.
    so use static variable for communication
    to avoid needing to modify lzo code. to add additional ctx argument
  */
  assert(p);
  assert(fill_file);
  // read 1 byte,  n times.

/*
  printf("fill : n %lu\n", n );

  return 0;

  for(unsigned i = 0; i < 100000000; i++) {
    __asm__("nop");
  }
*/

  size_t ret = fread(p, 1, n, fill_file);
  printf("fill : %lu     ret %u\n", n, ret );


  return ret;
}



static long flush(void*p, unsigned long n)
{
  printf("flush : %lu\n", n);
  size_t ret =  fwrite(p, 1, n,  stdout );
  return ret;
}


// same as ice40 bitstream
#define FLASH_SECT_ADDR   0x08060000


int flash_lzo_test(void)
{
  // assert(argc == 2);
  // fill_file = fopen(argv[1], "r") ;

  printf("flash lzo test 0\n");
  fill_file = flash_open_file(  FLASH_SECT_ADDR );

  printf("lzo test 1\n");
  assert(fill_file);

  printf("lzo test 2\n");


#if 1
/*
int unlzo(unsigned char *inbuf, long len,
	long (*fill)(void*, unsigned long),
	long (*flush)(void*, unsigned long),
	unsigned char *output,
	long *pos,
	void(*error)(char *x));
*/


  int ret  = unlzo(
    NULL, 0,
    fill,
    flush,
    NULL,
    0,
    myerror
  );



  printf("ret is %u\n", ret );
  assert(ret == 0); // success

  ret = fclose(fill_file);
  assert(ret == 0);

#endif
  return 0;
}




#if 0
int main(int argc, char **argv)
{
  assert(argc == 2);

  fill_file = fopen(argv[1], "r") ;
  assert(fill_file);

/*
int unlzo(unsigned char *inbuf, long len,
	long (*fill)(void*, unsigned long),
	long (*flush)(void*, unsigned long),
	unsigned char *output,
	long *pos,
	void(*error)(char *x));
*/


  // char *buf_out = malloc( file_sz);
  // assert(buf_out);

  int ret  = unlzo(
    NULL, 0,
    fill,
    flush,
    NULL,
    0,
    myerror
  );


  printf("ret is %u\n", ret );
  assert(ret == 0); // success

  ret = fclose(fill_file);
  assert(ret == 0);
}

#endif



