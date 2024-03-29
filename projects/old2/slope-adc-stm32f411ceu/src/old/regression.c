


#include <math.h>     // sqrt
#include <assert.h>
#include <stdbool.h>



#include <matrix.h>
#include "matrix2.h"   // inverse

#include "regression.h"


/*
  if don't know size of something just pass in simple MNULL

  s == scalar.
  v == vector
  i == integer

  refs,
  https://maths.anu.edu.au/files/CMAProcVol32-Complete.pdf
  http://homepage.divms.uiowa.edu/~dstewart/meschach/

  tutorial
  https://homepage.divms.uiowa.edu/~dstewart/meschach/html_manual/tutorial.html

  function index
  http://homepage.divms.uiowa.edu/~dstewart/meschach/html_manual/fnindex.html

*/

/*
  IMPORTANT - don't need to compute transpose separately. instead use

  // mmtr_mlt -- matrix-matrix transposed multiplication -- A.B^T is returned, and stored in OUT
  // mtrm_mlt -- matrix transposed-matrix multiplication -- A^T.B is returned, result stored in OUT


*/


#if 0
// element matrix invert
// ie. for doing division. TODO handle divide by 0. 1/0.

// actually not sure that we shouldn't just have m_element_div

MAT *m_element_invert( const MAT *mat, MAT *out)
{
  unsigned int  m,n,i, j;

  if ( mat==(MAT *)NULL )
    error(E_NULL,"sm_mlt");
  if ( out==(MAT *)NULL || out->m != mat->m || out->n != mat->n )
    out = m_resize(out,mat->m,mat->n);

  m = mat->m;  n = mat->n;

  for ( i=0; i<m; i++ )
  for ( j=0; j<n; j++ ) {

      assert( mat->me[i][j] != 0.);

      out->me[i][j] = 1 / mat->me[i][j];
    }


  return (out);
}
#endif


unsigned m_rows(const MAT *m)
{
  return m->m;
}

unsigned m_cols(const MAT *m)
{
  return m->n;
}




// element matrix mlt
// ie. avoids matrix mlt and taking the diagonal, which is expensive
// also use for squaring.
MAT *m_element_mlt(const MAT *mat1, const MAT *mat2, MAT *out)
{
  unsigned int  m,n,i, j;

  if ( mat1==(MAT *)NULL || mat2==(MAT *)NULL )
    error(E_NULL,"m_element_mlt");
  if ( mat1->m != mat2->m || mat1->n != mat2->n )
    error(E_SIZES,"m_element_mlt");
  if ( out==(MAT *)NULL || out->m != mat1->m || out->n != mat1->n )
    out = m_resize(out,mat1->m,mat1->n);

  m = mat1->m;  n = mat1->n;

  for ( i=0; i<m; i++ )
  for ( j=0; j<n; j++ )
      out->me[i][j] = mat1->me[i][j] * mat2->me[i][j];

  return (out);
}



MAT *m_element_div(const MAT *mat1, const MAT *mat2, MAT *out)
{
  unsigned int  m,n,i, j;

  if ( mat1==(MAT *)NULL || mat2==(MAT *)NULL )
    error(E_NULL,"m_element_div");
  if ( mat1->m != mat2->m || mat1->n != mat2->n )
    error(E_SIZES,"m_element_div");
  if ( out==(MAT *)NULL || out->m != mat1->m || out->n != mat1->n )
    out = m_resize(out,mat1->m,mat1->n);
  m = mat1->m;  n = mat1->n;

  for ( i=0; i<m; i++ )
  for ( j=0; j<n; j++ )
      out->me[i][j] = mat1->me[i][j] / mat2->me[i][j];

  return (out);
}


