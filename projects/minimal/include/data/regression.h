
#pragma once

#include <mesch12b/matrix.h>


struct regression_t
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

typedef struct regression_t regression_t ;



// TODO change name regression_free()
void r_free( regression_t *regression);
void r_regression_show( const regression_t * regression, FILE *f );

// TODO change name regression_run() or m_cal_regression_params() 
int m_regression( const MAT *x, const MAT *y,  regression_t * regression );
int m_regression_test(void);

