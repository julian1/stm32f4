
// consider rename  support.h


#include <stdio.h>
#include <assert.h>
#include <string.h>   // strcmp()
// #include <strings.h>   // strcasecmp().  use to_lower() instead
#include <stdlib.h> // strtoul()
#include <ctype.h>    // isdigit isspace
#include <math.h>    // fabs()


#include <support.h>
#include <mode.h> // needed for S1-S8 etc. for string decoding



////////////////////
// consider move to another file
// consider rename aper_n to just aper or aperture


uint32_t nplc_to_aperture( double nplc, uint32_t line_freq )
{
  assert( line_freq);

  double period = nplc / (double) line_freq;  // seonds
  uint32_t aper = period * CLK_FREQ;
  return aper;
}


double aper_n_to_nplc( uint32_t aper_n, uint32_t line_freq)
{
  assert( line_freq);

  double period   = aper_n / (double ) CLK_FREQ;          // use aper_n_to_period()
  double nplc     = period * (double) line_freq;
  return nplc;
}


double aper_n_to_period( uint32_t aper_n)
{
  double period   = aper_n / (double ) CLK_FREQ;
  return period;
}


uint32_t period_to_aper_n(  double period )
{
  return period * CLK_FREQ;
}


bool nplc_valid( double nplc )
{
  /*
    used in a few places to validate user input
    can be relaxed later.
    maybe use a switch/case
    todo - similar to validate/check  voltage source
  */
/*
  return
     nplc == 0.1 || nplc == 0.5 || nplc == 1
    || nplc == 2 || nplc == 10 || nplc == 100 || nplc == 1000;
*/
  return nplc >= 0.1 && nplc <= 100;

}


void aper_cc_print( /* FILE */ uint32_t aperture,  uint32_t line_freq)
{

//   uint32_t aperture = nplc_to_aperture( f0, app->line_freq );
  printf("aperture %lu\n",   aperture );
  printf("nplc     %.2lf\n",  aper_n_to_nplc( aperture, line_freq ));
  printf("period   %.2lfs\n", aper_n_to_period( aperture ));
}



char * mux_to_string( unsigned val,  char *buf, unsigned n  )
{
  // change name mux_to_str() at least.
  // this is 1ofN mux.
  char *s = 0;

  switch(val) {
    case SOFF: s = "soff";  break;
    case S1 : s = "s1";  break;
    case S2 : s = "s2";  break;
    case S3 : s = "s3";  break;
    case S4 : s = "s4";  break;
    case S5 : s = "s5";  break;
    case S6 : s = "s6";  break;
    case S7 : s = "s7";  break;
    case S8 : s = "s8";  break;
    default: assert(0);
  };

  strncpy(buf, s, n);
  return buf;
}



#if 0
static void set_seq( uint32_t *val,  uint32_t pc, uint32_t azmux)
{
      // would be handy to have in a function. or else return
      //uint32_t val =  ((u0 & 0b11) << 4) | ( u1 & 0b1111);
  assert(pc <= 0b11);
  assert(azmux <= 0b1111);

  // *val =  ((u0 & 0b11) << 4) | ( u1 & 0b1111);
  *val =  pc << 4 |  azmux ;
}

#endif




/*
  use strcasecmp() from strings.h.
  or just force lower case first?

*/


unsigned str_decode_uint( const char *s, uint32_t *val  )
{
  // decode int literal
  // set/reset  for relay.


  // reset == default position in the schematic.

  if (strcmp(s, "on") == 0
    || strcmp(s, "set") == 0
    || strcmp(s, "true") == 0)
    *val = 1;

  else if(strcmp(s, "off") == 0
    || strcmp(s, "reset") == 0
    || strcmp(s, "false") == 0)
    *val = 0;


  /*
    Can probably use this for adg1209. also - by puting the EN pin on 4th bit.
    eg. easy to remap in fpga.  but not for 4094.
    this allows az-mux. to usse either type of analog switch - 1x08 instead of 2x04 mux.
  */

  // 1 of 8 mux values.
  else if(strcmp(s, "s8") == 0 )
    *val = S8;
  else if(strcmp(s, "s7") == 0 )
    *val = S7;
  else if(strcmp(s, "s6") == 0 )
    *val = S6;
  else if(strcmp(s, "s5") == 0 )
    *val = S5;
  else if(strcmp(s, "s4") == 0 )
    *val = S4;
  else if(strcmp(s, "s3") == 0 )
    *val = S3;
  else if(strcmp(s, "s2") == 0 )
    *val = S2;
  else if(strcmp(s, "s1") == 0 )
    *val = S1;
  else if(strcmp(s, "soff") == 0 )
    *val = SOFF;


#if 0
  // 2 of 4 mux values
  else if(strcmp(s, "d4") == 0 )
    *val = D4;
  else if(strcmp(s, "d3") == 0 )
    *val = D3;
  else if(strcmp(s, "d2") == 0 )
    *val = D2;
  else if(strcmp(s, "d1") == 0 )
    *val = D1;
  else if(strcmp(s, "doff") == 0 )
    *val = DOFF;
#endif



  // we could factor all this handling.
  // read_int.
  else if( s[0] == '0' && s[1] == 'x' && sscanf(s, "%lx", val) == 1) {
    // printf("got hex\n" );
  }
  else if( s[0] == '0' && s[1] == 'o' && sscanf(s + 2, "%lo", val) == 1) {
    // for octal, sscanf doesn't like/accept a prefix
    // printf("got octal\n" );
  }
  else if( s[0] == '0' && s[1] == 'b') {
    // binary is very useful for muxes
    *val = strtoul(s + 2, NULL, 2);
    // char buf[100];
    // printf("got binary %s\n", format_bits(buf, 32, val ) );
  }
  else if( isdigit( (unsigned char) s[0] ) && sscanf(s, "%lu", val) == 1) {
    // printf("got decimal\n" );
  }

  else {
    printf("bad val arg\n" );
    return 0;   // fail
  }

  // OK.
  return 1 ;
}



