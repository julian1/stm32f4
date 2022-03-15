


#include <matrix.h>
#include "matrix2.h"   // inverse

#include "regression.h"

#include <assert.h>




/*
  if don't know size of something just pass in simple MNULL

  s == scalar.
  v == vector
  i == integer

  refs,
  https://maths.anu.edu.au/files/CMAProcVol32-Complete.pdf
  https://maths.anu.edu.au/files/CMAProcVol32-Complete.pdf
  http://homepage.divms.uiowa.edu/~dstewart/meschach/
  https://homepage.divms.uiowa.edu/~dstewart/meschach/html_manual/tutorial.html
  http://homepage.divms.uiowa.edu/~dstewart/meschach/html_manual/fnindex.html

*/

/*
  IMPORTANT - don't need to compute transpose separately. instead use


  // mmtr_mlt -- matrix-matrix transposed multiplication -- A.B^T is returned, and stored in OUT

  // mtrm_mlt -- matrix transposed-matrix multiplication -- A^T.B is returned, result stored in OUT


*/


#if 0
// element add
MAT	*m_add(const MAT *mat1, const MAT *mat2, MAT *out)
{
	unsigned int	m,n,i;

	if ( mat1==(MAT *)NULL || mat2==(MAT *)NULL )
		error(E_NULL,"m_add");
	if ( mat1->m != mat2->m || mat1->n != mat2->n )
		error(E_SIZES,"m_add");
	if ( out==(MAT *)NULL || out->m != mat1->m || out->n != mat1->n )
		out = m_resize(out,mat1->m,mat1->n);
	m = mat1->m;	n = mat1->n;
	for ( i=0; i<m; i++ )
	{
		__add__(mat1->me[i],mat2->me[i],out->me[i],(int)n);
		/**************************************************
		for ( j=0; j<n; j++ )
			out->me[i][j] = mat1->me[i][j]+mat2->me[i][j];
		**************************************************/
	}

	return (out);
}
#endif


// element matrix invert
// ie. for doing division. TODO handle divide by 0. 1/0.
MAT	*m_element_invert( const MAT *matrix, MAT *out)
{
	unsigned int	m,n,i, j;

	if ( matrix==(MAT *)NULL )
		error(E_NULL,"sm_mlt");
	if ( out==(MAT *)NULL || out->m != matrix->m || out->n != matrix->n )
		out = m_resize(out,matrix->m,matrix->n);
	m = matrix->m;	n = matrix->n;
	for ( i=0; i<m; i++ )
		// __smlt__(matrix->me[i],(double)scalar,out->me[i],(int)n);
		for ( j=0; j<n; j++ ) {

      if(matrix->me[i][j] == 0) {
        // TODO

      }

			out->me[i][j] = 1 / matrix->me[i][j];
    }


	return (out);
}


// element matrix mlt
// ie. avoids matrix mlt and taking the diagonal, which is expensive
MAT	*m_element_mlt(const MAT *mat1, const MAT *mat2, MAT *out)
{
	unsigned int	m,n,i, j;

	if ( mat1==(MAT *)NULL || mat2==(MAT *)NULL )
		error(E_NULL,"m_add");
	if ( mat1->m != mat2->m || mat1->n != mat2->n )
		error(E_SIZES,"m_add");
	if ( out==(MAT *)NULL || out->m != mat1->m || out->n != mat1->n )
		out = m_resize(out,mat1->m,mat1->n);
	m = mat1->m;	n = mat1->n;

	for ( i=0; i<m; i++ )
	{
		for ( j=0; j<n; j++ )
			out->me[i][j] = mat1->me[i][j] * mat2->me[i][j];
	}

	return (out);
}





MAT *m_fill(  MAT *a, double *p )
{
  // printf( "m=%u n=%u\n", a -> m, a->n );
  // get_row(a);
  // get_col(a);

  for(unsigned i = 0; i < a->m; ++i)  // rows
  for(unsigned j = 0; j < a->n; ++j)  // cols
    m_set_val( a, i, j,  *p++ );

  return a;
}



MAT *m_hconcat( MAT *a, MAT *b, MAT *out )
{
  // eg. to append ones matrix to the left, for doing a linear regression.
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



/*
  see. albeit these work with vec not mat.
  set_row() Set the row of a matrix to a given vector
  set_col() Set the column of a matrix to a given vector

  having the mat is easier. unless have mat_to_vec
*/

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



#if 1

MAT * concat_ones( MAT *x, MAT *out)
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


MAT * regression( MAT *x, MAT * y, MAT *out)
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


int regression_test()
{
  double xp[] = { 80, 100, 120, 140, 160, 180, 200, 220, 240, 260 };

  MAT *x_ =  m_fill( m_get(10, 1), xp );
  // m_foutput(stdout, x);

  double yp[] = {  70, 65, 90, 95, 110, 115, 120, 140, 155, 150  } ;
  MAT *y =  m_fill( m_get(10, 1), yp );
  // m_foutput(stdout, y);


  MAT *x =  concat_ones( x_, MNULL );
  MAT *b =  regression( x, y, MNULL );
  // m_foutput(stdout, b);


  MAT *predicted = m_mlt(x, b, MNULL );
  printf("predicted \n");
  m_foutput(stdout, predicted );


  // TODO assert() values...



  M_FREE(x);
  M_FREE(x_);
  M_FREE(y);
  M_FREE(b);
  M_FREE(predicted);

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

unsigned m_rows(MAT *m)
{
  return m->m;
}

unsigned m_cols(MAT *m)
{
  return m->n;
}



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





