
#ifndef F_BUFFER_H
#define F_BUFFER_H

#include <stdint.h>   // int32_t
#include <stddef.h>   // size_t
#include <stdbool.h>  // bool

typedef struct FBuf
{
  float *p;
  size_t sz;
  size_t wi;
  size_t ri;
  // overflow
} FBuf;


void fBufInit(FBuf *a, float *p, size_t sz);
void fBufPush(FBuf *a, float val);
bool fBufisEmpty(FBuf *a);


size_t fBufCount(FBuf *a);

float fBufPeekLast(FBuf *a);


float fBufPop(FBuf *a);

// consumes...
int32_t fBufCopy(FBuf *a, float *p, size_t n);


// want a copy interface func. and a reset interface func.

#endif
