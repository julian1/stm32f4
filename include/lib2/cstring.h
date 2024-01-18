/*
  like a std::string. with contiguous mem, and terminal char 0 for interfacing with C.   
  but uses a preallocated max buffer / and does no mem allocation
  simpler than using std::string and custom allocator.
  can add/expose begin() finish() funcs to work std::algorithm<> stuff

  - better than circular buffer, when don't need thread/interupt guarantees
*/


#pragma once


#ifdef __cplusplus
extern "C" {
#endif




#include <stdint.h>   // int32_t
#include <stddef.h>   // size_t
#include <stdbool.h>  // bool
#include <unistd.h>   // ssize_t




typedef struct CString
{
  char *start, *end;
  char *pos;
} CString;



void cStringInit(CString *a, char *start, char *end);


bool cStringisEmpty(const CString *a);
size_t cStringCount(const CString *a);
size_t cStringReserve(const CString *a);
int32_t cStringPeekLast(const CString *a);
int32_t cStringPeekFirst(const CString *a);

void cStringClear(CString *a);
void cStringPush(CString *a, char val);
int32_t cStringPop(CString *a);


char * cStringPtr(CString *a);


ssize_t cStringWrite(CString *x, const char *buf, size_t size);

#ifdef __cplusplus
}
#endif



