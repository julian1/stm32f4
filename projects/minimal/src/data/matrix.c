

#include <math.h>     // sqrt
#include <assert.h>



#include <mesch12b/matrix.h>

#include "data/matrix.h"



#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

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




unsigned m_rows_reserve(const MAT *m)
{
  return m->max_m;
}






void m_push_row( MAT *m, double *xs, size_t n )
{
  assert(  m_rows(m) < m_rows_reserve(m));

  for(unsigned i = 0; i < n; ++i )          // iterate columns
    m_set_val( m, m_rows(m), i,  xs[ i ] );

  ++m->m;

}


MAT * m_truncate_rows( MAT *m, size_t m_new )
{
  assert(m_new <= m_rows(m));   // can truncate to the same size.
  m->m = m_new;

  return m;
}







void mat_set_row (  MAT *xs, unsigned row_idx,   MAT *whoot )
{
  // set row. or push row.
  assert(xs);
  assert(whoot);
  assert(row_idx < m_rows(xs));


  assert( m_cols(whoot) == m_cols(xs) );
  assert( m_rows(whoot) == 1  );

  m_row_set( xs, row_idx, whoot );

}


void vec_set_val (  MAT *xs, unsigned row_idx,   double x)
{
  assert(xs);
  assert( m_cols(xs) == 1  );
  assert(row_idx < m_rows(xs));

  m_set_val( xs , row_idx, 0, x );

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


MAT *m_element_add(const MAT *mat1, const MAT *mat2, MAT *out)
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
      out->me[i][j] = mat1->me[i][j] + mat2->me[i][j];

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



bool m_is_scalar(const MAT *mat )
{

  // m_is_scalar( )
  return m_cols( mat) == 1 && m_rows(mat) == 1;   // scalar
}

double m_to_scalar(const MAT *mat )
{
  assert( m_is_scalar( mat));
  return m_get_val(mat, 0, 0);
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
    TODO. maybe. review.
      would be simller / faster / less mem to compute without MAT allocation.

    also we - generally only want return values as scalars
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





void m_octave_foutput( FILE *f, const char *format, const MAT *m  )
{
  // same format as matlab/ocatve
  // static const char    *format = "%14.9g ";
  // see matrixio.c
  if(!format)
    format = "%14.9g";

  fprintf(f, "[\n");

  for(unsigned i = 0; i < m_rows(m); ++i) {
    for(unsigned j = 0; j < m_cols(m); ++j) {

      double v = m_get_val( m, i, j);
      fprintf(f, format, v);

      if(j < m_cols(m) - 1)
        fprintf(f, ", ");
    }
    fprintf(f, ";\n");

    // needed. to avoid missing data.
    // need fflush(FILE*) or fsync();
    // Might be easier. to call usart1_flush();   inside
    // usart1_flush();

  }
  fprintf(f, "]\n");
}






int m_output_test()
{
  // octave > m =  [ 1, 2 ; 2, 4 ; 3, 7;  1, 4; 2, 3; 3, 5]
  double xp[] = { 1, 2, 2, 4, 3, 7, 1, 4, 2, 3, 3, 5 } ;
  MAT *m =  m_fill( m_get(6, 2), xp, ARRAY_SIZE(xp) );

  m_octave_foutput( stdout, NULL, m);

  return 0;
}






void m_print_details(MAT *m)
{
  // TODO consider rename m_show_detail ?


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


void m_foutput_binary( FILE *f, const MAT *m  )
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


