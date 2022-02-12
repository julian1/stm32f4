
#ifndef F_BUFFER_H
#define F_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif




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

bool fBufisEmpty(FBuf *a);
size_t fBufReserve(FBuf *a);
size_t fBufCount(FBuf *a);
float fBufPeekLast(FBuf *a);

void fBufPush(FBuf *a, float val);
float fBufPop(FBuf *a);
void fBufClear(FBuf *a);



// consumes...
int32_t fBufCopy(FBuf *a, float *p, size_t n);

int32_t fBufCopy2(const FBuf *a, float *p, size_t n);


// want a copy interface func. and a reset interface func.



#ifdef __cplusplus
}
#endif




#endif


