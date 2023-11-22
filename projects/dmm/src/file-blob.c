


#include <stdio.h>      // FILE
#include <stdbool.h>    // true
#include <assert.h>

#include "usart.h"  // usart1_flush()



#include "file-blob.h"


#define MAGIC 0xff00ff00

/*
  skip to end. ie. pos for COW write for nor flash
  end of blobs
*/


void file_blob_skip_end( FILE *f)
{
  // this func goes here. because it uses the Header structure
  assert(f );

  printf( "----------------------\n");
  printf( "file_skip_to_end()\n");
  usart1_flush();

  // seek the start of file

  printf("calling fseek\n");
  usart1_flush();
  fseek( f, 0 , SEEK_SET) ;

  printf("done fseek\n");
  usart1_flush();


  Header header;
  assert(sizeof(header) == 12);

  while(true) {

    // read hader
    printf("seek read header\n");
    usart1_flush();
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



/*
  OK. the fundamental issue is that we don't know the size, before write.
    because the stream interface is much more convenient.

    which makes it a bit harder to abstract tof the data we wish to write
  .  eg. the b matrix.
  But we can skip.

*/

void file_blob_write( FILE *f,    void (*pf)( FILE *, void *ctx ), void *ctx )
{
  assert(f );
  assert(pf);

  printf( "-----------------\n" );
  printf( "file_write_data f is %p   ftell() is  %ld\n", f, ftell( f) );


  usart1_flush();

  assert( sizeof(Header) == 12 );

  // advance past header that we will write later.
  fseek( f, sizeof(Header), SEEK_CUR ) ;
  long start = ftell( f);   // record postion from start.

  /////////////////////

  // write the blob data
  // we should return the header id that we want to use also.
  pf( f, ctx);

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
  header.id = 106;     // header id. for raw matrix.

  unsigned items = fwrite( &header, sizeof(header), 1, f);
  assert(items == 1);

  // now seek forward again to the end
  fseek( f, len, SEEK_CUR ) ;


  printf( "ftell() now %ld\n", ftell( f) );
}



int file_blobs_scan( FILE *f,  void (*pf)( FILE *f, Header *, void *ctx ), void *ctx )
{
  // return 0 if success.

  assert(f );

  printf( "----------------------\n");
  printf( "file_scan_cal()\n");
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

    unsigned here0 = ftell( f);

    // printf("pos %u, magic %x, header.id %u, len %u\n", here0, header.magic, header.id, header.len);
    printf(".");


    if(header.magic == MAGIC ) {
      // valid slot


      pf( f, &header, ctx );


      // position to the next frame
      fseek( f, here0 + header.len, SEEK_SET ) ;

    }
    else if( header.magic == 0xffffffff ) {
      // nor ram not yet written
      break;
    }
    else {
      // error
      assert( 0);
    }
  }


  // reset seek the start of file
  fseek( f, 0 , SEEK_SET) ;

  printf( "\ndone\n" );
  return 0;
}






#if 0
      switch(header.id) {

        case 99:
        case 101:
        case 102:
        case 103:   // some wrongly sized.

          // printf("old, ignore\n" );
          // fseek( f, header.len, SEEK_CUR ) ;
          break;

        case 104:
          if(header.len == 78) // fix to ignore cal type 105, that was saved as 104.
            break;
          // allow fallthrough

        case 105:
          if(header.len == 97) // fix to ignore cal type 105, that was saved as 104.
            break;

        case 106:
          {

          // printf("reading cal type 104,105,106\n" );

          Cal * cal = cal_create();
          file_read_cal_values( header.id, cal, f);

          // actually should be the cal_id_max = MAX(id cal_id_count)
          // cal_id_count
          *cal_id_max =  MAX( *cal_id_max , cal->id );

          // bounds
          assert( cal->slot < sz);

          // free old if exists
          if(cals[ cal->slot ] )
            cal_free( cals[ cal->slot ] );

          // set new
          cals[ cal->slot ] = cal;

          assert( here0  + header.len == (unsigned) ftell( f));
          }
          break;



      };
#endif




