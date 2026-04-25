
#pragma once


#ifdef __cplusplus
extern "C" {
#endif


#include <stdbool.h>  // bool
#include <unistd.h>   // ssize_t



typedef struct cbuf_t cbuf_t;

struct cbuf_t
{
  // uint32_t  magic;

  char      *p;
  size_t    sz_max;
  size_t    wi;
  size_t    ri;
  // bool wr_overflow_flag;
};



void cbuf_init( cbuf_t *b, char *p, size_t sz_max);


size_t cbuf_capacity( const cbuf_t *b);
bool cbuf_is_empty( const cbuf_t *b);
size_t cbuf_size( const cbuf_t *b);

int32_t cbuf_back( const cbuf_t *b);
int32_t cbuf_front( const cbuf_t *b);

// size_t cbuf_reserve(cbuf_t *b);
void cbuf_clear( cbuf_t *b);
void cbuf_push( cbuf_t *b, char val);
int32_t cbuf_pop( cbuf_t *b);


// int32_t cbuf_read( cbuf_t *b, char *p, size_t n);
ssize_t cbuf_read( cbuf_t *b, char *p, size_t n);
ssize_t cbuf_write( cbuf_t *x, const char *buf, size_t size);



#ifdef __cplusplus
}
#endif


