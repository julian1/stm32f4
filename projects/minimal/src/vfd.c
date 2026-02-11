

#include <ctype.h>        // toupper
#include <stdio.h>
#include <assert.h>
#include <math.h>       // fabs



// vfd
#include <peripheral/vfd.h>
#include <vfd.h>


#include <data/data.h>
#include <util.h>

#include <lib2/format.h>  // format_float



#include <device/spi-fpga0-reg.h>    // TODO REMOVE. does not belong here. for seq mode


#define UNUSED(x) ((void)(x))




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


void vfd_update_new_reading(data_t *data)
{
  assert(data);
  assert(data->magic == DATA_MAGIC);


  char buf[100];

  // from util.  should deprecate
  str_format_float_with_commas(buf, 100, 7, data->computed_val);


  // format_value( buf, 100 - 1, data->computed_val, 3, 6 );

  // write value
  vfd_write_bitmap_string2( buf, 0 , 0 );


/*
  if(status_seq_mode == SEQ_MODE_RATIO)
    printf(" meas %s", str_format_float_with_commas(buf, 100, 7, data->computed_val));
  else
    printf(" meas %sV", buf );
*/



#if 0
  uint8_t status_sample_idx      =  STATUS_SAMPLE_IDX( data->adc_status) ;

  uint8_t status_seq_mode =  STATUS_SAMPLE_SEQ_MODE( data->adc_status);

  // write a star, for the sample
  vfd_write_string2( status_sample_idx % 2 == 0 ? "*" : " ", 0, 3 );

  // write mode
  seq_mode_str( status_seq_mode, buf, 8 );
  stoupper( buf);
  vfd_write_string2( buf, 0, 4 );
#endif


  // write nplc
  double nplc = aper_n_to_nplc( data->adc_clk_count_sigmux, data->line_freq );
  snprintf(buf, 100, "nplc %.1lf ", nplc );
  vfd_write_string2( buf, 0, 5 );


  // write/ dummy other stuff.
  snprintf(buf, 100, "DCV 10M" );
  vfd_write_string2( buf, 0, 6 );


}




