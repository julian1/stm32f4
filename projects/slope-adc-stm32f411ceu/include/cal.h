
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
  Param     param;  // Or make optional pointer.
                    // Or just record fix_n, var_n ? maybe aperture.
                    // We are going to have to include app.h to pick up Param? hmmm
                    // app is currently including cal...

/*
  // - ( stddev(regression) / aperture ) * 1000000  is the uV 1SD deviation.
  double sigma ;           // explains fit. should consider storing.

  // we need to show this.
  double sigma_div_aperture;

  // date. tempertuare.
*/

  // can calc sigma, and aperture adjusted voltage. from this
  // double sigma2;

  // double temp;
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




