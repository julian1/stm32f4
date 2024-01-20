
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
  char *p;
  size_t sz;
  size_t wi;
  size_t ri;
  // bool wr_overflow_flag;
} cbuf_t;



void cbuf_init(cbuf_t *a, char *p, size_t sz);

// bool circ_buf_is_empty(const cbuf_t *a);   too long winded
// bool cbuf_is_empty(const cbuf_t *a);   ok.

bool cbuf_is_empty(const cbuf_t *a);
size_t cbuf_count(const cbuf_t *a);
size_t cbuf_reserve(cbuf_t *a);  
int32_t cbuf_peek_last(const cbuf_t *a);
int32_t cbuf_peek_first(const cbuf_t *a);

void cbuf_clear(cbuf_t *a);
void cbuf_push(cbuf_t *a, char val);
int32_t cbuf_pop(cbuf_t *a);


int32_t cbuf_copy_string(cbuf_t *a, char *p, size_t n); // change name readString...
int32_t cbuf_copy_string2(const cbuf_t *a, char *p, size_t n);


////////////////
int32_t cbuf_read(cbuf_t *a, char *p, size_t n);
ssize_t cbuf_write(cbuf_t *x, const char *buf, size_t size);

// can implement separate, non-string copy funcs, without sentinel for byte array /mem application functions
// non terminated...



#ifdef __cplusplus
}
#endif


