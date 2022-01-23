#ifndef _H_MESCH_CONF
#define _H_MESCH_CONF

// #pragma once

/*
  compiling err.c saves having to override/code the error handling ourselves.
  but we want to intercept a few calls for embedded.
  also see, alternative approach here, https://github.com/github0null/mesch/blob/master/port/mesch_conf.h
*/


// hacky,
// include stdio, which I think defines putc (not just declares).
#include <stdio.h>

// then redefine putc() to our own function.
int mesch_putc(int c, void *stream);
#define putc(a,b) mesch_putc(a,b)


int fileno(void *p) ;

// int isatty(int x) ;

int isascii(int c) ;
void exit(int status) ;

#endif
