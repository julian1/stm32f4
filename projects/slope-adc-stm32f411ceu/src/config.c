


#include <stdio.h>

#include "usart.h"  // usart_flush()
#include "assert.h"

// #include "matrix.h"
#include "regression.h"

#include "config.h"




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