MAT *m_element_sub(const MAT *mat1, const MAT *mat2, MAT *out)
{
  unsigned int  m,n,i, j;

  if ( mat1==(MAT *)NULL || mat2==(MAT *)NULL )
    error(E_NULL,"m_element_sub");
  if ( mat1->m != mat2->m || mat1->n != mat2->n )
    error(E_SIZES,"m_element_sub");
  if ( out==(MAT *)NULL || out->m != mat1->m || out->n != mat1->n )
    out = m_resize(out,mat1->m,mat1->n);
  m = mat1->m;  n = mat1->n;

  for ( i=0; i<m; i++ )
  for ( j=0; j<n; j++ )
      out->me[i][j] = mat1->me[i][j] - mat2->me[i][j];

  return (out);
}









MAT *m_sum(const MAT *mat,  MAT *out)
{
  // same as octave/matlab. column orientated/ vertically.

  if ( mat ==(MAT *)NULL )
    error(E_NULL,"m_sum");

  if ( out==(MAT *)NULL || out->m != 1 || out->n != mat->n )
    out = m_resize(out, 1, mat->n);

  // assert(m_rows(out) == 1);
  // assert(m_cols(out) == m_cols(out));

  for ( unsigned j=0; j<mat->n; j++ )  { // cols

    double sum = 0;
    for ( unsigned i=0; i<mat->m; i++ )   // rows
        sum += mat->me[i][j] ;

    out->me[0][ j] = sum;
  }

  return (out);
}




MAT *m_n(const MAT *mat,  MAT *out)
{
  // vector for number of rows. for each col.
  // same as octave/matlab. column orientated/ vertically.

  if ( mat ==(MAT *)NULL )
    error(E_NULL,"m_sum");

  if ( out==(MAT *)NULL || out->m != 1 || out->n != mat->n )
    out = m_resize(out, 1, mat->n);

  // assert(m_rows(out) == 1);
  // assert(m_cols(out) == m_cols(out));

  for ( unsigned j=0; j<mat->n; j++ )  { // cols

    out->me[0][ j] = m_rows( mat );   // constant
  }

  return (out);
}









MAT *m_sqrt( const MAT *mat, MAT *out)
{
  // change name element sqrt?
  // printf("sqrt %u x %u\n", m_cols(mat), m_rows( mat )  );

  // element
  unsigned int  m,n,i, j;

  if ( mat==(MAT *)NULL )
    error(E_NULL,"m_sqrt");
  if ( out==(MAT *)NULL || out->m != mat->m || out->n != mat->n )
    out = m_resize(out,mat->m,mat->n);

  assert( m_cols(out) == m_cols(mat));
  assert( m_rows(out) == m_rows(mat));

  m = mat->m;  n = mat->n;

  for ( i=0; i<m; i++ )
  for ( j=0; j<n; j++ ) {


      out->me[i][j] = sqrt( mat->me[i][j]) ;  // is this being redefined somewhere
    }

  return (out);
}



bool m_is1x1(const MAT *mat )
{
  // 1x1 better name?

  // m_is1x1( )
  return m_cols( mat) == 1 && m_rows(mat) == 1;   // scalar
}




MAT *m_from_scalar( double val, MAT *out )
{

  if ( out==(MAT *)NULL || out->m != 1 || out->n != 1 )
    out = m_resize(out, 1, 1);

  m_set_val( out, 0, 0, val );

  return out;
}

MAT *m_diagonal( MAT *mat, MAT *out )
{
  assert(m_cols(mat) == m_rows(mat));

  if ( mat==(MAT *)NULL )
    error(E_NULL,"m_diagonal");

  if ( out==(MAT *)NULL || out->m != mat->m || out->n != 1 )
    out = m_resize(out,mat->m, 1);

  for( unsigned i = 0; i < m_rows(mat); ++i) {
    double val =  m_get_val( mat, i, i );
    m_set_val( out, i, 0, val );
  }


  return out;
}





// constant


MAT *m_mean( const MAT *mat, MAT *out  )
{
  /*
    TODO maybe
    - would be faster to just compute directly for each column.
    without creating sum and n . but it works.
  */
  // expected values
  MAT *sum = m_sum(mat, MNULL);
  MAT *n   = m_n(mat, MNULL);     // must be matrix.

  assert(m_rows(sum) == 1);
  assert(m_rows(n) == 1);
  assert(m_cols(sum) == m_cols(n));

  MAT *ret = m_element_div( sum, n, out); // eg. if out is useable it will be used.
                                          // and returned.
  M_FREE(sum);
  M_FREE(n);

  return ret;
}





