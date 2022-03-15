
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
  bool          really_write;   // pre
};

typedef struct A A;

static ssize_t mywrite( A *a, const unsigned char *buf, size_t sz)
{
  printf("mywrite %u\n", sz);
  // alternatively might be able to return truncated sz...
  assert( a->pos + sz < a->n);

  if(a->really_write) {
    flash_program(  a->p + a->pos , buf, sz  );
    // memcpy(a->p + a->pos, buf, sz);
  }
  a->pos += sz;
  return sz;
}


static ssize_t myread(A *a, char *buf, size_t sz)
{
  // sz is just the advertized buffer space.
  printf("myread %u\n", sz);
  usart_flush(); 

  int remain = a->n - a->pos;           // signed. but it's not quite correct

  printf("remaining %u\n", remain );
  usart_flush(); 

  if(remain < sz)
    sz = remain;


  printf("sz now %u\n", sz );
  usart_flush(); 

  memcpy(buf, a->p + a->pos, sz);
  a->pos += sz;

  return sz;
}

#if 0
static int myseek(A *a, off64_t *offset, int whence)
{
  printf("**********\n" );
  printf("seek called \n");
  printf("whence %u\n", whence);
  assert(0);
  return 0;
}

static int myclose(A * a)
{
  printf("close called \n");
  // potentially would be useful.
  //a-> pos = 0;  // reseek to start   (should do this in fclose).
  return 0; // success
}
#endif



/* 
  might be less complicated to write into a temporary buffer. 
  then write 

  And work with the A structure. 
*/





void m_write_flash ( MAT *m )
{

  cookie_io_functions_t  memfile_func = {
    .read  = (cookie_read_function_t *) myread, // read
    .write = (cookie_write_function_t *) mywrite,
    .seek  = NULL,
    .close = NULL
  };

  // char buf[1000];
  // memset(buf, 0, sizeof(buf));
  A a;
  memset(&a, 0, sizeof(a));
  a.p = FLASH_SECT_ADDR;
  a.n = 0xffffffff; // unlimimited, when writing

  // write once to determine the size
  FILE *f = fopencookie(&a, "w", memfile_func);
  assert(f);
  a.pos = 0;
  a.really_write = false;
  m_foutput_binary( f, m);
  fclose(f);

  // set the size
  unsigned len = a.pos;
  unsigned magic = 0xff00ff00;
  printf("write len is %u\n", len );


  // reopen and write with magic and len
  f = fopencookie(&a, "w", memfile_func);
  assert(f);
  a.pos = 0;
  a.really_write = true;

  // write the packet length, as prefix
  fwrite( &magic, sizeof(magic), 1, f);
  fwrite( &len, sizeof(len), 1, f);
  // write for real
  m_foutput_binary( f, m);
  fclose(f);

}


MAT * m_read_flash( MAT *out)
{
  printf("here0 \n" );
  usart_flush();

  cookie_io_functions_t  memfile_func = {
    .read  = (cookie_read_function_t *) myread, // read
    .write = (cookie_write_function_t *) mywrite,
    .seek  = NULL,
    .close = NULL
  };


  A a;
  memset(&a, 0, sizeof(a));
  a.p = a.p = FLASH_SECT_ADDR;
  a.n = 0xffffffff; // just to read the magic number and length
  // a.n = 8;  // asuume just enough to read the magic number and length
                  // fails...

  // read
  FILE *f = fopencookie(&a, "r", memfile_func);
  assert(f);
  // a.n = 1000;
  a.pos = 0;

  unsigned len = 0;
  unsigned magic = 0;
  unsigned items;

  items = fread( &magic, sizeof(magic), 1, f);
  printf("magic is %u\n", magic );
  usart_flush();
  assert(items == 1);


  assert(magic == 0xff00ff00);
  items = fread( &len, sizeof(len), 1, f);
  printf("len is %u\n", len );
  usart_flush();
  assert(items == 1);

  // set to the written buffer size
  a.n = len;
  a.pos = 0;

  MAT *ret = m_finput_binary(f, out );
  fclose(f);

  return ret ;

}




