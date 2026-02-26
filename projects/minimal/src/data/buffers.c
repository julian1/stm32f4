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


    buffers->mean   = mean(   buffers->values, buffers->size);
    buffers->stddev = stddev( buffers->values, buffers->size);

    char buf[100 + 1];

    // printf( "(n %u) ", buffers->size);
    printf( "mean   %s, ", str_format_float_with_commas(buf, 100, 8, buffers->mean));
    printf( "stddev %s, ", str_format_float_with_commas(buf, 100, 8, buffers->stddev));

  }

}








bool buffers_repl_statement( buffers_t *buffers, const char *cmd)
{
  assert(buffers);
  assert(buffers->magic == BUFFERS_MAGIC);
  UNUSED(cmd);

  uint32_t u0;


  if( sscanf(cmd, "data buffer size %lu", &u0 ) == 1) {

#if 0
    // if(u0 < 2 || u0 > 500 ) {
    if(u0 < 1 || u0 > 10000 ) {
      printf("set buffer size bad arg\n" );
      return 1;
    }

    // set buffer size, efault
    data->buffer = buffer_reset( data->buffer, u0);

    assert( data->buffer);
    data_reset( data );
#endif
  }


  return 0;
}







#if 0
bool data_repl_statement( data_t *data,  const char *cmd )
{
  assert(data);
  assert(data->magic == DATA_MAGIC);


  // prefix with data.  same as the struct, function prefix. eg.   'data cal'  'data buffer size x' ?
  uint32_t u0;


  // reserve or resize...

  if( sscanf(cmd, "data buffer size %lu", &u0 ) == 1) {

    // if(u0 < 2 || u0 > 500 ) {
    if(u0 < 1 || u0 > 10000 ) {
      printf("set buffer size bad arg\n" );
      return 1;
    }

    // set buffer size, efault
    data->buffer = buffer_reset( data->buffer, u0);

    assert( data->buffer);
    data_reset( data );
  }

  else if( strcmp(cmd, "data buffer reset") == 0) {

    // set buffer size, efault
    data->buffer = buffer_reset( data->buffer, 10 );
    assert( data->buffer);
    data_reset( data );
  }



/*
  else if( strcmp(cmd, "data buffer print")) {
    // dump all vals.
    // print/show?
  }
*/

  else if( sscanf(cmd, "data line freq %lu", &u0 ) == 1) {

    if(  !(u0 == 50 || u0 == 60)) {
      // be safe for moment.
      printf("bad line freq arg\n" );
      return 1;
    }

    // printf("set lfreq\n" );
    data->line_freq = u0;
  }

  else if(strcmp(cmd, "data null") == 0) {
    // todo
    // eg. very easy, but useful. add an offset based on last value

  }
  else if(strcmp(cmd, "data div") == 0) {
    // div by gain.before
    // should probably be gain and offset for the calibration.
    // todo
  }

  // could be called, 'buffer show stats', 'buffer show extra' etc.

  else if(strcmp(cmd, "data show counts") == 0)
    data->show_counts = 1;

  else if(strcmp(cmd, "data show extra") == 0)
    data->show_extra = 1;

  else if(strcmp(cmd, "data show stats") == 0)
    data->show_stats = 1;


  else if(strcmp(cmd, "data cal show") == 0) {


    data_cal_show( data );

  }

  else
    return 0;

  return 1;


}


#endif




