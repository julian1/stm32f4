

#include <stdbool.h>    // true
#include <stdio.h>
#include <stdarg.h> // va_start etc
#include <ctype.h>    // isdigit

#include "assert.h"
#include "format.h"


char * indent_left(char *s, size_t sz, int indent, const char *string)
{
  // TODO use pointer loop and remove dependency on snprintf()
  // left indent, is pad to right, for field name
  snprintf(s, sz, "%-*s", indent, string );
  return s;
}



char * indent_right(char *s, size_t sz, int indent, const char *string)
{
  // right indent, is pad to left, for field value
  snprintf(s, sz, "%*s", indent, string);
  return s;
}



char * snprintf2(char *s, size_t sz, const char *format, ...)
{
  // same as snprintf but return the input buffer, as a convenience for caller
	va_list args;
	va_start(args, format);
	vsnprintf(s, sz, format, args);
	va_end(args);

  return s;
}




char * format_float(char *s, size_t sz, int digits, double value)
{
  /*
    this will zero pad to the right if needed.
    it also only formats the prec after the decimal point.
    this is probably mostly what we want. eg. 6 digits of uV.
    -------------
    EXTR.
    
      when we draw graphically. we want to draw from rhs digit to lhs digit.
      eg.  so if the most significant digit changes adds a digit. 
      then it does not disturb the precision of the lower bits.

      this is a bit difficult with glyph advance which assumes right direction.
      no. just draw with monospace font.
    ---------------
      eg.
        from 7.123  to 17.123.    the 123 should be aligned.
  */
  snprintf(s, sz, "%.*fV", digits, value);
  return s;
}



char * format_float_with_commas(char *s, size_t sz, int digits, double value)
{ 
  // assume we have plenty of working space in buffer
  char *j = s + 50;
  snprintf(j , sz - 50 , "%.*f", digits, value);

  bool gotdot = false;
  unsigned dotdigits = 0;
  char *dst = s;

  while(*j && dst < s + sz)   {

    if( *j == '.')
      gotdot = true;

    if(gotdot && isdigit(*j))
      ++dotdigits;

    *dst++ = *j++;

    // if(dotdigitdst == 6)
    // eg. comma every third digit 
    if( dotdigits != 0
      && (dotdigits % 3) == 0
      && dst < s + sz)
      *dst++ = ',';
  }

  // always add terminal
  if( dst < s + sz)
    *dst = 0;
  else
    dst[sz - 1] = 0; 


  return s;
}






char * format_bits(char *buf, size_t width, uint32_t value)
{
  // eg. size_t == width + 1 for terminal.
  // passing the buf, means can use more than once in printf expression. using separate bufs
  char *s = buf;

  for(int i = width - 1; i >= 0; --i) {
    *s++ = value & (1 << i) ? '1' : '0';
  }

  *s = 0;
  return buf;
}










/*
  compiler issues warnings about callers of this func, if used in the same file. quite odd,
  i think because digit width takes priority over the input buffer length passed to snprintf
  so compiler is doing printf format checking on the inlined function.
*/

#if 0
char * format_float(char *s, size_t sz, int digits, double value)
{
  /*
    // eg. works
    printf("%0.*g\n",  5, 123.456789 );      // 123.46
    printf("%0.*g\n",  5, 12.3456789 );      // 12.346
    printf("%0.*g\n",  5, -12.3456789 );     // -12.346
  */

  ASSERT( digits < ((int)sz) - 2);  // basic sanity check ... TODO review...

  /*
    correct number of digits, but this doesn't add trailing 0...

  */
  snprintf(s, sz, "%0.*g",  digits, value);
  return s;
}
#endif




/*
  for a certain amount of prec.
  why not just take the decimal past section and multiply eg. 1000 for 3 digits. and format as an int?
  OR.
  just print with extra digits and then truncate.
*/

#if 0

char * format_float(char *s, size_t sz, int digits, double value)
{
  // this isn't working as we want.
  // leading digits are included.
  // So if we can determine


  /*
  from man snprintf
    %g The precision specifies the number of significant digits.  [...]
    Trailing zeros are removed from the fractional part of the result; a decimal
    point appears only if it is followed by at least one digit.  */


  // format
  size_t i = snprintf(s, sz, "%0.*g",  digits, value);

  // count how many digits we got
  int c = 0;
  for(char *p = s; *p; ++p) {
    if(isdigit((int)*p))
      ++c;
  }

  // check for decimal decimal place...
  bool dot = false;
  for(char *p = s; *p; ++p) {
    if(*p == '.')
      dot = true;
  }

  // maybe add dot
  if(!dot && i < (sz - 1)) {
    s[i++] = '.';
  }

  // maybe add trailing zeros that were ignored/dropped
  for(int k = c; k < digits && i < (sz - 1); ++k) {
    s[i++] = '0';
  }

  // add new sentinel
  ASSERT(i < sz);
  s[i++] = 0;
  return s;
}

#endif




