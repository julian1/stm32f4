
#include <stdio.h>
#include <string.h> // strlen
#include <math.h>   // pow

#include "format_float.h"

/*
  TOO.
  OK. float formatting does need rouding. and propagation 
    eg. frequenly get 0.99999  instead of 1.0
    because we are not rounding.

  - this means we need a loop to run a separate buffer from lsd to hsd and propagate rounding up.
  - and probably should work out all significant digits for the representation eg. 4 bit float, 8 bit double. 

*/

/*
  it's the ptr and bounds checkign that makes this ugly....
  issue is that we need to be able to reverse...
*/



#define SWAP(T, a, b) do { T tmp = a; a = b; b = tmp; } while (0)

// #define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/*
  for clean log formatting (without vt100, ansi terminal), then more control is good. 
  it needs a get() function also to get contents. if want to consume()

*/

/*
  This interface is pretty useful.
  for - cookie printf based print functions
  and for indenting left and right - complicated text
  ------------- 

  to be able to be consumed as a source... however... it needs more
*/

typedef struct J J;

typedef struct J
{
  void (*push)(J *ctx, unsigned ch);
  // write(J *ctx, char *, size_t n )                 <- for efficiency. makes a more useful generalized stream interface. for format_bits, format_float , snprintf
                                                      // actually push can be rewritten in terms of write(j,  &ch, 1); for better efficiency
  void (*reverse)(J *ctx, int count);
  size_t (*mark)(J *ctx);
} J;


// wrapper functions push(), mark(), reverse() make the interface easier and more opaque, to use regardless.
// not sure.

/*
  ALTERNATIVELY. if have a get()  consume()  and separate buffers. 
    then we don't require the reverse() function... for float/integer formatting
*/



#if 0

void push(J *j, unsigned ch)
{
  j->write(j, &ch, 1);
}
#endif


static void format_integer(J *j,  unsigned int x)
{
  size_t m = j->mark(j);
  do
  {
    j->push(j, x % 10u + '0');
    x /= 10u;
  }
  while (x != 0);
  j->reverse(j, j->mark(j) - m);
}


static void format_frac(J *j, double x, int digits)
{
  // format frac, with n digits
  //

  for(int i = 0; i < digits; ++i)
  {

    int xx = x * pow (10, i + 1);

    // we should do rounding here, but it's too hard
/*
    int xx = ( i < digits - 1)
      ? x * pow (10, i + 1 )
      :  round( x * pow (10, i + 1 ) );
 */
    j->push(j, xx % 10u + '0');
  }
}




static void format_float1(J *j, double x, int digits )
{
  if(x < 0) {
    x *= -1.;
    j->push(j, '-');
  }

  int intpart = (int)x;

  size_t m = j->mark(j);
  format_integer(j, intpart);

  size_t intpartlen = j->mark(j) - m;

  j->push(j, '.');

  double fracpart = x - intpart;
  format_frac(j, fracpart, digits - intpartlen);

  // IMPORTANT don't terminate string here...
  // because cannot guarantee that overflow means sentinel will not get pushed.
}



////////////////////////////////

typedef struct K
{
  J   j;
  // char buf[100];
  char *buf;
  int sz;
  int i;
} K;


static void push(K *k, unsigned ch)
{
  if(k->i < k->sz) {
    k->buf[ k->i++] = ch;
  }
}


static size_t mark(K *k)
{
  return k->i;
}


static void reverse2(char *begin, char *end)
{
  // note. assuming begin is within range.
  // argument end is one past the last char. same as c++ stl...
  --end;

  while(begin < end)
  {
    SWAP(char, *begin, *end);
    ++begin;
    --end;
  }
}


static void reverse(K *k, int count)
{
  // reverse last count chars
  char *end = & k->buf[k->i];
  char *start = end - count;

  reverse2(start, end);
}




// what kind of interface do we want to present?
// we kind of want to print to a buffer...
// same as the uint

// bitfield format...

char * format_float(char *buf, size_t len, double value, int digits)
{
  // format float with a most digits...
  // same interface as uint_to_bits ...
  K   k;
  memset(&k, 0, sizeof(K));
  k.j.push = (void *)push;
  k.j.mark = (void *)mark;
  k.j.reverse = (void *)reverse;

  k.buf = buf;
  k.sz = len;

  format_float1((J *)&k, value, digits);    // total digits... except if less

  if(k.i < k.sz) {
    // push terminal
    k.j.push((J*)&k, '\0');
  } else {
    // ASSERT(len >= 1);
    // overflowed buffer, sentinel will not fit...
    *buf = '\0';
  }

  return buf;
}





#if 0

int main()
{
  char buf[100];

  printf("%s", format_float(buf, 100, -123456780.0000, 5));

  // printf("stdio value  %f\n", x );  // ignores the 8....
  // printf("our value    %s\n", k.buf );
}

#endif


