/*

  buffers holds historic data vals.  to calculate stats
    maintain own size

  have access to data for value.  and flags like first
  that are used to clear

*/


#include <stdio.h>
#include <assert.h>
#include <stdlib.h>     // malloc
#include <string.h>     // memcpy


#include <lib2/util.h>      // UNUSED, ARRAY_SIZE
#include <lib2/stats.h>
#include <lib2/format.h>  // format_with_commas



#include <data/buffers.h>
#include <data/data.h>



/*
  should just use a wrapped deque.
  contiguous.  -

  and maintain order rather than a modul
  and ability to resize as we are working.
  -------------

  No. want allocation of max buffer once at the start.
  So we should pass the buffer as a dependency of buffers_t.
  -------------

  write modulus is  good.
  if really want a historically continguous buffer also
  by just decrementing backwards.  from the current point.
*/





void buffers_init( buffers_t *buffers, data_t *data, double *values, size_t n)
{
  assert(buffers);
  assert(data && data->magic == DATA_MAGIC);

  memset( buffers, 0, sizeof( buffers_t));
  buffers->magic = BUFFERS_MAGIC;

  assert(data);
  buffers->data = data;

  buffers->values = values;
  buffers->max_n = n;
}



// want a separate buffers_data_init();


/*
  could only do the modulo  for the indexing action,  to know how many values there are.
  but issue of integer wrap around...
  so use a separate variable
*/

void buffers_update( buffers_t *buffers)
{
  assert(buffers);
  assert(buffers->magic == BUFFERS_MAGIC);

  data_t *data = buffers->data;
  assert(data && data->magic == DATA_MAGIC);



  if(data->first) {

    assert(!data->valid);

    // clear data
    // memset( buffers->values, 0, sizeof(double) * buffers->max_n );

    buffers->n = 10;
    buffers->i = 0;
    buffers->size = 0;

  }

  if(data->valid) {
    assert(!data->first);
    // push buffers.

    // printf("buffers i %u, size %u, ", buffers->i, buffers->size );
    printf("(%u, %u), ", buffers->i, buffers->size );

    buffers->values[ buffers->i ] = data->value;

    buffers->i    = (buffers->i + 1 ) % buffers->n;
    buffers->size = MIN( buffers->size  + 1, buffers->n) ;


    double values_mean   = mean(   buffers->values, buffers->size);
    double values_stddev = stddev( buffers->values, buffers->size);

    char buf[100 + 1];

    // printf( "(n %u) ", buffers->size);
    printf( "mean   %s, ", str_format_float_with_commas(buf, 100, 8, values_mean));
    printf( "stddev %s, ", str_format_float_with_commas(buf, 100, 8, values_stddev));

  }

}








bool buffers_repl_statement( buffers_t *buffers, const char *cmd)
{
  assert(buffers);
  assert(buffers->magic == BUFFERS_MAGIC);

  UNUSED(cmd);


  return 0;
}










#if 0

// perhaps better name resize()
// reset with argument


MAT * buffer_init( MAT *buffer, uint32_t sz)
{
  printf("buffer_init\n");
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

  printf("done buffer_init\n");

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


#endif





