
/*
  flashing takes ages - because it's writing the empty sectors.
  and that means it will overwrite configuration

  but we could have initial code, and then subsequent code.
  that are flashed separately.
  relink.

  Sector 0 0x0800 0000 - 0x0800 3FFF 16 Kbytes
  Sector 1 0x0800 4000 - 0x0800 7FFF 16 Kbytes
  Sector 2 0x0800 8000 - 0x0800 BFFF 16 Kbytes
  Sector 3 0x0800 C000 - 0x0800 FFFF 16 Kbytes

  Sector 4 0x0801 0000 - 0x0801 FFFF 64 Kbytes  <- where we align.
  Sector 5 0x0802 0000 - 0x0803 FFFF 128 Kbytes
  Sector 6 0x0804 0000 - 0x0805 FFFF 128 Kbytes
  Sector 7 0x0806 0000 - 0x0807 FFFF 128 Kbytes
  System memory 0x1FFF 0000 - 0x1FFF 77FF 30 Kbytes
  OTP area 0x1FFF 7800 - 0x1FFF 7A0F 528 bytes
  Option bytes 0x1FFF C000 - 0x1FFF C00F 16 byte

  file:///tmp/mozilla_me0/DM00119316-.pdf
  https://medium.com/theteammavericks/programming-flash-rom-in-stm32-f5b7d6dcba4f

  --------
  libopencm3 flash code for f4 here,

  Note there are multiple flash files, linked in the libopencm3 Makefile for f4

  OBJS    += flash.o flash_common_all.o flash_common_f.o flash_common_f24.o
    eg.

    flash_wait_for_last_operation() is defined here ../../libopencm3/lib/stm32/f4/flash.c
                                    but used here, ../../libopencm3/lib/stm32/common/flash_common_f24.c

    flash_unlock(void)  is defined in
                                      ../../libopencm3/lib/stm32/common/flash_common_f.c

  flash is read by simply reading the memory address.
  ----------
  stm32f411ceu7.
  C = 256 Kbytes of Flash memory
  E = 512 Kbytes of Flash memory <- us.

  use flash probe under openocd/st-link to query
  flash probe 0

  last sector for 512k. is sector 7. i think. start 0806.
  https://electronics.stackexchange.com/questions/138400/flash-memory-range-on-stm32f429ii

  flash erase_sector 0 2 2 ;
  flash write_bank 0 out.txt  0x08000

  -----------------
  stm32f413
   flash erase_sector bank_id first_sector_num (last_sector_num|'last')

  # stop
  halt

  # to erase just sector 7
  flash erase_sector 0 7 7 ;

  # to write sector 7
  flash write_bank 0 /home/me/devel/lzo/file.txt  0x00060000

  reset run

  # for f413
  #  7: 0x00060000 (0x20000 128kB) not protected

*/


#define _GNU_SOURCE     // must be first. required for cookie_io_functions_t
#include <stdio.h>

#include <string.h>
#include <assert.h>

// #include <limits.h>   // INT_MAX



#include <libopencm3/stm32/flash.h>


#include <lib2/streams.h>  // printf
#include <lib2/usart.h>     // usart1_flush()

// careful - conflicts with lib2/include/flash.h
#include "stream-flash.h"


/*
  sector 2.
  needs linker script re-organization.


// globals / or use a structure?
#define FLASH_SECT_ADDR   0x08008000
#define FLASH_SECT_NUM    2
*/


/*
  If we passed these by argument instead.  at fopen()
  then this code could be moved to library code lib2.
*/
// last 128 . on 512k.
#define FLASH_SECT_ADDR   0x08060000
#define FLASH_SECT_NUM    7



void flash_erase_sector_(void)
{

  // put in a command
  printf("flash unlock\n");
  flash_unlock();

  printf("flase erase sector\n");
  usart1_flush();

  flash_erase_sector(FLASH_SECT_NUM, 0 );
  // unsigned char buf[] = "whoot";

  // could do a test read ensure. value...

  printf("flash lock\n");
  flash_lock();

}



static void * file_to_cookie( FILE *f )
{
  /* should not be exposed.
    but allows supporting other file based operations over our structure
  */

  // actually a handler
  void *cookie_ptr= f->_cookie;
  // printf("cookie_ptr %p\n", cookie_ptr);

 // follow it
 void *cookie = * (void **) cookie_ptr ;
 // printf ("cookie %p\n",  cookie );

  return cookie;
}



////////////////////////



/*
  - could pass a FILE ptr.
    and then extract the sector info from the cookie where it's saved.

  - fp_to_cookie() ;
*/



struct A
{
  unsigned char *p;
  unsigned      pos;
  // unsigned      n;    // limit


/*
  unsigned      flash_sect_address;
  unsigned      flash_sect_num;
*/

};

typedef struct A A;


static ssize_t mywrite( void *a_, const char *buf, size_t sz)
{
  A *a = (A *) a_;

  // printf("** mywrite %u\n", sz);
  //printf("a %p\n", a );
  // printf("a->pos %d", a->pos ); // value


  // alternatively might be able to return truncated sz...
  // assert( a->pos + sz < a->n);

  flash_program( (uint32_t)  a->p + a->pos , (const unsigned char *) buf, sz  );

  a->pos += sz;

  // printf("a->pos now %d\n", a->pos ); // value

  return sz;
}


