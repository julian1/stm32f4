


// MUST BE FIRST...
#define _GNU_SOURCE     // required for cookie_io_functions_t
#include <stdio.h>

#include "streams.h"
#include "usart.h" // usart1_enable_output_interupt()
#include "cbuffer.h"

#include <stdarg.h> // va_starrt etc
#include <stdlib.h> // malloc
#include <assert.h>

#define UNUSED(x) (void)(x)


/*
  reason not to use fgetch / fread(), fgets() etc is that these are normally used blocking
  ------
  could /should use non-blocking fread() instead of cBufPop()...

  stdin can be set to non-block in linux. eg. like this. tested.
  so could harmonize interface

  Note. control over blocking would be useful for allowing software libs to use stdin.


  void fd_set_non_block(FILE *f, bool nonblock)
  {
  // works in linux.
  // https://stackoverflow.com/questions/6055702/using-fgets-as-non-blocking-function-c

  int fd = fileno(f);
  int flags = fcntl(fd, F_GETFL, 0);

  if(nonblock)
    flags |= O_NONBLOCK;
  else
    flags &= ~ O_NONBLOCK;

  fcntl(fd, F_SETFL, flags);
  }

  -----

  albeit we could control... reasonably easily.
  if( f == stdout )
  else if( f == stdin ) etc.


*/





#if 0
static void output_char( CBuf *console_out , int ch)
{
  if(ch  == '\n') {
    cBufPush(console_out, '\r');
  }

  cBufPush(console_out, ch);
}
#endif


struct Cookie
{
  CBuf *console_out;
  bool flush_on_newline;
};

typedef struct Cookie Cookie;



static void * file_to_cookie( FILE *f )
{
  /* should not be exposed.
    but allows supporting other file based operations over our structure
  */

  // actually a handler
  void *cookie_ptr= f->_cookie;
  // printf("cookie_ptr %p\n", cookie_ptr);

 // follow it
 void *cookie = * (void **) cookie_ptr ;
 // printf ("cookie %p\n",  cookie );

  return cookie;
}


void fflush_on_newline( FILE *f, bool val)
{
  assert(f);
  Cookie * cookie = file_to_cookie( f );
  cookie-> flush_on_newline = val;
}




static ssize_t mywrite(Cookie *cookie, const char *buf, size_t size)
{

  for(unsigned i = 0; i < size; ++i ) {

    int ch = buf[ i ];

    if(ch  == '\n') {
      cBufPush(cookie->console_out, '\r');
      cBufPush(cookie->console_out, ch);

      // maybe block control, in order to flush buffer
      if(cookie->flush_on_newline)
        usart1_flush();

    }
    else {
      cBufPush(cookie->console_out, ch);
    }
  }

  // re-enable tx interupt... if needed
  // could do line buffering here, if wanted.
  usart1_enable_output_interupt();

  return size;
}



void cbuf_init_std_streams( CBuf *console_out )
{
  /*
    advantage  is that we don't need intermediate handling and stack temp BUFFER.
    for vsnprintf. etc.
    Also see,
      https://www.openstm32.org/forumthread8113
  */

  cookie_io_functions_t memfile_func = {
     .read  = NULL, // read
     .write =  (cookie_write_function_t *) mywrite,
     .seek  = NULL, // seek,
     .close = NULL  //close
  };

  // memory never released.
  Cookie *cookie = malloc(sizeof(Cookie));
  assert(cookie);

  // loverly designated initializer
  *cookie = (Cookie const) {     
    .console_out      = console_out,
    .flush_on_newline = false,
  };


  FILE *f = fopencookie( cookie , "w", memfile_func);

  // required, because we use circ buffer as buffer
  setbuf(f, NULL);


  // change stdout to pOINT at f.
  stdout = f;
  stderr = f;
  // stdin... ignore for the moment.
}



/*
legacy, gg
  should probably move to project local util.c if really want to use.
*/

void usart1_printf(const char *format, ...)
{
  // old interface
	va_list args;
	va_start(args, format);
  int ret = vfprintf(stdout , format, args );
	va_end(args);
  UNUSED(ret);
}


