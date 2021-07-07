
#ifndef C_BUFFER_H
#define C_BUFFER_H

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
void cBufPut(CBuf *a, char val);  // change name push() consistent with stl.

bool    cBufisEmpty(CBuf *a);


size_t cBufElements(CBuf *a);     // change name size() consistent with stl.

int32_t cBufPop(CBuf *a);
int32_t cBufPeekLast(CBuf *a);    // change name to peek() ie. most recent

int32_t cBufPeekFirst(CBuf *a);

int32_t cBufCopyString(CBuf *a, char *p, size_t n);
int32_t cBufCopyString2(CBuf *a, char *p, size_t n);

// can implement non-string copy funcs without sentinel for byte array /mem functions

#endif
