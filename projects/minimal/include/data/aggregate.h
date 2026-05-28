
#pragma once

#include <stdbool.h>



#define AGGREGATE_MAGIC 928328428



typedef struct aggregate_t
{
  uint32_t magic;


  // memory dependency, pass on construction
  double *values;
  size_t max_sz;


  size_t i;         // next index/ modulus
  size_t size;



} aggregate_t;






void aggregate_init( aggregate_t *buffer, double *values, size_t sz);

bool aggregate_repl_statement( aggregate_t *, const char *cmd);

void aggregate_update_data( aggregate_t *aggregate, /*const */ data_t *data);