static ssize_t myread(void *a_, char *buf, size_t sz)
{
  A *a = (A *) a_;

  // sz is just the advertized buffer space.

  /*
    on linux. requests 8192 bytes.
    on stm32 its 1024 bytes.
    this is a buffering action. it is ok to read past the local datastructure.
  */

  // printf("** myread %u\n", sz);
  //printf("a %p\n", a );
  // printf("a->pos %d\n", a->pos ); // value
  // usart1_flush();

/*
  int remain = a->n - a->pos;           // signed. but it's not quite correct

  // printf("remaining %u\n", remain );
  // usart1_flush();

  if(remain < (int)sz)
    sz = remain;

  assert(remain >= 0);
*/

  // printf("sz now %u\n", sz );
  // usart1_flush();

  memcpy(buf, a->p + a->pos, sz);
  a->pos += sz;

  return sz;
}



/*
  Nov 21, 2023.
  offset is type 'long int *'  not '_off64_t *' contrary to man page for fopencookie().
  this appears to have changed

*/
// static int myseek(A *a, _off64_t *offset_, int whence)
static int myseek(void *a_, long int *offset_, int whence)
{
  assert(a_);
  assert(offset_);


  A *a = (A *) a_;

  long int offset = (int) *offset_;


//  printf("** seek offset %d", offset ); // value
//  printf(" a->pos %d", a->pos ); // value
                                    // pos should not be negative???

  switch(whence) {
    /*
      SEEK_SET, SEEK_CUR, or SEEK_END, the offset is relative to the start of the
      file, the current position indicator, or end-of-file, respectively

      https://stackoverflow.com/questions/56433377/why-is-the-beginning-of-a-c-file-stream-called-seek-set

      from man fopencookie
      Before returning, the seek function should update *offset to indicate the new stream offset.
      we need to write the offset value. that's why it's passed as a pointer.

      Eg. the glibc library - makes calls to seek using whence = seek_cur and offset = 0. to discover the underlying file offset.
      eg. why see it being called twice in a row.

      and doing this, gets ftell() to work.
    */
    case SEEK_SET:
//      printf(" seek_set, ");
      a->pos = 0 + offset;
      *offset_ = a->pos;
      break;
    case SEEK_CUR:
//      printf(" seek_cur, ");
      a->pos += offset;
      *offset_ = a->pos;
      break;
    case SEEK_END:
//      printf(" seek_end, ");


      assert(0); // jan 2024 remove limitation of sector end
      // a->pos = a->n + offset;
      *offset_ = a->pos;
      break;

    default:
      printf("whence unknown\n");
      assert(0);
  }

//  printf(" a->pos now %d", a->pos ); // value
//  printf("\n" );

  // return 0 on success
  return 0;
}




FILE * flash_open_file(void )
{
  // think fopencookie will copies
  static cookie_io_functions_t  memfile_func = {
    .read  = myread,                  // avoid casting ptrf, because types get confusing
    .write = mywrite,
    .seek  = myseek,
    .close = NULL
  };

  // WARNING A is static.
  static A a;
  memset(&a, 0, sizeof(a));
  a.p = (void *)  FLASH_SECT_ADDR;
  a.pos = 0;
  // a.n = INT_MAX;     // 128 ...
  // a.n = 128 * 1024 ;     // 128 ...

  FILE *f = fopencookie(&a, "r+", memfile_func); // read and write RW !!jjj
  assert(f);


  // printf("f is %p\n", f );
  // printf("_cookie %p\n", &a );

  //
  assert( file_to_cookie(f) == &a );


  return f;
}



#if 0

/*
  don't use these - except for initial flash test
  because they don't use the blob structure and will corrupt
*/

void flash_test_write(void)
{
  /*
    A sector must:w first be fully erased before attempting to program it.
    [in]  sector  (0 - 11 for some parts, 0-23 on others)
    program_size  0 (8-bit), 1 (16-bit), 2 (32-bit), 3 (64-bit)

  */

  /*
    - it is unhnecessary to always erase. instead erase once, then use COW strategy,
    and seek forward until hit 0xff bytes, where can write updated data.
    - looks like interupts might be disabled so flush stream
  */

  // put in a command
  printf("unlock flash\n");
  flash_unlock();

  printf("erasing sector\n");
  usart1_flush();

  flash_erase_sector(FLASH_SECT_NUM, 0 );
  unsigned char buf[] = "whoot";

  printf("writing\n");
  usart1_flush();

  flash_program(FLASH_SECT_ADDR, buf, sizeof(buf) );
  flash_lock();
  printf("done\n");

}


void flash_test_read(void)
{
  char *s = (char *) FLASH_SECT_ADDR;
  printf( "flash char is '%c' %u\n", *s, *s);
  // we expect null terminator
  // what we want is to read...
  if(*s == 'w')
    printf( "string is '%s'\n", s );

}

#endif


