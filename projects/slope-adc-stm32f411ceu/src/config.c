


#include <stdio.h>
#include <stdbool.h>

#include "usart.h"  // usart_flush()
#include "assert.h"

// #include "matrix.h"
#include "regression.h"

#include "config.h"




struct Header
{
  unsigned magic;
  unsigned len;
  unsigned id;
};

typedef struct Header Header;


#define MAGIC 0xff00ff00

// this is skip to end. good for writing.
// but we need a skip to first valid entry from the end

void c_skip_to_end(  FILE *f)
{
  assert(f );

  printf( "----------------------\n");
  printf( "config skip\n");
  usart_flush();

  // seek the start of file
  fseek( f, 0 , SEEK_SET) ;

  Header header;
  assert(sizeof(header) == 12);


  while(true) {

    // read hader
    unsigned items = fread( &header, sizeof(header), 1, f);
    assert(items == 1);

    printf("magic is %x\n", header.magic );
    usart_flush();

    if(header.magic == MAGIC ) {

      // advance the packet length  and continue
      fseek( f, header.len, SEEK_CUR ) ;
    } else {

      // return to where header would be if there was an entry
      fseek( f, - sizeof(header) , SEEK_CUR ) ;

      break;
    }
  }

  printf( "----------------------\n");
  printf( "ftell() now %ld\n", ftell( f) );

}

// OK. to read different id / slots . and want a switch.










MAT * m_read_flash( MAT *out, FILE *f)
{
  // we could just skip over the header

  // read packet header, and confim a valid entry
  Header  header;
  memset(&header, 0, sizeof(header));

  unsigned items = fread( &header, sizeof(header), 1, f);
  assert(items == 1);
  assert(header.magic == MAGIC );


  // now read.
  MAT *ret = m_finput_binary(f, out );

  // DO NOT CLOSE f.
  return ret ;
}




void m_write_flash ( MAT *m , FILE *f)
{
  assert(f );
  assert(m );

  printf( "m_write_flash f is %p   ftell() is  %ld\n", f, ftell( f) );
  usart_flush();

  assert( sizeof(Header) == 12 );

  // advance 12 bytes, from current position.
  fseek( f, sizeof(Header), SEEK_CUR ) ;
  long start = ftell( f);   // record postion from start.

  // write the matrix
  m_foutput_binary( f, m);

  // determine length
  long len = ftell( f) - start;
  usart_flush();


  printf("len %ld\n", len );
  // seek back to start
  fseek( f, -len - sizeof(Header), SEEK_CUR ) ;    // THIS IS generating a sseek_set from the start ???
                                  // or the meaning is different???

  // write the packet length, as prefix
  Header  header;
  header.magic = MAGIC;
  header.len = len;
  header.id = 99;

  unsigned items = fwrite( &header, sizeof(header), 1, f);
  assert(items == 1);

  // now seek forward again to the end
  fseek( f, len, SEEK_CUR ) ;
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

