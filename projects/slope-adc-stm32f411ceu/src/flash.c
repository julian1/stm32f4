
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

  flash_program(   a->p + a->pos , buf, sz  );

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

      OK. i think seek_cur - sets a stateful position.
      then set_seek returns to it.
     
      so set seek. should be called at start. then when we o
      or the other way.

      seek_cur - sets the cursor. and seek_set returns to it.

      https://stackoverflow.com/questions/56433377/why-is-the-beginning-of-a-c-file-stream-called-seek-set 
    */
    case SEEK_SET:
      printf(" seek_set, ");
      a->pos = 0 + offset;
      break;
    case SEEK_CUR:
      printf(" seek_cur, ");
      a->pos += offset;
      break;
    case SEEK_END:
      printf(" seek_end, ");
      // assert(0);
      // a->pos = a->n - offset; // negative ???
      a->pos = a->n + offset; // or positive?. eg. arg will be negative?
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
  a.p =  FLASH_SECT_ADDR;
  a.pos = 0;
  // a.n = INT_MAX;     // 128 ...
  a.n = 128 * 1024 ;     // 128 ...

  FILE *f = fopencookie(&a, "r+", memfile_func); // read and write RW !!jjj
  assert(f);


  printf("f is %p\n", f );
  printf("_cookie %p\n", &a );


  return f;
}


long ftell2( FILE *f)
{
  assert(f);

  // actually a handler
  void *cookie_ptr= f->_cookie;
  // printf("cookie_ptr %p\n", cookie_ptr);

   // follow it
   void *cookie = * (void **) cookie_ptr ;
   // printf ("cookie %p\n",  cookie );
   A *a = cookie;
  assert(a);

  return a->pos;

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
  
  fflush(f);

  printf("here0 \n" );
  printf("len %ld\n", len );
  // seeks bacikkk
  fseek( f, - len - 8 , SEEK_CUR ) ;    // THIS IS generating a sseek_set from the start ??? 
                                        // why?
  printf("here1 \n" );

  /* 
    No, it means "set to the specified position". Which may be or not be the beginning of the stream. – 
    Eugene Sh.
    Jun 3, 2019 at 19:21
    -  I think we shouldn't be calling it.
  */


  // write the packet length, as prefix
  unsigned magic = 0xff00ff00;
  fwrite( &magic, sizeof(magic), 1, f);
  fwrite( &len, sizeof(len), 1, f);


  // fseek( f, len  , SEEK_CUR ) ;
}
#endif






/*
  printf("here0 \n" );
  usart_flush();

  // we should be setting up the structure once.
  // not doing it past the

  cookie_io_functions_t  memfile_func = {
    .read  = (cookie_read_function_t *) myread, // read
    .write = (cookie_write_function_t *) mywrite,
    .seek  = NULL,
    .close = NULL
  };


  A a;
  memset(&a, 0, sizeof(a));
  a.p = a.p = FLASH_SECT_ADDR;
  a.pos = 0;
  a.n = INT_MAX;     // 128 ...


  // this is all wrong. we should not be setting len.
  // when we have the length... we should
  // it grabs 10k.
  // that is not really a problem


  // open
  FILE *f = fopencookie(&a, "r", memfile_func);
  assert(f);
  // a.n = 1000;
*/


//////////////////////////////////////


// TO write  get the size of this we are going to have

/*
  simpler approach.
  skip forward 8 bytes.
  tag position
  write structure.
  then skip back
*/


/*
210         printf("fp %p\n", f );
211
212         for(unsigned i = 0; i < 10 ; ++i ) {
213           printf("%u  %p\n", i, ((void **)f) [ i]  );
214         }
215
216           printf("========= 0x200017a8 \n"  );
217         for(unsigned i = 0; i < 5; ++i ) {
218           printf("%u  %p\n", i, ((void **)    0x200017a8 ) [ i]  );
219         }
220
221           // 7th field element
222           void *cookie_ptr= ((void **)f)[ 7 ];
223           printf("cookie_ptr %p\n", cookie_ptr);
224
225          // follow it
226          void *cookie = * (void **) cookie_ptr ;
227          printf ("cookie %p\n",  cookie );
228
*/



#if 0
void m_write_flash ( MAT *m )
{

  cookie_io_functions_t  memfile_func = {
    .read  = (cookie_read_function_t *)   myread, // read
    .write = (cookie_write_function_t *)  mywrite,
    .seek  = (cookie_seek_function_t *) myseek,
    .close = NULL
  };

  // char buf[1000];
  // memset(buf, 0, sizeof(buf));
  A a;
  memset(&a, 0, sizeof(a));
  a.p = (unsigned char *)FLASH_SECT_ADDR;
  a.n = INT_MAX ;       // signed. 128k.   // unlimimited, when writing

  // write matrix, without output, to determine size
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

  // now write matrix
  m_foutput_binary( f, m);
  fclose(f);
}
#endif




// how hard to skip... through.
// have to do reads

// We actually kind of buffer

/*
  we kind of want the ability to seek around.
  - which means persisting the A structure.
  could be a cookie.
  - and i think using a different structure to fake to size calculation.
  - fseeking over the structure would make everything a lot simpler.
  - open it once.
*/






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


/*
  glibc source
    https://elixir.bootlin.com/glibc/glibc-2.27/source/libio/iofopncook.c
*/