unsigned str_decode_int( const char *s, int32_t *val  )
{
  // it's useful to express a negative hex value for mdac.
  assert(s);

  bool dash = false;
  if(*s == '-') {
    ++s;
    dash = true;
  }

  unsigned ret = str_decode_uint( s, (uint32_t *)val);

  if(dash)
    *val *= -1;

  return ret;
}







unsigned str_decode_float( const char *s, double *val )
{

  // handle unit.

  char unit;

  unsigned n = sscanf(s, "%lf%c", val, &unit);

  if(n == 1) {
    return 1;
  }
  else if (n == 2) {

    switch( unit) {

      case 'm':
        *val *= 1e-3;  break;
      case 'u':
        *val *= 1e-6; break;
      case 'n':
        *val *= 1e-9;  break;

      default:
        return 0;     // eg. conversion failed.
    }
    return 1;
  }


  return 0;

}





/*
  this code wont work for ranges
  but is useful for some dynamic values where we don't know how to manage.
  leading digits

*/

#if 0
char * str_format_value( char *s, size_t n,  unsigned ndigits, unsigned leading, double value)
{
  // printf("%u\n", leading);

  if( ndigits < leading) {
    assert( 0);
  }
  else if( ndigits == leading) {
    // +1 for sign. but there will be no dot
    ndigits += 1;
    leading += 1;
  }
  else {
    // +1 for sign and +1 for dot
    ndigits += 2;
    leading += 2;
  }
  int trailing = ndigits  - leading ;

  snprintf(s, n, "%0*.*f", ndigits, trailing, fabs(value));

  // both 34470a, dmm7510, 3458a preserve leading positive sign in display
  // 34401a. uses empty space
  s[ 0] = value >= 0 ? '+' : '-';

  return s;
}
#endif

char * str_format_value( char *s, size_t n,  unsigned ndigits, unsigned leading, double val)
{
  // handle the sign separately, since may need different font
  // printf("%u\n", leading);

  if( ndigits < leading) {
    assert( 0);
  }
  else if( ndigits == leading) {

  }
  else {
    // +1 for dots
    ndigits += 1;
    leading += 1;
  }
  int trailing = ndigits  - leading ;

  // format without sign
  // important, this correctly handles rounding of last digit
  snprintf(s, n, "%0*.*f", ndigits, trailing, fabs( val));
  return s;
}





void val_adjust_multiplier( double *val, char *c)
{
  // adjust value to use a unit reasonable for display purpose
  unsigned count = 0;

  *c = ' '; // default

  char ch[] =  { ' ', 'm', 'u', 'n', 'p', 'f', 'a' };

  while( fabs( *val) <=  1.2 && (count + 1) < sizeof(ch ) ) {
    // printf("here1\n");
    *val *= 1000;
    ++count ;

    assert( count < sizeof( ch));
    *c = ch[ count];
  }

  char ch2[] =  { ' ', 'k', 'M', 'G', 'T', 'P', 'E' };

  while( fabs( *val) >  1200  && (count + 1) < sizeof(ch2 )  ) {
    // printf("here2\n");
    *val /= 1000;
    ++count ;
    assert( count < sizeof( ch2));
    *c = ch2[ count];
  }
}


void val_force_multiplier( double *val, char c)
{
  switch( c) {

    case ' ':   break;
    case 'm': *val *= 1000; break;
    case 'u': *val *= 1e6; break;
    case 'n': *val *= 1e9; break;
    case 'p': *val *= 1e12; break;
    case 'a': *val *= 1e15; break;
    case 'k': *val /= 1e3; break;
    case 'M': *val /= 1e6; break;
    case 'G': *val /= 1e9; break;
    case 'T': *val /= 1e12; break;
    case 'P': *val /= 1e15; break;
    default:
      assert( 0);
  };
}





unsigned val_leading_digits( double val_)
{
  // case of 0
  if (val_ == 0)
    return 1;

  unsigned val = fabs( val_);
  unsigned count = 0;

  while (val > 0) {
    // printf("val %f\n", val);
    val /= 10;
    count++;
  }
  return count ;
}



char * str_format_value_dynamic( char *s, size_t sz, double val, unsigned ndigits)
{
  // needs at least 4 digits to display value with unit adjust
  assert( ndigits >= 4);

  // adjust unit
  char ch;
  val_adjust_multiplier( &val, &ch);
  // printf("adjust val %f%c\n", val, ch );

  unsigned leading = val_leading_digits( val);
  assert( ndigits >= leading);
  // printf( "digits %u\n", digits );
  // printf( "leading %u\n", leading );

  char buf[ 100 + 1];
  snprintf( 
    s, sz, 
    "%c%s%c",
    val >= 0 ? '+' : '-',
    str_format_value( buf, 100, ndigits, leading, val ),
    ch
  );

  return s;
}






#if 0

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

#endif



