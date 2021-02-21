
#include "buffer.h"



void fBufInit(FBuf *a, float *p, size_t sz)
{
  a->p = p;
  a->sz = sz;
  a->wi = 0;
  a->ri = 0;
}

void fBufWrite(FBuf *a, float val)
{
  (a->p)[ a->wi] = val;
  a->wi = (a->wi + 1) % a->sz;
}


bool fBufEmpty(FBuf *a)
{
  return a->ri == a->wi;
}


float fBufRead(FBuf *a)
{
  // THIS AINT MUCH GOOD.... need a separate isEmpty...
  if(a->ri == a->wi)
    return -999999999;  // MAX_FLOAT?

  float ret = (a->p)[a->ri];
  a->ri = (a->ri + 1) % a->sz;
  return ret;
}

////////////////////


// ring buffer for output...


void cBufInit(CBuf *a, char *p, size_t sz)
{
  a->p = p;
  a->sz = sz;
  a->wi = 0;
  a->ri = 0;
}

void cBufWrite(CBuf *a, char val)
{
  (a->p)[ a->wi] = val;
  a->wi = (a->wi + 1) % a->sz;
}

int32_t cBufRead(CBuf *a)
{
  if(a->ri == a->wi)
    return -1;

  char ret = (a->p)[ a->ri];

  a->ri = (a->ri + 1) % a->sz;
  return ret;
}



