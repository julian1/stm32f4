/*
  TODO. our circular buffer does not handle overflow very nicely. - the result is truncated.

*/

#ifndef C_BUFFER_H
#define C_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif


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

void cBufClear(CBuf *a);


void cBufPush(CBuf *a, char val);

bool cBufisEmpty(const CBuf *a);

size_t cBufCount(const CBuf *a);

int32_t cBufPop(CBuf *a);

int32_t cBufPeekLast(const CBuf *a);
int32_t cBufPeekFirst(const CBuf *a);

int32_t cBufCopyString(CBuf *a, char *p, size_t n); // change name readString...
int32_t cBufCopyString2(const CBuf *a, char *p, size_t n);


////////////////
int32_t cBufRead(CBuf *a, char *p, size_t n);
ssize_t cBufWrite(CBuf *x, const char *buf, size_t size);

// can implement separate, non-string copy funcs, without sentinel for byte array /mem application functions
// non terminated...






#ifdef __cplusplus
}
#endif



#endif
