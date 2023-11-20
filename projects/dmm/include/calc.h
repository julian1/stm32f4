
// rename. to math.h or calc or somethng. or mcalc. this is just all very generalalized.


#pragma once

#include <stdbool.h>

#include "matrix.h"   // MAT




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

unsigned m_rows_reserve(const MAT *m);



void mat_set_row (  MAT *xs, unsigned row_idx,   MAT *whoot );
void vec_set_val (  MAT *xs, unsigned row_idx,   double x);



MAT * m_truncate_rows( MAT *m, size_t m_new );
void m_push_row( MAT *m, double *xs, size_t n );



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


//   // change name m_to_scalar. to m_to_scalar/real
double m_to_scalar(const MAT *mat );


struct R
{
  MAT *b;

  MAT *predicted;

  unsigned df;      // degrees of freedom

  double sigma2;    // variance of regression

  double sigma;     // stddev of regression

  MAT *var_cov_b ;  // variance/covariance matrix

  MAT *var_b;

  MAT *stddev_b ;   // stddev of the estimator. sqrt diagonal of var/covar matrix


  double nybar2;    // 'correction for mean'

  double ess;       // explained sum of squares
  double tss;       // total sum of squares

  double r2;

  double r;

};

typedef struct R R;



// TODO change name regression_free()
void r_free( R *regression);
void r_regression_show( const R * regression, FILE *f );

// TODO change name regression_run()
int m_regression( const MAT *x, const MAT *y,  R * regression );
int m_regression_test(void);

void m_octave_foutput( FILE *f, const char *format, const MAT *m);
int m_output_test(void);


void m_print_details(MAT *m) ;


void m_foutput_binary( FILE *f, const MAT *m  );
MAT * m_finput_binary(  FILE *f , MAT *out );

// required.
void m_row_set( MAT *src, unsigned row, MAT *dst );



// old...
// MAT * m_row_get( MAT *src, unsigned row, MAT *out );

// old. deprecate
// MAT * concat_ones( MAT *x, MAT *out);

