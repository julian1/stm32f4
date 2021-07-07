
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
void cBufPush(CBuf *a, char val);

bool    cBufisEmpty(CBuf *a);


size_t cBufCount(CBuf *a);

int32_t cBufPop(CBuf *a);
int32_t cBufPeekLast(CBuf *a);

int32_t cBufPeekFirst(CBuf *a);

int32_t cBufCopyString(CBuf *a, char *p, size_t n);
int32_t cBufCopyString2(CBuf *a, char *p, size_t n);

// can implement separate, non-string copy funcs, without sentinel for byte array /mem application functions

#endif
