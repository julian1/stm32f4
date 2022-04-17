/*
  todo change name cal.c?
*/


#include <stdio.h>
#include <stdbool.h>
#include <math.h>   // sqrt
// #include <string.h>   // strdup() is unix/posix
// #include <memory.h>   // strdup()

#include "usart.h"  // usart1_flush()

#include "util.h"   // strdup() better place?

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
  skip to end. ie. pos for COW write for nor flash
*/


void file_skip_to_end(  FILE *f)
{
  assert(f );

  printf( "----------------------\n");
  printf( "file skip to end\n");
  usart1_flush();

  // seek the start of file
  fseek( f, 0 , SEEK_SET) ;

  Header header;
  assert(sizeof(header) == 12);

  while(true) {

    // read hader
    unsigned items = fread( &header, sizeof(header), 1, f);
    assert(items == 1);

    // printf("magic is %x\n", header.magic );
    // usart1_flush();

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


static void file_read_cal_values( unsigned id, Cal *cal, FILE *f)
{
  assert(id == 104 || id == 105 || id == 106);


  unsigned items = 0;

  items = fread( &cal->slot, sizeof(cal->slot), 1, f);
  assert(items == 1);

  // now read matrix.
  cal->b = m_finput_binary(f, MNULL );

  items = fread( &cal->param, sizeof(cal->param), 1, f);
  assert(items == 1);

  items = fread( &cal->sigma2, sizeof(cal->sigma2), 1, f);
  assert(items == 1);

  items = fread( &cal->temp,   sizeof(cal->temp), 1, f);
  assert(items == 1);

  if(id == 105 || id == 106) {

    // read the comment len
    unsigned len;
    items = fread( &len, sizeof(len), 1, f);
    assert(items == 1);

    cal->comment = malloc( len + 1);

    items = fread( cal->comment, len, 1, f);
    assert(items == 1);
    cal->comment[ len ] = 0;

    // read the cal id
    items = fread( &cal->id, sizeof(cal->id), 1, f);
    assert(items == 1);
  } else {

    // strdup("") needs a non-null string

    cal->comment = strdup("");
    cal->id = 0;
  }

  if( id == 106) {

    items = fread( &cal->model, sizeof(cal->model), 1, f);
    assert(items == 1);
  } else {

    cal->model = 3;
  }

  // k
}





int file_scan_cal( FILE *f, Cal **cals, unsigned sz, unsigned *cal_id_max )
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

    unsigned here0 = ftell( f);

    // printf("pos %u  got header.id %u len %u magic %x\n", here0, header.id, header.len, header.magic);
    printf("pos %u, magic %x, header.id %u, len %u\n", here0, header.magic, header.id, header.len);

    if(header.magic == MAGIC ) {
      // valid slot

      switch(header.id) {

        case 99:
        case 101:
        case 102:
        case 103:   // some wrongly sized.

          printf("old, ignore\n" );
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

  printf( "done\n" );
  return 0;
}






/*
  Would be better, if could factor out the header advance/stuff from the data.
  change name, file_write_cal_with_header?
*/



static void file_write_cal_( Cal *cal, FILE *f)
{
  // write the slot
  fwrite( &cal->slot, sizeof(int), 1, f);

  // write the matrix
  m_foutput_binary( f, cal->b);

  // write the param
  fwrite( &cal->param, sizeof(cal->param), 1, f);

  fwrite( &cal->sigma2, sizeof(cal->sigma2), 1, f);

  fwrite( &cal->temp, sizeof(cal->temp), 1, f);


  // write comment len
  unsigned len = strlen(  cal->comment );
  fwrite( &len , sizeof(len), 1, f);

  // write comment
  fwrite( cal->comment, len, 1, f); // note. no NULL pad.


  fwrite( &cal->id, sizeof(cal->id), 1, f);


  fwrite( &cal->model, sizeof(cal->model), 1, f);
}











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

  file_write_cal_( cal, f);

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



// TODO change name cal_show() to cal_show()


static void show_slope_b_detail( double slope_b )
{
  // better name show_slope_b_info

  unsigned aperture =   nplc_to_aper_n( 10 );

  // double   slope_b    = m_get_val(cal->b, slope_idx, 0 );   // rows
  double   res        = fabs( slope_b / aperture ); // in V
  // could also work out the implied count here.

  printf("res       %.3fuV  ", res * 1000000);  // resolution  in uV.
  printf("digits %.2f ", log10( 10.f / res));   // ie. decimal=10 not +-11V
  // printf("bits %.2f ", log2( res));           // correct?   or should be aperture / slobe_b ?
  printf("  (nplc10)\n");
}



void cal_show( Cal *cal /* FILE *f */ )
{
  assert(cal);

  printf("--------------\n");

  printf("slot      %u\n", cal->slot );
  // printf("comment   '%s' %u\n", cal->comment, strlen(cal->comment) );
  printf("comment   '%s'\n", cal->comment);
  printf("id        %u\n", cal->id);
  printf("model     %u\n", cal->model);


  printf("b\n");
  m_foutput( stdout, cal->b );

  // only correct with 3 var model.
  // but useful to have
  // should be res(b[idx])

  // want to do the res and digits. for both slope parameters

#if 0
  unsigned slope_idx  = m_rows(cal->b) - 1;
  double   slope_b    = m_get_val(cal->b, slope_idx, 0 );   // rows
  double   res        = fabs( slope_b / nplc_to_aper_n( 10 )); // in V
  // could also work out the implied count here.

  printf("res       %.3fuV  (nplc10)\n", res * 1000000);
  printf("digits    %.2f (nplc10)\n", log10( 10.f / res)); // think needs to be decimal. not 11 or +-11V
#endif

  int slope_idx= m_rows(cal->b) - 1;
  assert( slope_idx - 1 >= 0);

  show_slope_b_detail( m_get_val(cal->b, slope_idx - 1, 0) ); // ref current b
  show_slope_b_detail( m_get_val(cal->b, slope_idx, 0) );     




  param_show(& cal->param);
  printf("\n");

  printf("sigma2    %.2f\n", cal->sigma2);

  double sigma = sqrt( cal->sigma2 );
  printf("sigma     %.2f\n", sigma);


  double sigma_div_aperture = sigma / nplc_to_aper_n( 10 ) * 1000000;  // in uV.
  // printf("sigma_div_aperture %.2fuV  nplc(10)\n", sigma_div_aperture);
  printf("stderr(V) %.2fuV  (nplc10)\n", sigma_div_aperture);

  printf("temp      %.1fC\n", cal->temp );

  printf("\n");
}



Cal * cal_create()
{
  Cal *cal = malloc(sizeof(Cal));
  memset( cal, 0, sizeof(Cal));
  return cal;
}



void cal_free( Cal *cal  )
{
  assert( cal );

  if(cal->b ) {
    M_FREE(cal->b);
  }

  if(cal->comment) {
    free(cal->comment);
    cal->comment = NULL;
  }

  free(cal);
}









Cal * cal_copy( Cal *in )
{
  assert(in);

  Cal * cal = malloc(sizeof(Cal));
  memset(cal, 0, sizeof(Cal));

/*
  // use designator initializer.
  *cal = (Cal const) {
    .slot   = in->slot,
    .b      = m_copy( in->b, MNULL),
    .param  = in->param,
    .sigma2 = in->sigma2,
    .temp   = in->temp,
  };
*/


  cal->slot   = in->slot;
  cal->id     = in->id;       // not sure this is very good....
  cal->comment = strdup( in->comment) ;


  cal->b      = m_copy( in->b, MNULL);
  cal->param  = in->param;
  cal->sigma2 = in->sigma2;
  cal->temp   = in->temp;


  cal->model  = in->model;


  // strdup(). for strings.

  return cal;
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

