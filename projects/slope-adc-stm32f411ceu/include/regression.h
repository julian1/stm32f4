
#pragma once

#include "matrix.h"   // MAT


// rename. to math.h or somethng.


/*

  - we can almost certainly write a function allocate on the stack as well.
      (but existing library functions will probably still allocate dynamically internally.
        check. may not need any extra stuff
      )

  double buf[1000];
  m_local_resize( mat, m, n, buf );
*/


// MAT	*m_element_invert( const MAT *matrix, MAT *out);

unsigned m_rows(const MAT *m) ;
unsigned m_cols(const MAT *m) ;


// element
MAT	*m_element_mlt(const MAT *mat1, const MAT *mat2, MAT *out);
MAT	*m_element_div(const MAT *mat1, const MAT *mat2, MAT *out);
MAT	*m_element_sub(const MAT *mat1, const MAT *mat2, MAT *out);


MAT	*m_from_scalar( double val, MAT *out );
MAT *m_diagonal( MAT *mat, MAT *out );

MAT *m_expand_rows( const MAT *mat, unsigned rows, MAT *out ) ;



// vertical
MAT	*m_sum(const MAT *mat1,  MAT *out);
MAT	*m_n(const MAT *mat,  MAT *out);
MAT	*m_sqrt( const MAT *matrix, MAT *out);
MAT	*m_mean( const MAT *mat, MAT *out  );

MAT	*m_var( const MAT *mat, unsigned w, MAT *out );
MAT	*m_stddev( const MAT *mat, unsigned w, MAT *out );

void m_stats_test(void);




MAT *m_fill(  MAT *a, double *p,  unsigned sz );
MAT *m_hconcat( MAT *a, MAT *b, MAT *out );


// MAT *m_regression( MAT *x, MAT * y, MAT *out);
// int m_regression_test(void);

bool m_is_scalar(const MAT *mat );
double d_from_scalar_m(const MAT *mat );


struct R
{
  MAT *b;

  MAT *predicted;

  unsigned df;

  double sigma2;

  MAT *var_cov_b ;

  MAT *var_b;

  MAT *stddev_b ;

  // 'correction for mean'
  double nybar2;

  double ess;
  double tss;

  double r2;

  double r ;

};

typedef struct R R;




void r_free( R *regression);
void r_report( R * regression, FILE *f );
int m_regression(  MAT *x, MAT *y,  R * regression );
int m_regression_test(void);




void m_print_details(MAT *m) ;


void m_foutput_binary( FILE *f, MAT *m  );
MAT * m_finput_binary(  FILE *f , MAT *out );

// required.
void m_row_set( MAT *src, unsigned row, MAT *dst );



// old...
// MAT * m_row_get( MAT *src, unsigned row, MAT *out );

// old. deprecate
// MAT * concat_ones( MAT *x, MAT *out);

