
#pragma once

#include <stdio.h>  // FILE
#include "matrix.h" // MAT




// do we malloc(). No. the reading/writing shouldn't be responsible / defer allocation strategy.

struct Cal
{
  // actually slot is read separately. and perhaps should not be included here.
  unsigned slot;

  // date. tempertuare.

  // Params used.

  MAT *b;

  // Param     param;  // Or make optional pointer.
                    // Or just record fix_n, var_n ? maybe aperture.
                    // We are going to have to include app.h to pick up Param? hmmm 
                    // app is currently including cal... 
};


typedef struct Cal Cal;



void file_skip_to_end(  FILE *f);



int file_scan_cal( FILE *f, Cal **cal, unsigned sz );

void file_write_cal ( Cal *cal, FILE *f);

void cal_report( Cal *cal /* FILE *f */ );



#if 0
MAT * m_read_flash( MAT *out, FILE *f);
int file_skip_to_last_valid(  FILE *f);
#endif




