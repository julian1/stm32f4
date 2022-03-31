/*
  todo change name cal.c?
*/


#include <stdio.h>
#include <stdbool.h>

#include "usart.h"  // usart1_flush()
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

/*
  skip to end. to find next pos for data write
*/


void file_skip_to_end(  FILE *f)
{
  assert(f );

  printf( "----------------------\n");
  printf( "config skip to end\n");
  usart1_flush();

  // seek the start of file
  fseek( f, 0 , SEEK_SET) ;

  Header header;
  assert(sizeof(header) == 12);

  while(true) {

    // read hader
    unsigned items = fread( &header, sizeof(header), 1, f);
    assert(items == 1);

    printf("magic is %x\n", header.magic );
    usart1_flush();

    if(header.magic == MAGIC ) {

      // another header, so skip the packet length and continue
      fseek( f, header.len, SEEK_CUR ) ;
    }
    else if( header.magic == 0xffffffff ) {
      // uninitialized nor ram.
      // move to where header would be if there was an entry
      fseek( f, - sizeof(header) , SEEK_CUR ) ;
      break;
    }
    else {
      // error
      assert( 0);
    }
  }

  printf( "ftell() now %ld\n", ftell( f) );
}




// OK. we want a variation. of file_skip. that fills in data... according to header ids.

int file_scan( FILE *f, MAT **b, unsigned b_sz )
{
  // return 0 if success.

  assert(f );

  printf( "----------------------\n");
  printf( "config scan flash\n");
  usart1_flush();

  // seek the start of file
  fseek( f, 0 , SEEK_SET) ;

  Header header;
  assert(sizeof(header) == 12);

  while(true) {

    // read header
    unsigned items = fread( &header, sizeof(header), 1, f);
    assert(items == 1);

    // printf("magic is %x\n", header.magic );
    // usart1_flush();

    unsigned here0 = ftell( f); // position from start.
    printf("here0 is %d\n", here0 );

    if(header.magic == MAGIC ) {
      // valid slot

      switch(header.id) {

        case 99:
          printf("got 99 a matrix, skipping\n" );
          // fseek( f, header.len, SEEK_CUR ) ;
          break;

        case 101:
          printf("got 101 a matrix and slot\n" );

          unsigned slot;
          items = fread( &slot, sizeof(slot), 1, f);
          assert(items == 1);

          printf("slot is %d\n", slot );

          // now read matrix.
          MAT *m = m_finput_binary(f, MNULL );
          printf("matrix is\n" );
          m_foutput( stdout, m );


          printf("setting slot %u with matrix\n", slot );
          assert(slot < b_sz);
          b[ slot ] = m ;


          unsigned here1 = ftell( f); // position from start.
          // printf("here is %d\n", here1 );
          assert( here0  + header.len == here1 );
          break;
      };

      // position to the next frame
      fseek( f, here0 + header.len, SEEK_SET ) ;

    }
    else if( header.magic == 0xffffffff ) {

      break;
    }
    else {
      // error
      assert( 0);
    }
  }


  // reset seek the start of file
  fseek( f, 0 , SEEK_SET) ;

  printf( "done\n" );
  return 0;
}






/*
  This should be factored... to be able to write any kind of packet.
  Issue is we don't know the size up-front.

  So pass a function/ ctx ...

*/

void m_write_flash ( MAT *m , int slot, FILE *f)
{
  assert(f );
  assert(m );

  printf( "-----------------\n" );
  printf( "m_write_flash f is %p   ftell() is  %ld\n", f, ftell( f) );
  usart1_flush();

  assert( sizeof(Header) == 12 );

  // advance 12 bytes, from current position.
  fseek( f, sizeof(Header), SEEK_CUR ) ;
  long start = ftell( f);   // record postion from start.


  // write the slot
  fwrite( &slot, sizeof(int), 1, f);
  // write the matrix
  m_foutput_binary( f, m);


  // determine length
  long len = ftell( f) - start;
  usart1_flush();


  printf("len %ld\n", len );
  // seek back to start
  fseek( f, -len - sizeof(Header), SEEK_CUR ) ;    // THIS IS generating a sseek_set from the start ???
                                  // or the meaning is different???

  // write the packet length, as prefix
  Header  header;
  header.magic = MAGIC;
  header.len = len;
  header.id = 101;     // header id. for raw matrix.

  unsigned items = fwrite( &header, sizeof(header), 1, f);
  assert(items == 1);

  // now seek forward again to the end
  fseek( f, len, SEEK_CUR ) ;


  printf( "ftell() now %ld\n", ftell( f) );

}














#if 0
MAT * m_read_flash( MAT *out, FILE *f)
{
  // we could just skip over the header
  printf( "-----------------\n" );
  printf( "m_read_flash\n" );

  // read packet header, and confim a valid entry
  Header  header;
  memset(&header, 0, sizeof(header));

  unsigned items = fread( &header, sizeof(header), 1, f);
  assert(items == 1);
  assert(header.magic == MAGIC );


  // now read matrix.
  MAT *ret = m_finput_binary(f, out );

  // DO NOT CLOSE f.
  return ret ;
}
#endif

#if 0
void m_write_flash ( MAT *m , FILE *f)
{
  assert(f );
  assert(m );
  printf( "m_write_flash f is %p\n", f);
  usart1_flush();


  // write the packet length, as prefix
  unsigned magic = 0xff00ff00;
  fwrite( &magic, sizeof(magic), 1, f);
  unsigned len = 168;
  fwrite( &len, sizeof(len), 1, f);


  // write the file
  m_foutput_binary( f, m);

}

#endif




#if 0
/*
  // OK. to read different id / slots . and want a switch.
  can take last valid, with fast seek.
  OR. can play forward, and free old, and use new

  - simplest test implemetnation is c_last_valid_entry

*/


int file_skip_to_last_valid(  FILE *f)
{
  // return 0 if success.

  assert(f );

  printf( "----------------------\n");
  printf( "config skip to last valid\n");
  usart1_flush();

  // seek the start of file
  fseek( f, 0 , SEEK_SET) ;

  Header header;
  assert(sizeof(header) == 12);

  unsigned last_len = 0;

  while(true) {

    // read header
    unsigned items = fread( &header, sizeof(header), 1, f);
    assert(items == 1);

    printf("magic is %x\n", header.magic );
    usart1_flush();

    if(header.magic == MAGIC ) {

      // valid slot
      last_len = header.len;
      // seek next header, so skip the packet length and continue
      fseek( f, header.len, SEEK_CUR ) ;
    }
    else if( header.magic == 0xffffffff ) {
      // uninitialized nor ram.
      // go back 12 bytes for attempted header read, and the len of last packet, and the header for that packet.
      fseek( f, -last_len -sizeof(header) -sizeof(header), SEEK_CUR ) ;

      // could be negative here.
      if(ftell( f) < 0) {
        printf("cannot skip to valid packet, no valid packets found\n" );
        // assert( 0 ) ; // we tried to read without any valid packet
        return -123;
      }

      break;
    }
    else {
      // error
      assert( 0);
    }
  }

  printf( "ftell() now %ld\n", ftell( f) );
  return 0;
}

#endif

