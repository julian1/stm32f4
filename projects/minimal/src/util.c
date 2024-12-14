
#include <stdio.h>
#include <assert.h>
#include <string.h>   // strcmp()
#include <stdlib.h> // strtolu()
// #include <strings.h>   // strcasecmp()
#include <ctype.h>    // isdigit isspace


#include <util.h>
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


void aper_cc_print( uint32_t aperture,  uint32_t line_freq)
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



  // 2 of 4 mux values
  else if(strcmp(s, "w4") == 0 )
    *val = W4;
  else if(strcmp(s, "w3") == 0 )
    *val = W3;
  else if(strcmp(s, "w2") == 0 )
    *val = W2;
  else if(strcmp(s, "w1") == 0 )
    *val = W1;
  else if(strcmp(s, "woff") == 0 )
    *val = WOFF;




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

    if(unit == 'm')
      *val *= 1e-3;
    else if(unit == 'u')
      *val *= 1e-6;
    else if(unit == 'n')
      *val *= 1e-9;
    else
      return 0;

    return 1;
  }


  return 0;

}




