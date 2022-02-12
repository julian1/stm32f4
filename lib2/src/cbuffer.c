/*
  TODO, Should change name cCircBuf

  ----------
  point of circular buffer, with separate ri,wi is thread safety.
    - interupt can write/push value, while in the middle of a read.
    - interupt can read value, while in the middle of a write

    - we won't ever index outsize the buffer.


  Actually better approach for handling uart could be
  to leave interrupt unenabled, until after character is written (mem + index update),
  then reenable the interrupt.

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
#include <assert.h>



void cBufInit(CBuf *a, char *p, size_t sz)
{
  assert(a);
  assert(p);
  assert(sz > 0);

  a->p = p;
  a->sz = sz;
  a->wi = 0;
  a->ri = 0;
}





bool cBufisEmpty(const CBuf *a)
{
  return a->ri == a->wi;
}


size_t cBufCount(const CBuf *a)
{
  int n = a->wi - a->ri;
  if(n < 0)
    n += a->sz;

  return n;
}





void cBufClear(CBuf *a)
{
  assert(a);

  a->wi = 0;
  a->ri = 0;
}



void cBufPush(CBuf *a, char val)
{
/*
  assert(a);
  assert(a->p);
  assert(a->wi < a->sz );
  assert( a->sz > 0);
*/
  // update val
  (a->p)[a->wi] = val;

  // increment wi
  a->wi = (a->wi + 1) % a->sz;

  /* handle overflow more gracefully.
    if overflow, increment the ri
    so that subsequent reads will get more recent data, and avoid truncation/ empty.
    ----------
    IMPORTANT but for thread safety, this breaks assumption, that pushing will not touch the read index.
  */
  if(a->wi == a->ri) {

    a->ri = (a->ri + 1) % a->sz;
  }
}



int32_t cBufPop(CBuf *a)
{
  // ie as fifo. pop first pushed. *not* most recent.
  assert(a->ri != a->wi);

  // read then update index. - but could be reordered by compiler
  char ret = (a->p)[ a->ri];
  a->ri = (a->ri + 1) % a->sz;

  return ret;
}




int32_t cBufPeekFirst(const CBuf *a)
{
  // ie. peek first char to be pushed, considered as fifo.
  // eg. char that will be popped
  // TODO rename cBufPeek()
  assert(a->ri != a->wi);

  return (a->p)[a->ri];
}



int32_t cBufPeekLast(const CBuf *a)
{
  // last item to be pushed...
  assert(a->ri != a->wi);

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
  // copy and consume
  // could use more testing
  // interface is for for c-style strings, so must handle sentinel

  size_t i = 0;

  while(i < (n - 1) && !cBufisEmpty(a)) {
    p[i++] = cBufPop(a);
  }

  // sentinel
  p[i++] = 0;
  return i;
}


int32_t cBufCopyString2(const CBuf *a, char *p, size_t n)
{
  // copy and and don't consume. leave buf intact

  // could use more testing
  // for c-style strings, so handle sentinel

  size_t ri = a->ri;
  size_t i = 0;

  while(ri != a->wi && i < (n - 1)) {

    assert(ri < a->sz);
    p[i++] = (a->p)[ri];

    ri = (ri + 1) % a->sz;
  }

  // sentinel
  p[i++] = 0;
  return i;
}



///////////////////

// consumes...
// no sentinel

/*
  for use with cookie_io_functions_t
*/

int32_t cBufRead(CBuf *a, char *p, size_t n)
{

  size_t i = 0;
  while(i < n && !cBufisEmpty(a)) {
    p[i++] = cBufPop(a);
  }

  return i;
}


ssize_t cBufWrite(CBuf *a, const char *buf, size_t size)
{
  assert(a->sz);

  for(size_t i = 0; i < size; ++i)
    cBufPush(a, buf[i]);

  return size;
}



