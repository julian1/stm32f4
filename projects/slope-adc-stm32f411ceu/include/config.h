
#pragma once

#include <stdio.h> // FILE
#include "matrix.h"




// do we malloc(). No. the reading/writing shouldn't be responsible / defer allocation strategy.

struct Cal
{
  // actually slot is read separately. and perhaps should not be included here.
  unsigned slot;

  // date. tempertuare.

  // Params used.
 
  MAT *b;
};


typedef struct Cal Cal;



void file_skip_to_end(  FILE *f);

// rename file_scan_cal.

int file_scan( FILE *f, Cal **cal, unsigned sz );

// rename file_write_cal
void m_write_flash ( Cal *cal, int slot, FILE *f);

void cal_report( Cal *cal /* FILE *f */ );



#if 0
MAT * m_read_flash( MAT *out, FILE *f);
int file_skip_to_last_valid(  FILE *f);
#endif




