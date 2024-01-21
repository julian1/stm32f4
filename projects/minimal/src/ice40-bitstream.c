


// #include <stdint.h>
// #include <stdio.h>
// #include <stdlib.h>
#include <assert.h>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>


#include <lib2/util.h>    // MAX
#include <lib2/stream-flash.h>


#include <ice40-bitstream.h>







/*
  need to pass the spi to use.


*/

int ice40_bitstream_test(void)
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

    ///  print deails of blob.
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




#if 0

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

#endif


