
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
  // overflow
} CBuf;



void cBufInit(CBuf *a, char *p, size_t sz);
void cBufPut(CBuf *a, char val);

bool cBufisEmpty(CBuf *a);
int32_t cBufPop(CBuf *a);
int32_t cBufPeekFirst(CBuf *a);
int32_t cBufPeekLast(CBuf *a);

int32_t cBufCopy(CBuf *a, char *p, size_t n);


#endif
