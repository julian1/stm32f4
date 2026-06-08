
#pragma once

#include <stdbool.h>



#define BUFFER_MAGIC 88888123

typedef struct decode_t decode_t;


typedef struct buffer_t
{
  uint32_t  magic;


  // memory dependency, pass on construction
  double    *values;
  size_t    max_sz;    // max work size
  size_t    size;      // buffer size

  size_t    i;         // index/ modulus
  size_t    count;     // count of elements present.


  double    mean;
  double    stddev;
  double    min, max;

  /*
    where to manage the stop policy.
    we dont want to pass the gpio_trigger from repl_trig_state;
  */
  // gpio_t        *gpio_trigger;
  // bool repl_trig_state.
  // bool  stop_on_full; // policya  hmmmm.... .
  // bool show;

  unsigned  verbose;

} buffer_t;






void buffer_init( buffer_t *buffer, double *values, size_t sz);

bool buffer_repl_statement( buffer_t *, const char *cmd);

void buffer_update_data( buffer_t *, const data_t *data);




