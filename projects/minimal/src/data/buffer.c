/*

  buffer holds data vals.  but also maintains the concept of its size.

*/

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>


#include <data/matrix.h>     // m_from_scalar

#include <data/buffer.h>



// perhaps better name resize()
// reset with argument


MAT * buffer_reset( MAT *buffer, uint32_t sz)
{
  printf("buffer_reset\n");
  // no magic for buffer, which may be NULL

  /* we should free and recreate buffer here - in order to free the memory.
      otherwise we can end up holding onto oversized allocation

    - on a large matrix,
      this frees the mesch data structure.
      although malloc() still hangs on to the reserved heap it took, like a page.
  */

  if(buffer) {

    M_FREE(buffer);
  }

  buffer    = m_resize( buffer, sz , 1 );   // rows x cols

  buffer    = m_zero( buffer ) ;    // just in case, probably not needed.
  buffer    = m_truncate_rows( buffer, 0 );

  assert(m_rows( buffer) == 0);

  printf("done buffer_reset\n");

  assert(buffer);
  return buffer;
}





void buffer_push( MAT *buffer, uint32_t *idx, double val )
{
  assert(buffer);


  if(m_rows(buffer) < m_rows_reserve(buffer)) {

    // just push onto sample buffer
    m_push_row( buffer, & val, 1 );
  }

  else {
    // buffer is full, so insert inplace
    unsigned imod = *idx % m_rows(buffer);
    // printf(" insert at %u\n", idx );
    m_set_val( buffer, imod, 0,  val );

    ++(*idx);
  }
}



#if 0


void buffer_print( MAT *buffer  )
{
  m_foutput(stdout, buffer);
} 
#endif



void buffer_stats_print( MAT *buffer /* double *mean, double *stddev */ )
{
  /*
    should just take some - doubles as arguments. .printing

    needs to return values, and used with better formatting instructions , that are not exposed here.
    format_float_with_commas()
  */
  assert(buffer);
  assert( m_cols(buffer) == 1);

  // take the mean of the buffer.
  MAT *mean = m_mean( buffer, MNULL );
  assert( m_is_scalar( mean ));
  double mean_ = m_to_scalar( mean);
  M_FREE(mean);



  MAT *stddev = m_stddev( buffer, 0, MNULL );
  assert( m_is_scalar( stddev ));
  double stddev_ = m_to_scalar( stddev);
  M_FREE(stddev);

  // report
  // char buf[100];
  // printf("value %sV ",          format_float_with_commas(buf, 100, 7, value));

  // printf("mean(%u) %.2fuV, ", m_rows(buffer),   mean_ * 1e6 );   // multiply by 10^6. for uV
  printf("mean(%u) %.7fV, ", m_rows(buffer),   mean_  );

  printf("stddev(%u) %.2fuV, ", m_rows(buffer), stddev_  * 1e6 );   // multiply by 10^6. for uV

  // printf("\n");

}



