

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


#include <lzo/unlzo.h>

#include <lib2/util.h>    // msleep
#include <lib2/stream-flash.h>


// fix me
int flash_lzo_test(void);
int flash_raw_test(void);
int flash_raw_test2(void);


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


/*
#define MIN(i, j) (((i) < (j)) ? (i) : (j))
#define MAX(i, j) (((i) > (j)) ? (i) : (j))
*/

int flash_raw_test2(void)
{
  printf("flash raw test 2\n");
  FILE *f = flash_open_file();
  assert(f);


  // read magic and length.

  uint32_t magic  = 0 ;
  uint32_t size = 0;

  fread(&magic, 1, 4, f);
  fread(&size, 1, 4, f);


  printf("magic %lx\n", magic );
  printf("size %lu\n", size );        // need to swap the byte order perhaps.

  // if(magic != 0xfe00fe00) {
  if(magic != 0xfe00fe00) {
    printf("bad magic!\n");
    fclose(f);
    return -1;
  }


  // I think there may be an issue - with the flash sector????
  // it fails and repeats at 4k to go. which is about 130k - 128k.

  // like our circular bufer fails...

  size_t remaining = size;
  while(remaining > 0) {


    char buf[10000];
    size_t ret = 0;

    if(remaining >= 10000) {
      ret =  fread(buf, 1, 10000, f);
      assert(ret);
    }
    else {
      ret = fread(buf, 1, remaining , f);
      assert(ret);
    }

    printf("%u   ", ret);

    for(unsigned i = 0; i < 8; ++i)
      printf("%2x ", buf[i]);

    printf(" .. ");

    // not sure if the max deals with signedness here...
    for(unsigned i = MAX( (signed)ret - 8, 0); i < ret; ++i)
      printf("%2x ", buf[i]);

    printf("\n");

    remaining -= ret;
  }


  printf("done remaining now %u\n", remaining);

  assert(remaining == 0);


  fclose(f);
  return 0;
}









// should be passing an argument to check.

int flash_raw_test(void)
{
  printf("flash raw test\n");
  FILE *f = flash_open_file();
  assert(f);
  // print first 100 chars.
  char buf[ 100 ] ;
  size_t ret = fread(buf, 1, 100, f);
  if(!ret ) {
      printf("flash read returned nothing\n");
  } else {

    for(unsigned i = 0; i < ret; ++i ) {
      putchar( buf[ i] );
    }
    printf("\n");
    for(unsigned i = 0; i < ret; ++i ) {
      printf("%2x ", buf[ i] );
    }

    printf("\n");
  }

  fclose(f);
  return 0;
}




int flash_lzo_test(void)
{
  // assert(argc == 2);
  // fill_file = fopen(argv[1], "r") ;

  printf("flash lzo test 0\n");
  fill_file = flash_open_file();

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