MAT * m_expand_rows( const MAT *mat, unsigned rows, MAT *out )
{
  /*
    Change name m_repeat_rows()

    Note some cases, can use scalar multiply instead.
    MAT	*sm_mlt(double scalar, const MAT *matrix, MAT *out)

  */

  // repeat elements vertically
  // change name expand_vertical

  assert(m_rows(mat) == 1);

  if ( mat==(MAT *)NULL )
    error(E_NULL,"m_sqrt");

  if ( out==(MAT *)NULL || out->m != rows || out->n != mat->n )
    out = m_resize(out, rows ,mat->n);

  assert( m_cols(out) == m_cols(mat));
  assert( m_rows(out) == rows );

  // m = mat->m;  n = mat->n;

  for ( unsigned i=0; i<rows; i++ )  // rows
  for ( unsigned j=0; j<mat->n; j++ ) { // cols


      out->me[i][j] = mat->me[0][j] ;
    }

  return (out);

}







MAT *m_var( const MAT *mat, unsigned w, MAT *out )
{
  /*
    TODO. maybe.
      would be much faster to compute without allocation.
  */

  /*
    same as octave. support pop and sample
     // alternate calc approach. also works.

    std (x) = sqrt ( 1/(N-1) SUM_i (x(i) - mean(x))^2 )
    depends if use N or  N-1

  octave/
  https://octave.sourceforge.io/octave/function/std.html

    0: normalize with N-1, provides the square root of the best unbiased estimator of the variance [default]
    1: normalize with N, this provides the square root of the second moment around the mean

  matlab
    When w = 0 (default), the standard deviation is normalized by N-1, where N
    is the number of observations. When w = 1, the standard deviation is normalized
    by the number of observations

  }*/

  /*
    could eliminate/reduce allocation, with repeat calls.
    by passing in a structure with all variables - so that nothing gets reallocated.
    Or by redefining NEW to use alloca()
    except for the last instance etc.
  */

  if ( mat==(MAT *)NULL )
    error(E_NULL,"m_stddev");

  if ( out==(MAT *)NULL || out->m != 1 || out->n != mat->n )
    out = m_resize(out, 1, mat->n);


  MAT *mean = m_mean(mat , MNULL);  // 1 row

  MAT *mean_expand = m_expand_rows( mean, m_rows(mat), MNULL); // expand vertically in order to do element x(i) - mean(x)

  MAT *delta = m_element_sub(mat, mean_expand, MNULL);
  // printf("delta\n");
  // m_foutput(stdout, delta );

  MAT *delta_2 = m_element_mlt(delta, delta, MNULL);
  // printf("delta_2\n");
  // m_foutput(stdout, delta_2 );

  MAT *sum_delta_2 = m_sum( delta_2, MNULL);
  // printf("sum_delta_2\n");
  // m_foutput(stdout, sum_delta_2);

  MAT *ones = m_ones(  m_get( 1, m_cols(mat))) ;

  // 0 => use n-1,   1 => use m_zero
  MAT *denom =    w == 0 ? m_ones(  m_get( 1, m_cols(mat)) )
                : w == 1 ? m_zero(  m_get( 1, m_cols(mat)) )
                : (assert(0), NULL );

  MAT *n = m_n( mat, MNULL);
  MAT *n_sub_1 = m_element_sub( n, denom, MNULL);
  // printf("n_sub_1\n");
  // m_foutput(stdout, n_sub_1);

  MAT *n_div_n_sub_1 = m_element_div( ones, n_sub_1, MNULL);
  // printf("n_div_n_sub_1\n");
  // m_foutput(stdout, n_sub_1);

  MAT *mean_sum_delta_2 = m_element_mlt(n_div_n_sub_1, sum_delta_2, out );
  // printf("mean_sum_delta_2 \n");
  // m_foutput(stdout, mean_sum_delta_2);

  M_FREE(mean);
  M_FREE(mean_expand);
  M_FREE(delta);
  M_FREE(delta_2);
  M_FREE(sum_delta_2);
  M_FREE(ones);
  M_FREE(denom);
  M_FREE(n);
  M_FREE(n_sub_1);
  M_FREE(n_div_n_sub_1);

  return mean_sum_delta_2;

}


