/*

  TODO - consider adding clearable flag, set on overflow condition, that caller can use to check status

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



void cbuf_init(cbuf_t *a, char *p, size_t sz)
{
  assert(a);
  assert(p);
  assert(sz > 0);

  a->p = p;
  a->sz = sz;
  a->wi = 0;
  a->ri = 0;
}





bool cbuf_is_empty(const cbuf_t *a)
{
  return a->ri == a->wi;
}


size_t cbuf_count(const cbuf_t *a)
{
  int n = a->wi - a->ri;
  if(n < 0)
    n += a->sz;

  return n;
}



size_t cbuf_reserve(cbuf_t *a)
{
  return a->sz;
}


void cbuf_clear(cbuf_t *a)
{
  assert(a);

  a->wi = 0;
  a->ri = 0;
}



void cbuf_push(cbuf_t *a, char val)
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

    // could set/raise overflow flag here.
    a->ri = (a->ri + 1) % a->sz;
  }
}



int32_t cbuf_pop(cbuf_t *a)
{
  // ie as fifo. pop first pushed. *not* most recent.
  assert(a->ri != a->wi);

  // read then update index. - but could be reordered by compiler
  char ret = (a->p)[ a->ri];
  a->ri = (a->ri + 1) % a->sz;

  return ret;
}




int32_t cbuf_peek_first(const cbuf_t *a)
{
  // ie. peek first char to be pushed, considered as fifo.
  // eg. char that will be popped
  // TODO rename cBufPeek()
  assert(a->ri != a->wi);

  return (a->p)[a->ri];
}



int32_t cbuf_peek_last(const cbuf_t *a)
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

int32_t cbuf_copy_string(cbuf_t *a, char *p, size_t n)
{
  // copy and consume
  // could use more testing
  // interface is for for c-style strings, so must handle sentinel

  size_t i = 0;

  while(i < (n - 1) && !cbuf_is_empty(a)) {
    p[i++] = cbuf_pop(a);
  }

  // sentinel
  p[i++] = 0;
  return i;
}


int32_t cbuf_copy_string2(const cbuf_t *a, char *p, size_t n)
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

int32_t cbuf_read(cbuf_t *a, char *p, size_t n)
{

  size_t i = 0;
  while(i < n && !cbuf_is_empty(a)) {
    p[i++] = cbuf_pop(a);
  }

  return i;
}


ssize_t cbuf_write(cbuf_t *a, const char *buf, size_t size)
{
  assert(a->sz);

  for(size_t i = 0; i < size; ++i)
    cbuf_push(a, buf[i]);

  return size;
}



