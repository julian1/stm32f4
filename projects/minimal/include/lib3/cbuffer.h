
#pragma once


#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>   // int32_t
#include <stddef.h>   // size_t
#include <stdbool.h>  // bool
#include <unistd.h>   // ssize_t

// circ_buf_t ?
// cbuf_t

typedef struct cbuf_t
{
  char      *p;
  size_t    sz;
  size_t    wi;
  size_t    ri;
  // bool wr_overflow_flag;
} cbuf_t;



void cbuf_init( cbuf_t *a, char *p, size_t sz);


size_t cbuf_capacity( const cbuf_t *a);
bool cbuf_is_empty( const cbuf_t *a);
size_t cbuf_size( const cbuf_t *a);

int32_t cbuf_back( const cbuf_t *a);
int32_t cbuf_front( const cbuf_t *a);

// size_t cbuf_reserve(cbuf_t *a);
void cbuf_clear( cbuf_t *a);
void cbuf_push( cbuf_t *a, char val);
int32_t cbuf_pop( cbuf_t *a);


// int32_t cbuf_read( cbuf_t *a, char *p, size_t n);
ssize_t cbuf_read( cbuf_t *a, char *p, size_t n);
ssize_t cbuf_write( cbuf_t *x, const char *buf, size_t size);



#ifdef __cplusplus
}
#endif


