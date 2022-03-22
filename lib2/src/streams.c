


// MUST BE FIRST...
#define _GNU_SOURCE     // required for cookie_io_functions_t
#include <stdio.h>

#include "streams.h"
#include "usart.h" // usart1_enable_output_interupt()
#include "cbuffer.h"

#include <stdarg.h> // va_starrt etc

#define UNUSED(x) (void)(x)


/*
  reason not to - is that fread(), fgets() etc are normally used blocking
  ------
  could /should use non-blocking fread() instead of cBufPop()...

  stdin can be set to non-block in linux. eg. like this. tested.
  so could harmonize interface

  Note. control over blocking would be useful for allowing software libs to use stdin.


  void fd_set_non_block(FILE *f, bool nonblock)
  {
  // works.
  // https://stackoverflow.com/questions/6055702/using-fgets-as-non-blocking-function-c

  int fd = fileno(f);
  int flags = fcntl(fd, F_GETFL, 0);

  if(nonblock)
    flags |= O_NONBLOCK;
  else
    flags &= ~ O_NONBLOCK;

  fcntl(fd, F_SETFL, flags);
  }



*/






static void output_char( CBuf *console_out , int ch)
{
  if(ch  == '\n') {
    cBufPush(console_out, '\r');
  }

  cBufPush(console_out, ch);
}



static ssize_t mywrite(void *cookie, const char *buf, size_t size)
{

  CBuf *console_out = (CBuf *) cookie;

  for(unsigned i = 0; i < size; ++i ) {
    output_char( console_out, buf[ i ] );
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

  cookie_io_functions_t  memfile_func = {
       .read  = NULL, // read
       .write = mywrite,
       .seek  = NULL, // seek,
       .close = NULL  //close
   };

  FILE *f = fopencookie( console_out , "w", memfile_func);
  // FILE *f = fopencookie(NULL, "w", memfile_func);
  // FILE *f = fopencookie(&x, "w", memfile_func); // this works too.

  // required
  setbuf(f, NULL);


  // change stdout to point at f.
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


