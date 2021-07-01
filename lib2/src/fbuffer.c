
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


#include "fbuffer.h"

// TODO rename write() to put(), read() to get(), or even push() and pop()

// float buffer...

void fBufInit(FBuf *a, float *p, size_t sz)
{
  a->p = p;
  a->sz = sz;
  a->wi = 0;
  a->ri = 0;
}

void fBufPut(FBuf *a, float val)
{
  // update val, then increment index
  (a->p)[ a->wi] = val;
  a->wi = (a->wi + 1) % a->sz;
}


bool fBufEmpty(FBuf *a)
{
  // TODO change name IsEmpty
  return a->ri == a->wi;
}


float fBufPop(FBuf *a)
{
  // THIS AINT MUCH GOOD.... need a separate isEmpty...
  if(a->ri == a->wi)
    return -999999999;  // MAX_FLOAT?

  // read then update index. - but could be reordered by compiler
  float ret = (a->p)[a->ri];
  a->ri = (a->ri + 1) % a->sz;

  return ret;
}


