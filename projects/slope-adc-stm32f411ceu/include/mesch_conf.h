#ifndef _H_MESCH_CONF
#define _H_MESCH_CONF

/*
  compiling err.c is useful. but we have to intercept a few calls for embedded.
  also see, alternative https://github.com/github0null/mesch/blob/master/port/mesch_conf.h
  
  

*/

// JA. most of these are for err.c

#define UNUSED(x) (void)(x)

static inline int fileno(void *p) 
{
    UNUSED(p);
    return 0;
}

#if 0
static inline int isatty(int x) 
{
    UNUSED(x);
    return 0;
}
#endif

static inline int isascii(int c) 
{
    UNUSED(c);
    return 0;
}

static inline void exit(int status) 
{
    // must intercept, to avoid link errors via linking standard library
    UNUSED(status);
}

#endif
