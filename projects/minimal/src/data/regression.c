

#include <math.h>     // sqrt
#include <assert.h>


#include <data/matrix.h>      // m_rows()
#include <data/regression.h>

#include <mesch12b/matrix2.h>   // inverse TODO - review needed in the header?



#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))







static MAT * m_concat_ones( MAT *x, MAT *out)
{
  // concat a ones field to lhs of mat.
  // note. probably should avoid.
  // just add the 1 constant when adding a row

  // TODO review memory handling here.
  MAT *j = m_get( x-> m, 1 );
  MAT *ones = m_ones( j );

  // MAT *ones = m_ones( m_copy( x, MNULL  ));
  // m_foutput(stdout, ones);

  MAT *ret =  m_hconcat( ones, x , out );

  M_FREE(ones);

  return ret;
}



#if 0

MAT * m_regression( MAT *x, MAT * y, MAT *out)
{

  MAT *xtx = mtrm_mlt(x, x, MNULL );    // matrix transpose x matrix

  MAT *xtxi = m_inverse(xtx, MNULL);

  MAT *temp1 = mtrm_mlt(x, y, MNULL );    // matrix transpose x matrix


  MAT *b  = m_mlt(  xtxi , temp1, out );

  // printf("b \n");
  // m_foutput(stdout, ret );


  M_FREE(xtx);
  M_FREE(temp1);
  M_FREE(xtxi);

  return b;
}
#endif




int m_regression( const MAT *x, const MAT *y,  regression_t * regression )
{
  /*
      example from "basic econometrics" p 291.
      and chap 3. p82
  */
  /////////////
  // work out b
  MAT *xtx      = mtrm_mlt(x, x, MNULL );
  MAT *xtxi     = m_inverse(xtx, MNULL);
  MAT *temp1    = mtrm_mlt(x, y, MNULL );
  regression->b = m_mlt(  xtxi , temp1, MNULL );

  assert( m_rows(regression->b) == m_cols( x) );

  // remember predicted, needs to be adjusted by aperture
  regression->predicted = m_mlt(x, regression->b, MNULL );

  ///////////////////////////////
  // work out sigma2
  // utu = yty - btxty

  MAT *yty_     = mtrm_mlt(y, y, MNULL );
  MAT *xty      = mtrm_mlt(x, y, MNULL );
  MAT *btxty_   = mtrm_mlt(regression->b, xty, MNULL );

  assert( m_is_scalar( yty_ ));
  assert( m_is_scalar( btxty_));

  double yty   = m_to_scalar(yty_ );
  double btxty = m_to_scalar(btxty_);

  // utu = yty - btxty
  double utu          = yty - btxty;

  // df = n - k
  regression->df      =  m_rows(y) - m_rows( regression->b );

  // utu / (n - k)
  regression->sigma2  = utu / regression->df;

  // standard error of regression
  regression->sigma   = sqrt( regression->sigma2 );

  ///////////////////////////////
  // variance / covariance matrix of B

  regression->var_cov_b = sm_mlt( regression->sigma2, xtxi, MNULL );

  // var_b is the diagonal of the var_cov_b
  regression->var_b     = m_diagonal( regression->var_cov_b, MNULL);

  regression->stddev_b  = m_sqrt( regression->var_b, MNULL);


  //////////////////
  // r2. coefficient of determination.
  // = ESS / TSS = (btxty - ny2 ) / (y'y - ny2 )

  MAT *ybar = m_mean(y, MNULL);
  assert( m_is_scalar( ybar) );   // scalar
  double ybar_ = m_to_scalar( ybar );

  // 'correction for mean'
  regression->nybar2  = m_rows(y) *  ybar_ * ybar_ ;

  regression->ess     = btxty  - regression->nybar2;

  regression->tss     = yty  - regression->nybar2;

  regression->r2      = regression->ess / regression->tss;

  regression->r       = sqrt( regression->r2);


  /////////////
  // TODO f stat.

  // free up everything
  M_FREE( xtx);
  M_FREE( xtxi);
  M_FREE( temp1);

  M_FREE( yty_);
  M_FREE( xty );
  M_FREE( btxty_);

  M_FREE( ybar);

  return 0;
}




void r_free( regression_t *regression)
{

  M_FREE( regression->b);
  M_FREE( regression->predicted );
  M_FREE( regression->var_cov_b );
  M_FREE( regression->var_b );
  M_FREE( regression->stddev_b );

}




void r_regression_show( const regression_t * regression, FILE *f )
{
  // change name print? no . its for a structure
  // could pass the stream

  fprintf(f, "b\n");
  m_foutput(f, regression->b);

  // needs to be flushed also.
  // fprintf(f, "\npredicted\n");
  // m_foutput(f, regression->predicted);

  fprintf(f, "\ndf     %u\n", regression->df);

  fprintf(f, "sigma2 %f\n", regression->sigma2);

  fprintf(f, "sigma  %f\n", regression->sigma);



  fprintf(f, "\nvar_cov_b\n");
  m_foutput(f, regression->var_cov_b);

  fprintf(f, "\nvar_b\n");
  m_foutput(f, regression->var_b);

  fprintf(f, "\nstddev_b\n");
  m_foutput(f, regression->var_b);

  // 'correction for mean'
  fprintf(f, "\nnybar2 %f\n", regression->nybar2);

  fprintf(f, "ess    %f\n", regression->ess);

  fprintf(f, "tss    %f\n", regression->tss);

  fprintf(f, "r2     %.10f\n", regression->r2);

  fprintf(f, "r      %.10f\n", regression->r);

}








static bool float_equal(double a, double b, double epsilon )
{
    return fabs(a - b) < epsilon;
}





int m_regression_test()
{
  /*
      example from "basic econometrics" p 291.
      and chap 3. p82
  */
  double xp[] = { 80, 100, 120, 140, 160, 180, 200, 220, 240, 260 };
  MAT *x_ =  m_fill( m_get(10, 1), xp, ARRAY_SIZE(xp) );
  // m_foutput(stdout, x);

  double yp[] = {  70, 65, 90, 95, 110, 115, 120, 140, 155, 150  } ;
  MAT *y =  m_fill( m_get(10, 1), yp, ARRAY_SIZE(yp) );
  // m_foutput(stdout, y);

  double e = 0.00001;

  MAT *x      =  m_concat_ones( x_, MNULL );


  regression_t regression;
  memset( &regression, 0, sizeof(regression));

  m_regression(  x, y,  &regression );

  // row 0:     24.4545455
  // row 1:    0.509090909
  assert( float_equal( m_get_val( regression.b, 0, 0), 24.4545455, e ))  ;
  assert( float_equal( m_get_val( regression.b, 0, 1), 0.509090909, e ))  ;

  // assert( float_equal( m_get_val( utu, 0, 0), 337.272727, e ))  ;

  assert(regression.df == 8);

  assert( float_equal( regression.sigma2, 42.159091, e ))  ;

  // p82.
  assert( float_equal( m_get_val( regression.var_cov_b, 0, 0), 41.1370523 , e ))  ;
  assert( float_equal( m_get_val( regression.var_cov_b, 1, 1), 0.00127754821, e ))  ;

  // p82
  assert( float_equal( m_get_val( regression.stddev_b, 0, 0), 6.4138173, e ))  ;
  assert( float_equal( m_get_val( regression.stddev_b, 1, 0), 0.0357428064, e ))  ;


  // p 82
  assert( float_equal( regression.r2, 0.962062, e ))  ;
  assert( float_equal( regression.r, 0.980847, e ))  ;


  r_regression_show( &regression, stdout );

  r_free( &regression);

  return 0;
}



