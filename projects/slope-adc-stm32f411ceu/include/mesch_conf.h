#ifndef _H_MESCH_CONF
#define _H_MESCH_CONF

/*
  compiling err.c saves having to override/code the error handling ourselves.
  but we want to intercept a few calls for embedded.
  also see, alternative https://github.com/github0null/mesch/blob/master/port/mesch_conf.h
*/

int fileno(void *p) ;

#if 0
int isatty(int x) ;
#endif

int isascii(int c) ;
void exit(int status) ;

#endif
