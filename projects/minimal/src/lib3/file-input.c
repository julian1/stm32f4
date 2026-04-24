
// MUST BE FIRST...
#define _GNU_SOURCE     // required for cookie_io_functions_t
#include <stdio.h>


#include <stdarg.h> // va_starrt etc
#include <stdlib.h> // malloc
#include <assert.h>


#include <lib3/file-input.h>
#include <lib3/cbuffer.h>


#define UNUSED(x) (void)(x)






struct Cookie
{
  cbuf_t  *circ_buf;
  int   flags;
};

typedef struct Cookie Cookie;





static ssize_t myread( void *cookie_, char *buf, size_t sz)
{
  Cookie *cookie = (Cookie *) cookie_;

  /* return 0 on non-blocking buf empty,
  which gets turned to EOF(-1) by FILE read.
  EOF can be clear by calling clearerr()
  */
  assert(cookie);

  // so if we want block / no block sync...

  /*
    the issue with blocking here is that it won't pump update()
  */

  return cbuf_read( cookie->circ_buf, buf, sz);
}




FILE *file_open_input_cbuf( cbuf_t *console_in )
// void cbuf_init_stdin_streams( cbuf_t *console_in )
{
  assert(console_in);


  cookie_io_functions_t file_func = {
     .read  =  myread,      // avoid casting ptrf, because types get confusing
     .write = NULL ,
     .seek  = NULL,
     .close = NULL
  };

  // memory never released.
  Cookie *cookie = malloc(sizeof(Cookie));
  assert(cookie);

  // lovely designated initializer
  *cookie = (Cookie const) {
    .circ_buf  = console_in,
    .flags     = 0,
  };


  FILE *f = fopencookie( cookie , "r", file_func);

  // Not sure if required. for stdin.
  // required, because we use circ buffer as buffer
  setbuf(f, NULL);

  // stdin = f;
  return f;
}





