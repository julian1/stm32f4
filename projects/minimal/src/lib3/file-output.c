

// MUST BE FIRST...
#define _GNU_SOURCE     // required for cookie_io_functions_t
#include <stdio.h>


// #include <stdarg.h> // va_starrt etc
#include <stdlib.h> // malloc
#include <assert.h>


#include <lib3/usart.h> // usart1_enable_output_interupt()
#include <lib3/cbuffer.h>

#include <lib3/file-output.h>


#define UNUSED(x) (void)(x)





struct Cookie
{
  cbuf_t    *circ_buf;
  int       flags;

  /* usart */

};


typedef struct Cookie Cookie;



static void * file_to_cookie( FILE *f )
{

  // actually a handler
  void *cookie_ptr= f->_cookie;
  // printf("cookie_ptr %p\n", cookie_ptr);

 // follow it
 void *cookie = * (void **) cookie_ptr ;
 // printf ("cookie %p\n",  cookie );

  return cookie;
}

// change name SYNC_OUTPUT_ON_NEWLINE
#define SYNC_OUTPUT_ON_NEWLINE   0x01



int ffnctl( FILE *f, int cmd)
{
  assert(f);
  Cookie * cookie = file_to_cookie( f );
  assert(cookie);

  if( cmd) {
    cookie->flags = cmd;
  }

  return cookie->flags;
}





static ssize_t mywrite( Cookie *cookie, const char *buf, size_t size)
{

  for(unsigned i = 0; i < size; ++i ) {

    int ch = buf[ i ];

    if(ch  == '\n') {
      cbuf_push(cookie->circ_buf, '\r');
      cbuf_push(cookie->circ_buf, ch);

      // block control, in order to flush circular buffer
      if(cookie->flags & SYNC_OUTPUT_ON_NEWLINE)
        usart1_flush();

    }
    else {
      cbuf_push(cookie->circ_buf, ch);
    }
  }

  // re-enable tx interupt... if needed
  // could do line buffering here, if wanted.
  usart1_enable_output_interupt();

  return size;
}



FILE *file_open_output_cbuf( cbuf_t *circ_buf /* usart */ )
// void cbuf_init_stdout_streams( cbuf_t *circ_buf )
{
  /*
    advantage of using is stdout, and avoid intermediate handling with buffers
    for vsnprintf. etc.
    Also see,
      https://www.openstm32.org/forumthread8113
  */

  cookie_io_functions_t file_func = {
     .read  = NULL,
     .write =  (cookie_write_function_t *) mywrite,
     .seek  = NULL,
     .close = NULL
  };

  // memory never released.
  Cookie *cookie = malloc(sizeof(Cookie));
  assert(cookie);

  // loverly designated initializer
  *cookie = (Cookie const) {
    .circ_buf  = circ_buf,
    .flags        = 0,
  };


  FILE *f = fopencookie( cookie , "w", file_func);

  // required, because we use circ buffer as buffer
  setbuf(f, NULL);

  return f;

/*
  // change stdout to pOINT at f.
  stdout = f;
  stderr = f;
*/
}













#if 0

/*
legacy, gg
  should probably move to project local util.c if really want
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


#endif

/*

  Good error / reporting idea, alternative to stack trace,

Just log/fprintf to a circular buffer. Which is ordinarily silent, but then flush to stdout/spi flash on assert.
Just need to override stream buffer implematation

void my_logger( FILE **circbuffer, __FILE__, __LINE___, const char **msg) ;
#define logger(f, msg)   my_logger( f, __FILE__, __LINE_, msg)

*/
/*
  reason not to use fgetch / fread(), fgets() etc is that these are normally used blocking
  ------
  could /should use non-blocking fread() instead of cbuf_pop()...

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



