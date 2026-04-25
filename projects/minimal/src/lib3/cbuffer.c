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


#include <assert.h>

#include <lib3/cbuffer.h>



void cbuf_init( cbuf_t *b, char *p, size_t sz_max)
{
  assert( b);
  assert( p);
  assert( sz_max > 0);

  *b = (const cbuf_t) {
    // .magic = BUFFER_MAGIC,
    .p      = p,
    .sz_max = sz_max,
    .wi     = 0,
    .ri     = 0,
  };
}


size_t cbuf_capacity( const cbuf_t *b)
{

  return b->sz_max;
}


bool cbuf_is_empty( const cbuf_t *b)
{
  return b->ri == b->wi;
}


size_t cbuf_size( const cbuf_t *b)
{

  int n = b->wi - b->ri;
  if(n < 0)
    n += b->sz_max;

  return n;
}



int32_t cbuf_back( const cbuf_t *b)
{
  // ie. peek last char to be pushed, considered as fifo.

  // last item to be pushed...
  assert( !cbuf_is_empty( b));

  // this kind of needs some tests
  if(b->wi == 0) {
    return (b->p)[b->sz_max - 1];
  }
  else
    return (b->p)[b->wi - 1];
}



int32_t cbuf_front( const cbuf_t *b)
{
  // ie. peek first char to be pushed, considered as fifo.

  assert( !cbuf_is_empty( b));

  return (b->p)[b->ri];
}




void cbuf_clear( cbuf_t *b)
{
  assert( b);

  b->wi = 0;
  b->ri = 0;
}


void cbuf_push( cbuf_t *b, char val)
{
/*
  assert( b);
  assert( b->p);
  assert( b->wi < b->sz_max );
  assert( b->sz_max > 0);
*/
  // update val
  (b->p)[b->wi] = val;

  // increment wi
  b->wi = (b->wi + 1) % b->sz_max;

  /* handle overflow more gracefully.
    if overflow, increment the ri
    so that subsequent reads will get more recent data, and avoid truncation/ empty.
    ----------
    IMPORTANT but for thread safety, this breaks assumption, that pushing will not touch the read index.
  */
  if(b->wi == b->ri) {

    // could set/raise overflow flag here.
    b->ri = (b->ri + 1) % b->sz_max;
  }
}



int32_t cbuf_pop( cbuf_t *b)
{
  // ie as fifo. pop first pushed. *not* most recent.

  assert( !cbuf_is_empty( b));

  // read then update index. - but could be reordered by compiler
  char ret = (b->p)[ b->ri];
  b->ri = (b->ri + 1) % b->sz_max;

  return ret;
}



ssize_t cbuf_read( cbuf_t *b, char *p, size_t n)
{

  size_t i = 0;
  while(i < n && !cbuf_is_empty( b)) {
    p[ i++] = cbuf_pop( b);
  }

  return i;
}


ssize_t cbuf_write( cbuf_t *b, const char *buf, size_t size)
{
  assert( b->sz_max);   // must be initialized

  for(size_t i = 0; i < size; ++i)
    cbuf_push( b, buf[i]);

  return size;
}




#if 0

/*
  funcs with space for string sentinal.
  if we really need these funcs,
  then just rewrite using cbuf_read() and cbuf_write(), with n-1

  but probably better if caller handles.

*/

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

  size_t ri = b->ri;
  size_t i = 0;

  while(ri != b->wi && i < (n - 1)) {

    assert(ri < b->sz_max);
    p[i++] = (b->p)[ri];

    ri = (ri + 1) % b->sz_max;
  }

  // sentinel
  p[i++] = 0;
  return i;
}

#endif


