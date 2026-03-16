

#include <ctype.h>        // toupper
#include <stdio.h>
#include <assert.h>
#include <math.h>       // fabs
#include <string.h>       // memcpy
#include <stdbool.h>





#include <lib2/util.h>  // UNUSED
#include <lib2/format.h>  // format_float


#include <util.h>         // aper_n_to_nplc

#include <data/data.h>
#include <data/range.h>
#include <data/buffer.h>

// vfd
#include <peripheral/vfd.h> // magic
#include <peripheral/vfd-fonts.h>
#include <display_vfd.h>



static void string_rotate_right( char *s, size_t len,  unsigned shift )
{
  // rotate string right by shift digits
  // correctly handles sentinal/terminating

  if(shift == 0)
    return;

  // string_rotate_right right
  for(unsigned i = len-1; i >= shift; --i) {

    // printf("i %u   i-shift %u \n", i, i - shift );
    assert( i < len);
    assert( (i - shift)  < len);
    s[ i ] = s [ i - shift ];
  }

}


static signed strpos( char *s, size_t len, char ch)
{
  // like, strchr( s, ch) - s ; but no assumption string is null terminated
  signed cur = 0;
  for(unsigned i = 0; i < len && cur == 0 && s[i] != 0 ; ++i) {
    if( s[i] == ch)
      cur = i;
  }
  // return 0 if not found
  return cur;
}



static void string_align_dot( char *s, size_t len, signed pos )
{
  /*
    simpler way to do this - would be with log10().
    eg. prefix_digits = floor(log10( aval)) + 1;
    but it doesn't quite work, since sprintf always adds a leading zero.
    so use string manipulation/interrogation instead
  */

  signed cur = strpos(s, len, '.' );
  assert(cur != 0);

  // determine chars to shift
  signed shift = pos - cur;
  assert( shift >= 0);
  string_rotate_right( s, len,  shift );


  // prepend zeros
  for(unsigned i = 0; i < (unsigned) shift; ++i) {
    assert( i < len);
    s[i] = '0';
  }
}



static void format_value( char *s, size_t n, double value, unsigned leading, unsigned trailing )
{

  // snprintf(s, n, "%.6f", fabs( value ) );
  snprintf(s, n, "%.*f", trailing, fabs(value));

  string_align_dot( s, n, leading);   // eg. -12.xxx

  s[ 0 ] = value >= 0 ? '+' : '-';
}



static char * intersperse_commas(char *dst, size_t sz,  const char *src)
{
  /*
    sz is the dst size buffer.
  */
  char *d = dst;
  const char *s = src;

  bool      gotdot = false;
  unsigned  dotdigits = 0;

  while(*s && d < dst + sz) {

    if( *s == '.')
      gotdot = true;

    if(gotdot && isdigit( (int) *s))
      ++dotdigits;


    *d++ = *s++;

    // eg. comma every third digit
    if( dotdigits != 0
      && (dotdigits % 3) == 0
      && d < dst + sz)
      *d++ = ',';
  }

  // always add a terminal
  if( d < dst + sz)
    *d = 0;
  else
    *(dst + sz - 1)  = 0;


  return dst;
}





static void stoupper( char *s)
{
  // inplace
  size_t n  = strlen(s);
  for(unsigned i = 0; i < n; ++i)   // stoupper
    s[i] = toupper( s[i]);

}







// STTCPW




void display_vfd_init( display_vfd_t *display_vfd, vfd_t *vfd, buffer_t *buffer)
{
  memset( vfd, 0, sizeof( display_vfd_t));
  display_vfd->magic = DISPLAY_VFD_MAGIC;

  assert( buffer && buffer->magic == BUFFER_MAGIC);

  display_vfd->vfd = vfd;
  display_vfd->buffer = buffer;
}










void display_vfd_update( display_vfd_t *display_vfd, data_t *data)
{
  assert( display_vfd && display_vfd->magic == DISPLAY_VFD_MAGIC);
  assert( data && data->magic == DATA_MAGIC);


  vfd_t *vfd = display_vfd->vfd;
  assert( vfd);
  // assert(vfd && vfd->magic == VFD_MAGIC);

  buffer_t *buffer = display_vfd->buffer;
  assert( buffer && buffer->magic == BUFFER_MAGIC);



  // nothing to do
  if( !data->valid)
    return;


  // only have range if data is valid
  range_t *range = data->range;
  assert( range && range->magic == RANGE_MAGIC);



  // feb 2026


  char buf[100 + 1];
  char buf2[100 + 1];


  // from util.  should deprecate
  sprintf( buf, "%s%s",
    str_format_float_with_commas( buf2, 100, 7, data->reading),
    "" // range->unit
  );


  // format_value( buf, 100 - 1, data->reading, 3, 6 );

  vfd_clear( vfd);


  // write value
  vfd_write_bitmap_string2( vfd, buf, 0 , 0 );


/*
  if(status_seq_mode == SEQ_MODE_RATIO)
    printf(" meas %s", str_format_float_with_commas(buf, 100, 7, data->reading));
  else
    printf(" meas %sV", buf );
*/


  // wont work. until we assert valid for a ZERO.
  uint8_t status_sample_idx = data->status.sample_idx;


  // write a star, for the sample
  vfd_write_string2( vfd, (status_sample_idx == 0) ? "x" : "z", 0, 4 );


  // uint8_t status_sample_idx      =  STATUS_SAMPLE_IDX( data->adc_status) ;
  // write a star, for the sample
  // vfd_write_string2( status_sample_idx % 2 == 0 ? "*" : " ", 0, 3 );



  /*
    TODO .  add the noaz. flag. to the status register
    so we can display mode. here. if AZ. or NOAZ.
    ---
    EXTR>  ... putting the 10Meg. flag in the status register as well
      easy if put in cr.
      so it is available in the display.
  */
  bool  noaz = 0;

  double nplc = aper_n_to_nplc( data->adc_clk_count_sigmux, data->line_freq );

  snprintf( buf, 100, "%s-%s %s", range->name, range->arg,  noaz ? "NOAZ" : "AZ");
  vfd_write_string2( vfd, buf, 0, 3 );


  snprintf( buf, 100, "nplc %.1lf ", nplc );
  vfd_write_string2( vfd, buf, 0, 4 );


  // str_format_float_with_commas(buf, 100, 7, data->reading);
  // snprintf( buf, 100, "n %u, mean %f", buffer->count, buffer->mean);
  snprintf( buf, 100, "mean   %.8f", buffer->mean);
  vfd_write_string2( vfd, buf, 0, 5 );

  snprintf( buf, 100, "stddev %.8f", buffer->stddev);
  vfd_write_string2( vfd, buf, 0, 6 );


}




