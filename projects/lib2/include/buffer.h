
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
void fBufWrite(FBuf *a, float val);
bool fBufEmpty(FBuf *a);
float fBufRead(FBuf *a);

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
void cBufWrite(CBuf *a, char val);
int32_t cBufRead(CBuf *a);


#endif
