
#pragma once

// #include <stdint.h>
#include <stdbool.h>


#if 0
// better name clear_reset?
// void buffer_set_size( MAT *buffer, uint32_t sz);

MAT * buffer_reset( MAT *buffer, uint32_t sz);

void buffer_push( MAT *buffer, uint32_t *idx, double val );

void buffer_stats_print( MAT *buffer /* flags */ );

#endif

/* feb 2026.
      we want to add the repl command to change the buffer size
      and add a buffers update.

*/


#define BUFFERS_MAGIC 88888123

typedef struct data_t data_t;


typedef struct buffers_t
{
  uint32_t magic;

  data_t *data;

  // malloc'd.
  double *values;

  unsigned size;
  unsigned i;   //  modulus
  unsigned n;

  bool show_buffers;

  bool show;

} buffers_t;





buffers_t * buffers_create( data_t *);

bool buffers_repl_statement( buffers_t *, const char *cmd);

void buffers_update( buffers_t *);

