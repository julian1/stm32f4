
#pragma once

#include <stdio.h>  // FILE
#include "matrix.h" // MAT

#include "app.h"    // Param



// do we malloc(). No. the reading/writing shouldn't be responsible / defer allocation strategy.

struct Cal
{
  // actually slot is read separately. and perhaps should not be included here.
  unsigned slot;


  MAT *b;

  // used - for tests. not sure if there's a better place to store.
  Param param;  // Or make optional pointer.
                    // Or just record fix_n, var_n ? maybe aperture.
                    // We are going to have to include app.h to pick up Param? hmmm
                    // app is currently including cal...

  /* -----------------
  stderr(regression) *is* a very good indicator for DA. because DA will introduce non-linearities that are difficult to fit.
  -----------------
  */
  // we can calc sigma, and aperture adjusted voltage. from this
  double sigma2;

  double temp;

  // unsigned df;
  // char *comment; eg. 1nF/100k. 45kHz. timedate.


  unsigned comment_sz;
  char * comment;

  unsigned id;   // identifier would be useful. for associating.

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




