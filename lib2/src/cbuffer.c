
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


#include "cbuffer.h"
#include "assert.h"

// TODO rename write() to put(), read() to get(), or even push() and pop()


// char buffer


void cBufInit(CBuf *a, char *p, size_t sz)
{
  ASSERT(a);
  ASSERT(p);
  a->p = p;
  a->sz = sz;
  a->wi = 0;
  a->ri = 0;
}

void cBufPut(CBuf *a, char val)
{
  (a->p)[a->wi] = val;
  a->wi = (a->wi + 1) % a->sz;
}

bool cBufisEmpty(CBuf *a)
{
  return a->ri == a->wi;
}


size_t cBufElements(CBuf *a)
{
  // note, not correct if overflows...
  int n = a->wi - a->ri;
  if(n < 0)
    n += a->sz;

  return n;
}


int32_t cBufPop(CBuf *a)
{
  ASSERT(a->ri != a->wi);

  // read then update index. - but could be reordered by compiler
  char ret = (a->p)[ a->ri];
  a->ri = (a->ri + 1) % a->sz;

  return ret;
}



int32_t cBufPeekFirst(CBuf *a)
{
  // maybe change name to just cBufPeek()
  ASSERT(a->ri != a->wi);

  return (a->p)[a->ri];
}

int32_t cBufPeekLast(CBuf *a)
{
  ASSERT(a->ri != a->wi);

  // this kind of needs some tests
  if(a->wi == 0) {
    return (a->p)[a->sz - 1];
  }
  else
    return (a->p)[a->wi - 1];
}


/*
  really think these should be returning size_t.
  due to no return error codes...
  but leave for future
*/

int32_t cBufCopyString(CBuf *a, char *p, size_t n)
{
  // could use more testing
  // copy and consume
  // interface is for for c-style strings, so must handle sentinel

  size_t i = 0;

  while(i < (n - 1) && !cBufisEmpty(a)) {
    p[i++] = cBufPop(a);
  }

  // sentinel
  p[i++] = 0;
  return i;
}


int32_t cBufCopyString2(CBuf *a, char *p, size_t n)
{
  // could use more testing
  // copy and leave intact without consuming
  // for c-style strings, so handle sentinel

  size_t ri = a->ri;
  size_t i = 0;

  while(ri != a->wi && i < (n - 1)) {

    ASSERT(ri < a->sz);
    p[i++] = (a->p)[ri];

    ri = (ri + 1) % a->sz;
  }

  // sentinel
  p[i++] = 0;
  return i;
}


