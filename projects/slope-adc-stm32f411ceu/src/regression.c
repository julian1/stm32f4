
#undef DEBUG



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
  http://homepage.divms.uiowa.edu/~dstewart/meschach/
  https://homepage.divms.uiowa.edu/~dstewart/meschach/html_manual/tutorial.html
  http://homepage.divms.uiowa.edu/~dstewart/meschach/html_manual/fnindex.html

*/


MAT *m_fill(  MAT *a, double *p )
{
  // printf( "m=%u n=%u\n", a -> m, a->n );
  // get_row(a);
  // get_col(a);

  for(unsigned i = 0; i < a->m; ++i)
  for(unsigned j = 0; j < a->n; ++j)
    m_set_val( a, i, j,  *p++ );

  return a;
}



MAT *m_hconcat( MAT *a, MAT *b, MAT *out )
{
  // should probably take the output as argument, then resize it. if needed.

  assert(a->m == b->m);
  // MAT *ret  = m_get( a->m , a->n + b->n );

  MAT *ret = m_resize( out, a->m, a->n + b->n);

  for(unsigned i = 0; i < a->m; ++i)
  for(unsigned j = 0; j < a->n; ++j)
    m_set_val( ret, i, j,  m_get_val( a, i, j ) );

  for(unsigned i = 0; i < b->m; ++i)
  for(unsigned j = 0; j < b->n; ++j)
    m_set_val( ret, i , j + a->n,  m_get_val( b, i, j ) );

  return ret;
}


void m_row_set( MAT *src, unsigned row, MAT *dst )
{
  // should probably take the output as argument, then resize it. if needed.
  // rows x cols

  assert(src->m == 1);      // single row. could create variant.
  assert(src->n == dst->n); // cols equal
  assert(row < src->m);     // row exists

   for(unsigned j = 0; j < src->n; ++j) {

    m_set_val( dst, row, j,  m_get_val( src, row, j ) );
  }

}









MAT * concat_ones( MAT *x, MAT *out)
{

  // TODO review memory handling here.
  MAT *j = m_get( x-> m, 1 ); 
  MAT *ones = m_ones( j );

  // MAT *ones = m_ones( m_copy( x, MNULL  ));
  // m_foutput(stdout, ones);

  MAT *ret =  m_hconcat( ones, x , out );

  M_FREE(ones);

  return ret;
}




MAT * regression( MAT *x, MAT * y, MAT *out)
{

  MAT *xt =  m_transp( x, MNULL  );
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


  M_FREE(x);
  M_FREE(x_);
  M_FREE(y);
  M_FREE(b);
  M_FREE(predicted);

  return 0;
}



#if 0
void regression(void)
{
  /*
    loop() subsumes update()
  */

    printf("WHOOT in regression()\n");

    MAT   *A = m_get(3,4);

    m_foutput(stdout, A );

    M_FREE(A);
}
#endif





