/*
  jmpbuf size is 200 bytes on x86/64.

  cortex m4 setjmp implementation
  https://gist.github.com/buserror/3129219
    4 * ( 10 + 8 + 1 ) = 76 bytes.

  with gcc 9.2.1  20191025. libc  it's 92 bytes.
  ---

  cortext m4 spi slave.
*/

/*
It's quite reasonable because a trivial debug check is able to detect that
longjmp tries to jump upwards which is definitely incorrect (of course in
reality the code does not tries to jump upwards, it tries to jump to a
completely unrelated stack). The good news is that the check can be suppressed
with #undef _FORTIFY_SOURCE.



The setjmp() function saves various information about the calling environment
(typically, the stack pointer, the instruction pointer, possibly the values of
other registers and the signal  mask) in the buffer env for later use by
longjmp().

The  longjmp()  function  uses  the  information  saved in env to
transfer control back to the point where setjmp() was called and to restore

*/

#undef _FORTIFY_SOURCE

#include <setjmp.h>
#include <stdio.h>
// #include <stdlib.h> // abort, arc4random
// #include <stdbool.h> // true

#include "coroutine.h"
#include "usart.h" // flush



// static void *coarg;


void *coto(jmp_buf *here, jmp_buf *there, void *arg)
{
  // coroutine to
  // coarg = arg;

  // save return point into here
  if (setjmp(*here))
    return(arg );

  // jump to there
  longjmp(*there, 1);
}


// x86,arm stack grows down.
#define STACKDIR - // set to + for upwards and - for downwards
#define STACKSIZE (1<<12)   // 4096 4k is quite a bit.

static char *tos; // top of stack

void *cogo(jmp_buf *here, void (*fun)(void*), void *arg)
{

  printf("creating thread \n" );

  // first time, then set the top of the stack
  if (tos == NULL)
    tos = (char*)&arg;

  // increase stack top
  tos += STACKDIR STACKSIZE;

  // read/put argument on stack?/ or return value?
  char n[STACKDIR (tos - (char*)&arg)];
  void * coarg = n; // ensure optimizer keeps n

  if (setjmp(*here))
    return(coarg);
  fun(arg);
  // abort();
}


///////////////////////////////////


// #define MAXTHREAD 10000
#define MAXTHREAD 100

static jmp_buf thread[MAXTHREAD];

// current number of coroutines. changes during spawning
// start at 1, for main execution thread
static int thread_count = 1;

// current executing thread_id,
// start at 0 for main thread
static int thread_current_id = 0;




static void yield( void )
{
  // current thread, so we can jump back here
  int tmp = thread_current_id;

  // deterministic
  thread_current_id = (thread_current_id + 1)  % thread_count;
  // random
  // thread_current_id = rand() % thread_count;

  printf("jumping to %d\n", thread_current_id );

  coto(&thread[ tmp], &thread[ thread_current_id ] , 0 );
}





static void create( void (*pf)(void *), void *arg )
{
  // spawn a different function
  thread_current_id = thread_count;
  ++thread_count;
  cogo( &thread[0], pf , arg );
}





static void comain(void *arg)
{
  /* 
    we want to be able to get the top of stack for each function.
    to know that we are inside it.
    and
  */


  // we can pass any function we like to the main func
  // we don't want to have to pass arguments for the thread to yeld from
  // ma
  int id = *(int *)arg;

  for (;;) {
    printf("coroutine %d     thread_current_id %d   %p \n", id, thread_current_id, &arg );
    yield();
  }
}


static void comain2(void *arg)
{
  for (;;) {
    printf("coroutine2 <-- \n" );
    yield();
  }
}


/*
  EXTR.
  - handling interupts quickly (eg. adc 50Hz) use a dedicated coroutine could
  . since we can break up any long running function with a yield.

  - can test the millisecs before yielding in a loop. if want to be
    more complex.

*/


int coroutine_main(void)
{
  printf("-----------\n" );
  // jumpbuf size is 200 bytes.


  printf("coroutine_main()\n" );
  printf("jumpbuf size %u\n" , sizeof(jmp_buf) );

  
  
  // this is correct.
  for(unsigned i = 1; i < 5; ++i) {
    create( comain, &i  );
  }


 // create( comain2, &thread_count);


#if 1
  // and yield from main thread when we drop through
  while(1) {

    usart1_flush();
    //printf("yield from main thread\n");
    yield();
  }
#endif



  return 0;
}


