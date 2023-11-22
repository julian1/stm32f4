
/* perhaps rename file-blob. 
    this can save any type of structure
*/

#pragma once

#include <stdio.h>  // FILE


// #include "matrix.h" // MAT

// #include "app.h"    // Param


  /* -----------------
  stderr(regression) *is* a very procy good indicator of DA and non linearity.
    because they won't fit to a linear regression.
  -----------------
  */

// do we malloc(). No. the reading/writing shouldn't be responsible / defer allocation strategy.



// change name blob_header_t etc.
struct Header
{
  unsigned magic;
  unsigned len;
  unsigned id;
};

typedef struct Header Header;


void file_skip_to_end(  FILE *f);


void file_write_blob( FILE *f,    void (*pf)( FILE *, void *ctx ), void *ctx );

// int file_scan_blobs( FILE *f,  void (*pf)( Header *, void *ctx ), void *ctx );
int file_scan_blobs( FILE *f,  void (*pf)( FILE *f, Header *, void *ctx ), void *ctx );


#if 0
int file_scan_cal( FILE *f, Cal **cals, unsigned sz, unsigned *cal_id_max );
// int file_scan_cal( FILE *f, Cal **cal, unsigned sz );

void file_write_cal ( Cal *cal, FILE *f);
#endif

#if 0

void cal_show( Cal *cal /* FILE *f */ );


Cal * cal_create(void);
void cal_free( Cal *cal  );
Cal * cal_copy( Cal *cal  );


#endif



#if 0


MAT * m_read_flash( MAT *out, FILE *f);
int file_skip_to_last_valid(  FILE *f);


#endif



#if 0
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


#endif
