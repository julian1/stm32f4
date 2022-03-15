
#pragma once

// void regression(void);

#include "matrix.h"   // MAT

// typedef struct  MAT MAT;



MAT	*m_element_invert( const MAT *matrix, MAT *out);
MAT	*m_element_mlt(const MAT *mat1, const MAT *mat2, MAT *out);


MAT *m_fill(  MAT *a, double *p );
MAT *m_hconcat( MAT *a, MAT *b, MAT *out );


void m_row_set( MAT *src, unsigned row, MAT *dst );
MAT * m_row_get( MAT *src, unsigned row, MAT *out );

MAT * concat_ones( MAT *x, MAT *out);
MAT * regression( MAT *x, MAT * y, MAT *out);
int regression_test(void);


void m_print_details(MAT *m) ;

void m_extend_rows(MAT *m, unsigned m_new);

unsigned m_rows(MAT *m) ;
unsigned m_cols(MAT *m) ;
void m_foutput_binary( FILE *f, MAT *m  );
MAT * m_finput_binary(  FILE *f , MAT *out );



