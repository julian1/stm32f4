
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

// #define _GNU_SOURCE     // required for cookie_io_functions_t
// #include <stdio.h>


#include "cbuffer.h"
#include <assert.h>



// char buffer


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
  // set val
  (a->p)[a->wi] = val;

  // increment wi
  a->wi = (a->wi + 1) % a->sz;

  // if we overflowed, then increment the ri
  // so read reads the more recent data, and avoid treated as truncated/empty.
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




bool cBufisEmpty(const CBuf *a)
{
  return a->ri == a->wi;
}


size_t cBufCount(const CBuf *a)
{
  // note, not correct if overflows...
  int n = a->wi - a->ri;
  if(n < 0)
    n += a->sz;

  return n;
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


int32_t cBufCopyString2(const CBuf *a, char *p, size_t n)
{
  // could use more testing
  // copy and leave and don't consume, leaving buf intact
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

  // read from the buf
  size_t i = 0;

  while(i < n && !cBufisEmpty(a)) {
    p[i++] = cBufPop(a);
  }

  return i;
}


ssize_t cBufWrite(CBuf *x, const char *buf, size_t size)
{
  /* more conventional interface
  */
  assert(x->sz);

  for(size_t i = 0; i < size; ++i)
    cBufPush(x, buf[i]);

  return size;
}


#if 0

see streams instead

FILE * cBufMakeStream( CBuf *x )
{
  static cookie_io_functions_t  memfile_func = {
       .read  = (void *) cBufRead,
       .write = (void *) cBufWrite,
       .seek  = NULL, // (void *) xseek,      // think seek might be needed to read...
       .close = NULL  // close
   };

  // FILE *f = fopencookie(x, "w+", memfile_func);
  // FILE *f = fopencookie(x, "w+", memfile_func);
  FILE *f = fopencookie(x, "r+", memfile_func);
  // FILE *f = fopencookie(x, "a+", memfile_func);

  // WE MUST close this later...
  return f;
}


///////////////


#include <stdarg.h>     // vprintf, va_starrt etc


void cBufprintf( CBuf *cookie,
  ssize_t (*cBufWrite_)(CBuf *x, const char *buf, size_t size),
  const char *format, ...)
{
  /*
    printf like formatting to cBuf
    - set up the stream, as required.
    but how expensive is this? versus passing the stream cookie to multiple formatting calls?
    - note that we can actually pass a context as well if we want...
  */
  cookie_io_functions_t  memfile_func = {
       .read  = NULL, // read
       .write = (void *) cBufWrite_,
       .seek  = NULL, // seek,
       .close = NULL  // close
   };

  FILE *f = fopencookie(cookie, "w", memfile_func);
  assert(f);

  va_list args;
  va_start(args, format);
  vfprintf(f, format, args);
  va_end(args);

  /* OK. closing f, fixes seg fault. even if occurs after expected seg.
    perhaps due to the way all streams is flushed by os on process term?
  */
  fclose(f);
}


void cBufWriteStream(CBuf *x, FILE *stream)
{
  // read from buf, and write to stream
  char buf[1000];
  ssize_t n;
  do {
    n = cBufRead(x, buf, sizeof(buf));
    fwrite(buf, 1, n, stream);
  } while (n > 0);
}


#endif

