
// MUST BE FIRST...
#define _GNU_SOURCE     // required for cookie_io_functions_t
#include <stdio.h>


#include <stdlib.h> // malloc
#include <assert.h>


#include <lib3/cbuffer.h>
#include <lib3/file-input.h>


#define UNUSED(x) (void)(x)





typedef struct cookie_t cookie_t;

struct cookie_t
{
  // consider put flags first. so ffnctl can
  // abstract

  cbuf_t    *cinput;
  // int    flags;
};






static ssize_t cookie_read( cookie_t *cookie, char *buf, size_t sz)
{

  /* return 0 on non-blocking buf empty,
  which gets turned to EOF(-1) by FILE read.
  EOF can be clear by calling clearerr()
  */
  assert( cookie);

  /*
    could also implement blocking behvior here,
    controllable with ffcntl()
    instead of bytes available/ returned
  */
  return cbuf_read( cookie->cinput, buf, sz);
}




FILE *file_open_input_cbuf( cbuf_t *cinput)
{
  assert( cinput);


  cookie_io_functions_t file_func = {
    .read  =  (cookie_read_function_t *) cookie_read,      // avoid casting ptrf, because types get confusing
    .write = NULL ,
    .seek  = NULL,
    .close = NULL
  };

  // TODO memory not released.
  // implement close()
  cookie_t *cookie = malloc( sizeof(cookie_t));
  assert(cookie);


  // nice C99 init
  *cookie = (cookie_t const) {
    .cinput  = cinput,
    // .flags     = 0,
  };


  FILE *f = fopencookie( cookie , "r", file_func);

  // Not sure if required. for stdin.
  // required, because we use circ buffer as buffer
  setbuf(f, NULL);

  return f;
}