MAT *m_stddev( const MAT *mat, unsigned w, MAT *out )
{

  MAT *var = m_var(mat, w, MNULL );

  MAT *ret = m_sqrt(var, out );


  M_FREE(var);

  // need to handle out.
  return ret;
}





#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))


void m_stats_test()
{
  double v[] = { 1,2, 3, 4,5,7 };

  MAT *m = m_fill(  m_get(3, 2) , v, ARRAY_SIZE(v));
  m_foutput( stdout, m);


  printf("---------\n");
  printf("n\n");
  MAT *n = m_n( m, MNULL );
  m_foutput( stdout, n);


  printf("---------\n");
  printf("sum\n");
  MAT *sum = m_sum( m, MNULL );
  m_foutput( stdout, sum);

  printf("---------\n");
  printf("mean\n");
  MAT *mean = m_mean( m, MNULL );
  m_foutput( stdout, mean);



  // THIS IS WRONG. should be element x element
  printf("---------\n");
  printf("sqrt\n");
  MAT *sqrt = m_sqrt( m, MNULL );
  m_foutput( stdout, sqrt);



  printf("---------\n");
  printf("stddev\n");
  MAT *stddev = m_stddev( m, 0, MNULL );
  m_foutput( stdout, stddev);
}


















MAT *m_fill(  MAT *a, double *p,  unsigned sz )
{
  // printf( "m=%u n=%u\n", a -> m, a->n );

  // or could pass starting element (i,j) as argument
  assert( m_rows(a) * m_cols(a) == sz);

  for(unsigned i = 0; i < a->m; ++i)  // rows
  for(unsigned j = 0; j < a->n; ++j)  // cols
    m_set_val( a, i, j,  *p++ );

  return a;
}



MAT *m_hconcat( MAT *a, MAT *b, MAT *out )
{
  // ie. to append columns.
  // should probably take the output as argument, then resize it. if needed.

  assert(a->m == b->m);
  // MAT *ret  = m_get( a->m , a->n + b->n );

  MAT *ret = m_resize( out, a->m, a->n + b->n);
  assert(out == MNULL || ret == out);  // otherwise it's a potential leak??

  for(unsigned i = 0; i < a->m; ++i)
  for(unsigned j = 0; j < a->n; ++j)
    m_set_val( ret, i, j,  m_get_val( a, i, j ) );

  for(unsigned i = 0; i < b->m; ++i)
  for(unsigned j = 0; j < b->n; ++j)
    m_set_val( ret, i , j + a->n,  m_get_val( b, i, j ) );

  return ret;
}




#if 1

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

#endif


//
#if 0
MAT * m_regression( MAT *x, MAT * y, MAT *out)
{

  MAT *xt =  m_transp( x, MNULL  );     // TODO  see function that can do combined multipley/ transpose.
  // printf("xt is \n");
  // m_foutput(stdout, xt);

  MAT *temp0 = m_mlt(xt, x, MNULL );
  // printf("result of mult\n");
  // m_foutput(stdout, temp0);

  MAT *temp1 = m_mlt(xt, y, MNULL );

  MAT *temp2 =  m_inverse(temp0, MNULL);

  // MAT *temp3 = m_mlt(  temp2 , temp1, MNULL );
  MAT *ret = m_mlt(  temp2 , temp1, out );

  // printf("b \n");
  // m_foutput(stdout, ret );


  M_FREE(xt);
  M_FREE(temp0);
  M_FREE(temp1);
  M_FREE(temp2);

  return ret;
}
#endif


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



static bool float_equal(double a, double b, double epsilon )
{
    return fabs(a - b) < epsilon;
}


