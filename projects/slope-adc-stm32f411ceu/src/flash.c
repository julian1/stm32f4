
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


*/


#define _GNU_SOURCE     // must be first. required for cookie_io_functions_t
#include <stdio.h>

#include <limits.h>   // INT_MAX



#include <libopencm3/stm32/flash.h>

// bad conflicts with lib2/include/flash.h
#include "flash.h"

#include "streams.h"  // usart_printf
#include "usart.h"  // usart_flush()




/*
  sector 2.
  needs linker script re-organization.


// globals / or use a structure?
#define FLASH_SECT_ADDR   0x08008000
#define FLASH_SECT_NUM    2
*/


/*
*/
// last 128 . on 512k.
#define FLASH_SECT_ADDR   0x08060000
#define FLASH_SECT_NUM    7


void flash_erase_sector1(void)
{
  flash_erase_sector(FLASH_SECT_NUM, 0 );
}


void flash_write(void)
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
  usart_printf("unlock flash\n");
  flash_unlock();

  usart_printf("erasing sector\n");
  usart_flush();

  flash_erase_sector(FLASH_SECT_NUM, 0 );
  unsigned char buf[] = "whoot";

  usart_printf("writing\n");
  usart_flush();

  flash_program(FLASH_SECT_ADDR, buf, sizeof(buf) );
  flash_lock();
  usart_printf("done\n");

}


void flash_read(void)
{
  char *s = (char *) FLASH_SECT_ADDR;
  usart_printf( "flash char is '%c' %u\n", *s, *s);
  // we expect null terminator
  // what we want is to read...
  if(*s == 'w')
    usart_printf( "string is '%s'\n", s );

}



////////////////////////



#include "assert.h"
// #include "matrix.h"
#include "regression.h"



struct A
{
  unsigned char *p;
  unsigned      pos;
  unsigned      n;    // limit
  // bool          really_write;   // pre
};

typedef struct A A;


static ssize_t mywrite( A *a, const unsigned char *buf, size_t sz)
{
  printf("** mywrite %u\n", sz);
  //printf("a %p\n", a );
  printf("a->pos %d", a->pos ); // value


  // alternatively might be able to return truncated sz...
  assert( a->pos + sz < a->n);

  flash_program( (uint32_t)  a->p + a->pos , buf, sz  );

  a->pos += sz;

  printf("a->pos now %d\n", a->pos ); // value

  return sz;
}


static ssize_t myread(A *a, char *buf, size_t sz)
{
  // sz is just the advertized buffer space.

  /*
    on linux. requests 8192 bytes.
    on stm32 its 1024 bytes.
    this is a buffering action. it is ok to read past the local datastructure.
  */

  printf("** myread %u\n", sz);
  //printf("a %p\n", a );
  printf("a->pos %d\n", a->pos ); // value

  usart_flush();

  int remain = a->n - a->pos;           // signed. but it's not quite correct

  // printf("remaining %u\n", remain );
  // usart_flush();

  if(remain < (int)sz)
    sz = remain;

  assert(remain >= 0);

  // printf("sz now %u\n", sz );
  // usart_flush();

  memcpy(buf, a->p + a->pos, sz);
  a->pos += sz;

  return sz;
}



static int myseek(A *a, _off64_t *offset_, int whence)
{
  //  int seek(void *cookie, off64_t *offset, int whence);

  // only lower 4 bytes on stm32.
  int offset = (int) *offset_;

  // this isn't looking right

  printf("** seek offset %d", offset ); // value
  // printf("** seek offset %lld", *offset_); // value


  // printf(" a %p", a );
  printf(" a->pos %d", a->pos ); // value
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
      printf(" seek_set, ");
      a->pos = 0 + offset;
      *offset_ = a->pos;
      break;
    case SEEK_CUR:
      printf(" seek_cur, ");
      a->pos += offset;
      *offset_ = a->pos;
      break;
    case SEEK_END:
      printf(" seek_end, ");
      a->pos = a->n + offset;
      *offset_ = a->pos;
      break;

    default:
      printf("whence unknown\n");
      assert(0);
  }

  printf(" a->pos now %d", a->pos ); // value
  printf("\n" );

  // return 0 on success
  return 0;
}




FILE * open_flash_file(void )
{
  // think fopencookie will copies
  static cookie_io_functions_t  memfile_func = {
    .read  = (cookie_read_function_t *) myread,
    .write = (cookie_write_function_t *) mywrite,
    .seek  = (cookie_seek_function_t *) myseek,
    .close = NULL
  };

  // WARNING A is static.
  static A a;
  memset(&a, 0, sizeof(a));
  a.p = (void *)  FLASH_SECT_ADDR;
  a.pos = 0;
  // a.n = INT_MAX;     // 128 ...
  a.n = 128 * 1024 ;     // 128 ...

  FILE *f = fopencookie(&a, "r+", memfile_func); // read and write RW !!jjj
  assert(f);


  printf("f is %p\n", f );
  printf("_cookie %p\n", &a );


  return f;
}




MAT * m_read_flash( MAT *out, FILE *f)
{
  unsigned len = 0;
  unsigned magic = 0;
  unsigned items;

  items = fread( &magic, sizeof(magic), 1, f);
  printf("magic is %x\n", magic );
  usart_flush();
  assert(items == 1);

  assert(magic == 0xff00ff00);
  items = fread( &len, sizeof(len), 1, f);
  printf("len is %u\n", len );
  usart_flush();
  assert(items == 1);


  MAT *ret = m_finput_binary(f, out );

  // DO NOT CLOSE f.
  return ret ;
}




#if 0
void m_write_flash ( MAT *m , FILE *f)
{
  assert(f );
  assert(m );
  printf( "m_write_flash f is %p\n", f);
  usart_flush();


  // write the packet length, as prefix
  unsigned magic = 0xff00ff00;
  fwrite( &magic, sizeof(magic), 1, f);
  unsigned len = 168;
  fwrite( &len, sizeof(len), 1, f);


  // write the file
  m_foutput_binary( f, m);

}

#endif

/*
  Gahhhh.... I think it's buffering really heavily.
  so that requests for ftell(). are not correct.

  need flush().
  trying to use offset - may not be goojbd strategy.
  having a dummy writer to get length .

*/


#if 1
void m_write_flash ( MAT *m , FILE *f)
{
  assert(f );
  assert(m );

  printf( "m_write_flash f is %p\n", f);
  usart_flush();

  // do a seek_set. to eliminate buffering, from a previous read.
  // fseek( f, 0 , SEEK_SET) ;
  // fflush(f);

  // advance 8 bytes, from current position.
  fseek( f, 8 , SEEK_CUR ) ;
  long start = ftell( f);   // record postion from start.

  // write the file
  m_foutput_binary( f, m);

  // fseek( f, 0 , SEEK_CUR ) ;
  long len = ftell( f) - start;
  usart_flush();

  // fflush(f);

  printf("here0 \n" );
  printf("len %ld\n", len );
  // seeks bacikkk
  fseek( f, -len - 8, SEEK_CUR ) ;    // THIS IS generating a sseek_set from the start ???
                                  // or the meaning is different???

  // write the packet length, as prefix
  unsigned magic = 0xff00ff00;
  fwrite( &magic, sizeof(magic), 1, f);
  fwrite( &len, sizeof(len), 1, f);


  fseek( f, len  , SEEK_CUR ) ;
}
#endif



