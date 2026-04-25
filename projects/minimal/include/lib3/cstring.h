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




#include <stdbool.h>  // bool
#include <unistd.h>   // ssize_t




typedef struct cstring_t
{
  char *start, *end;
  char *pos;
} cstring_t;



void cstring_init( cstring_t *a, char *start, char *end);


bool cstring_empty( const cstring_t *a);   // predicate
size_t cstring_size( const cstring_t *a);
size_t cstring_capacity( const cstring_t *a);
int32_t cstring_back( const cstring_t *a);
int32_t cstring_front( const cstring_t *a);

void cstring_clear( cstring_t *a);
void cstring_push_back( cstring_t *a, char val);
int32_t cstring_pop_back( cstring_t *a);


char * cstring_ptr( cstring_t *a);


ssize_t cstring_write( cstring_t *x, const char *buf, size_t size);

#ifdef __cplusplus
}
#endif



