


#include <stddef.h> // size_t
#include <stdio.h> // snprintf
#include <math.h> // pow
#include <string.h> // memcpy


#include "assert.h" 

#include "value.h"

#define UNUSED(x) (void)(x)

/*

    - use centre button to drill in. and other button to drill out.
    - controller exposes idx and focus as external vars. to suggest the active items.

    -----
    list controller - passes item to edit to the element controller.
                    - probably want a structure for this. eg. bounds.


*/


/*
  validation function - eg. bounds should be passed as a function.
  probably also the formatting function.
    eg. for number of digits. rather than specify as a value.
*/


static char * format_float(char *s, size_t sz, int suffix_digits, double value)
{
  /*
    %f. always adds '.' and suffix '0' even input is rounded.
    this will prefix with '-'
  */
  // format
  size_t n = snprintf(s, sz, "%.*f", suffix_digits, value);
  UNUSED(n);
  return s;
}







void value_float_edit(double *x, int idx, int amount)
{
  /*
    this isn't working with the decimal point
  */

  printf("value_float_edit x=%f   idx=%d amount=%d \n", *x, idx, amount );

  // skip decimal point. should perhaps be done outside here.
  // index
  if (idx > 0 ) {
    --idx;
  }

  // must be float for negative idx
  // some math.h have pow10(double)
  double u = pow(10, idx);
  double delta = amount * u;
  // printf("idx=%d amount=%d u=%f \n", idx, amount, u );

  *x = *x + delta;

  // return x + delta;
}


void value_float_copy( const double *x, void *dst, size_t sz )
{
  /*
    UGGH. no we want to be copying all the functions as well.
    No. just the underlying void object.
  */
  assert( sizeof(double) <= sz);

  // void *memcpy(void *dest, const void *src, size_t n);
  memcpy(dst, x, sizeof(double) );
}



void value_float_format( const double *x, char *buf, size_t sz)
{
    char buf2[100];
    snprintf(buf, sz,  "%smV" , format_float(buf2, 100, 6, *x  ));
}

void value_float_format2( const double *x, char *buf, size_t sz)
{
    char buf2[100];
    snprintf(buf, sz,  "%suV" , format_float(buf2, 100, 2, *x  ));
}



