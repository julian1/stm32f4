
#pragma once

#include <stdbool.h>



#define BUFFERS_MAGIC 88888123

typedef struct data_t data_t;


typedef struct buffer_t
{
  uint32_t magic;

  data_t *data;

  // memory dependency, pass on construction
  double *values;
  size_t max_n ;    // max memory to work size



  size_t i;         // next index/ modulus
  size_t size;      // limit size of full buffer.  rename sz? consistent with stl.
  size_t count;     // current count of elements present.


  double mean;
  double stddev;
  double min, max;


  // bool show_buffer;

  bool show;

} buffer_t;






void buffer_init( buffer_t *buffer, data_t *data, double *values, size_t n);

bool buffer_repl_statement( buffer_t *, const char *cmd);

void buffer_update( buffer_t *);




