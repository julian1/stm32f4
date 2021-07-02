
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

// TODO rename write() to put(), read() to get(), or even push() and pop()


// char buffer


void cBufInit(CBuf *a, char *p, size_t sz)
{
  a->p = p;
  a->sz = sz;
  a->wi = 0;
  a->ri = 0;
}

void cBufPut(CBuf *a, char val)
{
  (a->p)[ a->wi] = val;
  a->wi = (a->wi + 1) % a->sz;
}

bool cBufisEmpty(CBuf *a)
{
  return a->ri == a->wi;
}




int32_t cBufPop(CBuf *a)
{
  // sentinal value...
  if(a->ri == a->wi)
    return -1;

  // read then update index. - but could be reordered by compiler
  char ret = (a->p)[ a->ri];
  a->ri = (a->ri + 1) % a->sz;

  return ret;
}





int32_t cBufPeekFirst(CBuf *a)
{
  // sentinal value...
  if(a->ri == a->wi)
    return -1;

  return (a->p)[a->ri];
}

int32_t cBufPeekLast(CBuf *a)
{
  // sentinal value...
  if(a->ri == a->wi)
    return -1;

  // this kind of needs some tests
  if(a->wi == 0) {
    return (a->p)[a->sz - 1];
  }
  else
    return (a->p)[a->wi - 1];
}


int32_t cBufCopy(CBuf *a, char *p, size_t n)
{
  // copy and consume
  // could use cBufIsEmpty(), but the sential is guaranteed for chars.

  // we should do the sentinal....  this is for char arrays

  // not tested much...
  int32_t ch;
  size_t i = 0;
  while(i < n && (ch = cBufPop(a)) >= 0) {
    p[i++] = ch;
  }
  return i;
}


// we really need to be able to copy out the buffer... so we have a nice string without consuming.
// if this is used for chars


int32_t cBufCopy2(CBuf *a, char *p, size_t n)
{

  size_t i = a->ri;
  size_t u = 0;

  while((i % a->sz) != a->wi && u < n - 1) {

    p[u++] = (a->p)[i];
    ++i;
  }

  p[u] = 0;   // sentinel
  return u;
}







