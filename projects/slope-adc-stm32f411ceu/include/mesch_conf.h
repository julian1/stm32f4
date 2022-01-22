#ifndef _H_MESCH_CONF
#define _H_MESCH_CONF

/*
  Not compiling err.c 
  but instead writing our own handler, saves a lot of redefining stuff here versus using defines,

    https://github.com/github0null/mesch/blob/master/port/mesch_conf.h

*/

#define UNUSED(x) (void)(x)

static inline int fileno(void *p) 
{
    UNUSED(p);
    return 0;
}



#endif
