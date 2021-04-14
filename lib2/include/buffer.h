
#ifndef C_BUFFER_H
#define C_BUFFER_H

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
void fBufPut(FBuf *a, float val);
bool fBufEmpty(FBuf *a);
float fBufPop(FBuf *a);

/////////

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
int32_t cBufPop(CBuf *a);
int32_t cBufPeekFirst(CBuf *a);
int32_t cBufPeekLast(CBuf *a);

int32_t cBufCopy(CBuf *a, char *p, size_t n);


#endif
