
// MUST BE FIRST...
#define _GNU_SOURCE     // required for cookie_io_functions_t
#include <stdio.h>


#include <stdlib.h> // malloc
#include <assert.h>
#include <errno.h>    // EAGAIN


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





#if 0

This is not needed and probably does not work correctly
since the FILE is cookie based, without underlying file descriptor

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>


static void set_stdin_nonblocking( void) {
  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

#endif



static ssize_t cookie_read( cookie_t *cookie, char *buf, size_t sz)
{

  /*
    EAGAIN: The file is non-blocking and the read would block.
  */

  /*
    you do not need to explicitly call clearerr() or clear errno
    immediately after receiving EAGAIN (or EWOULDBLOCK). EAGAIN is a temporary
    state indicating a non-blocking operation would block, not a fatal error. You
    should simply retry the operation later. However, you must ensure you check
    errno only immediately after a system call returns -1.
  */

  assert( cookie);

#if 0
  // OK. this works to propagate errno out of calls on FILE *.
  errno = EAGAIN;
  return 0;   // must be 0. not -1.
#endif

  /*
    could control over blocking behvior here,
    using flags and ffcntl()
  */

  if( cbuf_is_empty( cookie->cinput)) {
    errno = EAGAIN;
    return 0; // must be 0. not -1.
  }
  else {

    return cbuf_read( cookie->cinput, buf, sz);
  }
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





