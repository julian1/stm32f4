
#pragma once

#include <stdio.h>  // FILE
#include "matrix.h" // MAT

#include "app.h"    // Param


  /* -----------------
  stderr(regression) *is* a very procy good indicator of DA and non linearity.
    because they won't fit to a linear regression.
  -----------------
  */

// do we malloc(). No. the reading/writing shouldn't be responsible / defer allocation strategy.

struct Cal
{
  // actually slot is read separately. and perhaps should not be included here.
  unsigned slot;


  MAT     *b;

  // var_n, fix_n
  Param   param;

  // we can calc sigma, and aperture adjusted voltage. from this
  double  sigma2;

  // temperatureC
  double  temp;

  // TODO. remove comment_sz.... instead we are already computing dynamically.
  unsigned comment_sz;  // unused.
  char    *comment;

  unsigned id;   // identifier would be useful. for associating.

  unsigned model;

};


typedef struct Cal Cal;



void file_skip_to_end(  FILE *f);



int file_scan_cal( FILE *f, Cal **cal, unsigned sz );

void file_write_cal ( Cal *cal, FILE *f);

void cal_report( Cal *cal /* FILE *f */ );


Cal * cal_create(void);
void cal_free( Cal *cal  );
Cal * cal_copy( Cal *cal  );

#if 0


MAT * m_read_flash( MAT *out, FILE *f);
int file_skip_to_last_valid(  FILE *f);


#endif




