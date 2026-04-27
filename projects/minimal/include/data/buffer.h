
#pragma once

#include <stdbool.h>



#define BUFFER_MAGIC 88888123

typedef struct decode_t decode_t;


typedef struct buffer_t
{
  uint32_t magic;


  // memory dependency, pass on construction
  double *values;
  size_t max_sz;    // max memory to work size
                    // change name max_sz or sz_max?



  size_t i;         // next index/ modulus
  size_t size;      // limit size of full buffer.  rename sz? consistent with stl.
  size_t count;     // current count of elements present.


  double mean;
  double stddev;
  double min, max;


  // bool show_buffer;
  bool show;

} buffer_t;






void buffer_init( buffer_t *buffer, double *values, size_t sz);

bool buffer_repl_statement( buffer_t *, const char *cmd);

void buffer_update_data( buffer_t *, const data_t *data);