/*
  so we need a structure. for the resut....

*/


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

  /////////////
  // work out b
  MAT *xtx    = mtrm_mlt(x, x, MNULL );
  MAT *xtxi   = m_inverse(xtx, MNULL);
  MAT *temp1  = mtrm_mlt(x, y, MNULL );
  MAT *b      = m_mlt(  xtxi , temp1, MNULL );


  // MAT *b =  m_regression( x, y, MNULL );
  // m_foutput(stdout, b);

  printf("b\n");
  m_foutput(stdout, b );
  // row 0:     24.4545455
  // row 1:    0.509090909
  assert( float_equal( m_get_val( b, 0, 0), 24.4545455, e ))  ;
  assert( float_equal( m_get_val( b, 0, 1), 0.509090909, e ))  ;


  /*
  MAT *predicted = m_mlt(x, b, MNULL );
  printf("predicted \n");
  m_foutput(stdout, predicted );
  */


  //////////////
  // work out theta2
  // utu = yty - btxty

  MAT *yty    = mtrm_mlt(y, y, MNULL );
  MAT *xty    = mtrm_mlt(x, y, MNULL );
  MAT *btxty  = mtrm_mlt(b, xty, MNULL );

  // printf("yty\n");
  // m_foutput(stdout, yty );  // correckkt

  // printf("xty\n");
  // m_foutput(stdout, xty );  // correckkt

  // printf("btxty\n");
  // m_foutput(stdout, btxty);  // correckkt

  MAT *utu = m_element_sub( yty , btxty, MNULL ); // utu = yty - btxty correct
  // printf("utu \n");
  // m_foutput(stdout, utu);  // correckkt

  assert( float_equal( m_get_val( utu, 0, 0), 337.272727, e ))  ;

  // unsigned n = m_rows(y)
  unsigned df =  m_rows(y) - m_rows(b);   // n - k
  assert(df == 8);
  printf("df %u \n", df );

  double theta2 = m_get_val( utu, 0, 0) / df;   // utu / (n - k)
  assert( float_equal( theta2, 42.159091, e ))  ;
  printf("theta2 %lf \n", theta2);


  ///////////////////////////////
  // variance / covariance matrix of B

  MAT *var_cov_b = sm_mlt( theta2, xtxi, MNULL );   // this is a scalar
  printf("var_cov_b\n");
  m_foutput(stdout, var_cov_b);  // correct

  // p82.
  assert( float_equal( m_get_val( var_cov_b, 0, 0), 41.1370523 , e ))  ;
  assert( float_equal( m_get_val( var_cov_b, 1, 1), 0.00127754821, e ))  ;

  // var_b is the diagonal of the var_cov_b
  MAT *var_b = m_diagonal( var_cov_b, MNULL);
  printf("var_b\n");
  m_foutput(stdout, var_b);

  MAT *stddev_b = m_sqrt( var_b, MNULL);
  printf("stddev_b\n");
  m_foutput(stdout, stddev_b);

  // p82
  assert( float_equal( m_get_val( stddev_b, 0, 0), 6.4138173, e ))  ;
  assert( float_equal( m_get_val( stddev_b, 1, 0), 0.0357428064, e ))  ;

  //////////////////
  // r2. coefficient of determination.
  // = ESS / TSS = (btxty - ny2 ) / (y'y - ny2 )

  MAT *ybar = m_mean(y, MNULL);
  assert( m_is1x1( ybar) );   // scalar

  // printf("y_bar/ymean\n");
  // m_foutput(stdout, ybar);

  double ybar_ = m_get_val(ybar, 0, 0 );

  // 'correction for mean'
  double nybar2 = m_rows(y) *  ybar_ * ybar_ ;

  // printf("nybar2\n");
  // printf("%f\n" ,  nybar2);

  assert( m_is1x1( btxty));
  assert( m_is1x1( yty));

  double ess = m_get_val(btxty, 0, 0) - nybar2;
  double tss = m_get_val(yty  , 0, 0) - nybar2;

  double r2 = ess / tss;
  printf("r2 %f\n" ,  r2 );

  double r = sqrt( r2);
  printf("r %f\n" ,  r );


  /////////////

  // TODO f stat.



  // TODO assert() values...
  M_FREE(x);
  M_FREE(x_);
  M_FREE(y);
  M_FREE(b);

  return 0;
}








