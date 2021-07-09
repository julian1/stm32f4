
#ifndef C_BUFFER_H
#define C_BUFFER_H

#include <stdio.h>    // FILE
#include <stdint.h>   // int32_t
#include <stddef.h>   // size_t
#include <stdbool.h>  // bool


typedef struct CBuf
{
  char *p;
  size_t sz;
  size_t wi;
  size_t ri;
  // bool wr_overflow_flag; //
} CBuf;



void cBufInit(CBuf *a, char *p, size_t sz);
void cBufPush(CBuf *a, char val);

bool cBufisEmpty(const CBuf *a);

size_t cBufCount(const CBuf *a);

int32_t cBufPop(CBuf *a);

int32_t cBufPeekLast(const CBuf *a);
int32_t cBufPeekFirst(const CBuf *a);

int32_t cBufCopyString(CBuf *a, char *p, size_t n); // change name readString...
int32_t cBufCopyString2(const CBuf *a, char *p, size_t n);


#if 0
// don't think these belong here, as primitives
//
int32_t cBufMark(CBuf *a);
void cBufReverse(CBuf *a, int count);
void indentLeft(CBuf *a, int count);
#endif

// can implement separate, non-string copy funcs, without sentinel for byte array /mem application functions
// non terminated...

////////////////
int32_t cBufRead(CBuf *a, char *p, size_t n);
ssize_t cBufWrite(CBuf *x, const char *buf, size_t size);


FILE * cBufMakeStream( CBuf *x );   // change name cBufMkStream ? to be able to write...

////////////

void cBufprintf( CBuf *cookie,
  ssize_t (*cBufWrite)(CBuf *x, const char *buf, size_t size),
  const char *format, ...);


void cBufWriteStream(CBuf *x, FILE *stream);






#endif
