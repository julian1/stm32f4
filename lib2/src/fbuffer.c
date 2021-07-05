
/*
  Actually better approach is just,
    leave interrupt unenabled, until item is written (mem + index update), then reenable the interrupt.

    this avoids a double interupt.

    this doesn't get and put at the same time but ought to be enough.
    also write the value before updating the index.
    so index will not just jump.

  --------------
  - issue - an interupt interupted by same priority interupt.  eg. get() on top of get()
      so

      see, https://www.eevblog.com/forum/embedded-computing/concurrency-in-a-bare-metal-environment/msg3506550/?topicseen#msg3506550

  - need mutex.

 43   circular buffer
 44     - can an interupt be interrupted by a same priority interrupt.
 45       this is where the update of the index, and the write of the value can go wrong.
*/


#include <float.h>    // FLT_MAX ughhh
#include "fbuffer.h"

// TODO rename write() to put(), read() to get(), or even push() and pop()

// float buffer...

void fBufInit(FBuf *a, float *p, size_t sz)
{
  // memset()
  a->p = p;
  a->sz = sz;
  a->wi = 0;
  a->ri = 0;
}

void fBufPut(FBuf *a, float val)
{
  // update val,
  (a->p)[ a->wi] = val;
  // then increment index
  a->wi = (a->wi + 1) % a->sz;
}


bool fBufisEmpty(FBuf *a)
{
  return a->ri == a->wi;
}



size_t fBufElements(FBuf *a)
{
  int n = a->wi - a->ri;
  if(n < 0)
    n += a->sz;

  return n;
}




float fBufPop(FBuf *a)
{
  // messy, but shouldn't be called
  if(a->ri == a->wi)
    return FLT_MAX ;
    // return NAN;       // math.h

  // read then update index. - but could be reordered by compiler
  float ret = (a->p)[a->ri];
  a->ri = (a->ri + 1) % a->sz;

  return ret;
}


int32_t fBufCopy(FBuf *a, float *p, size_t n)
{
  size_t i = 0;
  while(i < n && !fBufisEmpty(a))
    p[i++] = fBufPop(a);

  return i;
}
