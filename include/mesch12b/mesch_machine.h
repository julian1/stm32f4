/* machine.h.  Generated automatically by configure.  */
/* Any machine specific stuff goes here */
/* Add details necessary for your own installation here! */

/* This is for use with "configure" -- if you are not using configure
	then use machine.van for the "vanilla" version of machine.h */

/* Note special macros: ANSI_C (ANSI C syntax)
			SEGMENTED (segmented memory machine e.g. MS-DOS)
			MALLOCDECL (declared if malloc() etc have
					been declared) */

/* #undef const */

#if defined __has_include
#if __has_include("mesch_conf.h")
#include "mesch_conf.h"
#endif
#else
#include "mesch_conf.h"
#endif

/* memory alloc is thread safe */
#define THREADSAFE 1
#undef THREADSAFE       // JA. required for  void mem_stat_dump(FILE *fp,int list);

/* #undef MALLOCDECL */
#define NOT_SEGMENTED 1
/* #undef HAVE_COMPLEX_H */
#define HAVE_MALLOC_H 1
#define STDC_HEADERS 1
#define HAVE_BCOPY 1
#define HAVE_BZERO 1
#define CHAR0ISDBL0 1
/* #undef WORDS_BIGENDIAN */
#define U_APER_DEF 1
#define VARARGS 1

/* for basic or larger versions */
#define COMPLEX 1

// JA
#define SPARSE 0

/* for loop unrolling */
/* #undef VUNROLL */
/* #undef MUNROLL */

/* for segmented memory */
#ifndef NOT_SEGMENTED
#define SEGMENTED
#endif

/* if the system has malloc.h */
#ifdef HAVE_MALLOC_H
#define MALLOCDECL 1
//#include	<malloc.h>
#endif

/* any compiler should have this header */
/* if not, change it */
#include <stdio.h>

/* Check for ANSI C memmove and memset */
#ifdef STDC_HEADERS

/* standard copy & zero functions */
#define MEM_COPY(from, to, size) memmove((to), (from), (size))
#define MEM_ZERO(where, size) memset((where), '\0', (size))

#ifndef ANSI_C
#define ANSI_C 1
#endif

#endif

/* standard headers */
#ifdef ANSI_C
/* disable stdlib */
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <float.h>
#endif

/* if have bcopy & bzero and no alternatives yet known, use them */
#ifdef HAVE_BCOPY
#ifndef MEM_COPY
/* nonstandard copy function */
#define MEM_COPY(from, to, size) bcopy((char *)(from), (char *)(to), (int)(size))
#endif
#endif

#ifdef HAVE_BZERO
#ifndef MEM_ZERO
/* nonstandard zero function */
#define MEM_ZERO(where, size) bzero((char *)(where), (int)(size))
#endif
#endif

/* if the system has complex.h */
#ifdef HAVE_COMPLEX_H
#include <complex.h>
#endif

/* If prototypes are available & ANSI_C not yet defined, then define it,
	but don't include any header files as the proper ANSI C headers
        aren't here */
#define HAVE_PROTOTYPES 1
#ifdef HAVE_PROTOTYPES
#ifndef ANSI_C
#define ANSI_C 1
#endif
#endif

/* floating point precision */

/* you can choose single, double or long double (if available) precision */

#define FLOAT 1
#define DOUBLE 2
#define LONG_DOUBLE 3

/* #undef REAL_FLT */
/* #undef REAL_DBL */

/* if nothing is defined, choose double precision */
#ifndef REAL_DBL
#ifndef REAL_FLT
#define REAL_DBL 1
#endif
#endif

/* single precision */
#ifdef REAL_FLT
#define Real float
#define LongReal float
#define REAL FLOAT
#define LONGREAL FLOAT
#endif

/* double precision */
#ifdef REAL_DBL
#define Real double
#define LongReal double
#define REAL DOUBLE
#define LONGREAL DOUBLE
#endif

/* machine epsilon or unit roundoff error */
/* This is correct on most IEEE Real precision systems */
#ifdef DBL_EPSILON
#if REAL == DOUBLE
#define MACHEPS DBL_EPSILON
#elif REAL == FLOAT
#define MACHEPS FLT_EPSILON
#elif REAL == LONGDOUBLE
#define MACHEPS LDBL_EPSILON
#endif
#endif

#define F_MACHEPS 1.19209e-07
#define D_MACHEPS 2.22045e-16

#ifndef MACHEPS
#if REAL == DOUBLE
#define MACHEPS D_MACHEPS
#elif REAL == FLOAT
#define MACHEPS F_MACHEPS
#elif REAL == LONGDOUBLE
#define MACHEPS D_MACHEPS
#endif
#endif

/* #undef M_MACHEPS */

/********************
#ifdef DBL_EPSILON
#define	MACHEPS	DBL_EPSILON
#endif
#ifdef M_MACHEPS
#ifndef MACHEPS
#define MACHEPS	M_MACHEPS
#endif
#endif
********************/

#define M_MAX_INT 2147483647
#ifdef M_MAX_INT
#ifndef MAX_RAND
#define MAX_RAND ((double)(M_MAX_INT))
#endif
#endif

/* for non-ANSI systems */
#ifndef HUGE_VAL
#define HUGE_VAL HUGE
#endif

#ifdef ANSI_C
// JA. use #include <unistd.h> instead
// else gives repeat declation warnings everywhere
//extern int isatty(int);
#endif
