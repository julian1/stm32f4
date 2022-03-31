
#pragma once

#include <stdio.h> // FILE
#include "matrix.h"


// rename file_skip_to_end

void c_skip_to_end(  FILE *f);
#if 0
int c_skip_to_last_valid(  FILE *f);
#endif


// rename file_scan_values.

int c_scan( FILE *f, MAT **b, unsigned b_sz );


/*
  these are poorly named.
  they write a header, specific for iterating through nor flash.
  but are using the generic stream interface
*/

void m_write_flash ( MAT *m , int slot, FILE *f);

MAT * m_read_flash( MAT *out, FILE *f);