void m_print_details(MAT *m)
{

  assert(m);
  assert(  *(m->me) == m->base);  // first value of handler == base memory

  printf( "m %u, n %u, ", m->m, m->n );
  printf( "max_m %u, max_n %u\n", m->max_m, m->max_n );

/*
  // me appear to be row pointers
  printf( "me   %p\n", m->me  );
  printf( "*me  %p\n", *m->me  );
  printf( "base %p\n", m->base );

  printf( "me[0]  %p\n", (m->me)[0]  );
  printf( "me[1]  %p\n", (m->me)[1]  );
  printf( "me[0]  %p\n", (m->me)[0]  );
  printf( "me[1] - me[0]  %ld\n", m->me[1] - m->me[0]  );   // row pointers
*/
}


// Actually I think we can just call m_resize()


#if 0

void m_extend_rows(MAT *m, unsigned m_new)
{
  // extend.
  // without reallocation.  assumes already oversize.
  // avoid calling m_resize repeadedly because may be expensive... because of having to reconstruct the me base pointers.

  assert( m->m + m_new <= m->max_m );

  // see code for m_resize().  risk that is expensive... due to having to reconstruct the me base pointers.
  // m_resize( m, m->m + 1, m->n );

  m->m += m_new;

}
#endif



/*
  IMPORTANT.
  if need to have the message size, to incorporate in header.
  then can just call this twice..
  once to compute size. and then to properly write.
*/


void m_foutput_binary( FILE *f, MAT *m  )
{
  // m_serialize()
  // size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

  size_t items;

  items = fwrite( &m->m, sizeof(m->m), 1, f);
  assert(items == 1);

  items = fwrite( &m->n, sizeof(m->n), 1, f);
  assert(items == 1);

  assert( sizeof(Real) == sizeof(double));

  for(unsigned i = 0; i < m->m; ++i)
  for(unsigned j = 0; j < m->n; ++j) {

    double v = m_get_val( m, i, j);

    items = fwrite( &v, sizeof(v), 1, f);
    assert(items == 1);
  }
}



MAT * m_finput_binary(  FILE *f , MAT *out )
{
  // m_deserialize()
  // size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);

  int items;

  unsigned m, n ;
  items = fread( &m, sizeof(m), 1, f);
  assert(items == 1);

  items = fread( &n, sizeof(n), 1, f);
  assert(items == 1);

  MAT *ret = m_resize( out, m, n );

  // not tested
  for(unsigned i = 0; i < ret->m; ++i)
  for(unsigned j = 0; j < ret->n; ++j) {

    double v ;
    items = fread( &v, sizeof(v), 1, f);
    assert(items == 1);

    m_set_val( ret, i, j, v);
  }

  return ret;
}







void m_row_set( MAT *dst, unsigned row, MAT *src )  // note argument order is kind of reversed
{
  // should probably take the output as argument, then resize it. if needed.
  // rows x cols
  assert(dst);
  assert(src);

  assert(src->m == 1);      // single row. could create variant.
  assert(src->n == dst->n); // cols equal
  assert(row < dst->m);     // row exists

  for(unsigned j = 0; j < src->n; ++j) {

    double x = m_get_val( src, 0, j );
    m_set_val( dst, row, j,  x );
  }
}

#if 0
/*
  see. albeit these work with vec not mat.
  set_row() Set the row of a matrix to a given vector
  set_col() Set the column of a matrix to a given vector

  having the mat is easier. unless have mat_to_vec
*/




MAT * m_row_get( MAT *src, unsigned row, MAT *out )
{

  MAT *ret = m_resize( out, 1, src->n );
  assert(out == MNULL || ret == out);  // otherwise it's a potential leak??

  for(unsigned j = 0; j < src->n; ++j) {

    double x = m_get_val( src, row, j );
    m_set_val( ret, 0, j,  x );
  }

  return ret;
}
#endif


