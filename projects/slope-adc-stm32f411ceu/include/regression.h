
#pragma once

// void regression(void);

MAT *m_fill(  MAT *a, double *p );
MAT *m_hconcat( MAT *a, MAT *b, MAT *out );


void m_row_set( MAT *src, unsigned row, MAT *dst );
MAT * m_row_get( MAT *src, unsigned row, MAT *out );

MAT * concat_ones( MAT *x, MAT *out);
MAT * regression( MAT *x, MAT * y, MAT *out);
int regression_test(void);



