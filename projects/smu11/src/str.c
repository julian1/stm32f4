
//
#include <stdio.h>
#include <stdarg.h> // va_start etc

#include "assert.h"
#include "str.h"


char * indent_left(char *s, size_t sz, int indent, const char *string)
{
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



/*
  compiler issues warnings about callers of this func, if used in the same file. quite odd,
  i think because digit width takes priority over the input buffer length passed to snprintf
  so compiler is doing printf format checking on the inlined function.
*/

char * format_float(char *s, size_t sz, int digits, double value)
{
  /*
    // eg. works
    printf("%0.*g\n",  5, 123.456789 );      // 123.46
    printf("%0.*g\n",  5, 12.3456789 );      // 12.346
    printf("%0.*g\n",  5, -12.3456789 );     // -12.346
  */

  ASSERT( digits < ((int)sz) - 2);  // basic sanity check ... TODO review...

  snprintf(s, sz, "%0.*g\n",  digits, value);
  return s;
}


char * format_bits(char *buf, size_t width, uint32_t value)
{
  // passing the buf, means can use more than once in printf expression. using separate bufs
  char *s = buf;

  for(int i = width - 1; i >= 0; --i) {
    *s++ = value & (1 << i) ? '1' : '0';
  }

  *s = 0;
  return buf;
}


