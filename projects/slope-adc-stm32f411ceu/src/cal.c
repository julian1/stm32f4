/*
  todo change name cal.c?
*/


#include <stdio.h>
#include <stdbool.h>

#include "usart.h"  // usart1_flush()
#include "assert.h"

// #include "matrix.h"
#include "regression.h"

#include "cal.h"




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


int file_scan_cal( FILE *f, Cal **cals, unsigned sz )
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
          MAT *b = m_finput_binary(f, MNULL );
          printf("matrix is\n" );
          m_foutput( stdout, b );

          printf("setting slot %u with matrix\n", slot );
          assert(slot < sz);

          Cal * cal = malloc(sizeof(Cal));
          memset(cal, 0, sizeof(Cal));
          cal->slot = slot;
          cal->b    = b;
          cals[ slot ] = cal;

          assert( here0  + header.len == ftell( f));
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
  Would be better, if could factor out the header advance/stuff from the data.
  change name, file_write_cal_with_header?
*/


void file_write_cal ( Cal *cal, FILE *f)
{
  assert(f );
  assert(cal);

  printf( "-----------------\n" );
  printf( "file_write_cal f is %p   ftell() is  %ld\n", f, ftell( f) );
  usart1_flush();

  assert( sizeof(Header) == 12 );

  // advance over expected header we will write later.
  fseek( f, sizeof(Header), SEEK_CUR ) ;
  long start = ftell( f);   // record postion from start.

  /////////////////////
  // write cal data

  // write the slot
  fwrite( &cal->slot, sizeof(int), 1, f);

  // write the matrix
  m_foutput_binary( f, cal->b);

  // determine how much we advanced
  long len = ftell( f) - start;
  usart1_flush();

  printf("len %ld\n", len );
  // seek back to header
  fseek( f, -len - sizeof(Header), SEEK_CUR );

  // write the header
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





void cal_report( Cal *cal /* FILE *f */ )
{
  assert(cal);

  printf("cal \n");
  printf("slot %u\n", cal->slot );

  m_foutput( stdout, cal->b );

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
void file_write_cal ( MAT *m , FILE *f)
{
  assert(f );
  assert(m );
  printf( "file_write_cal f is %p\n", f);
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

