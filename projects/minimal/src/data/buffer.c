/*

  buffer to hold data vals.  and calculate basic stats
    maintain own size

  should access to data for value.  and flags like first
  that are used to clear


  could just pass the status_register on data update.

*/


#include <stdio.h>
#include <assert.h>
#include <stdlib.h>     // malloc
#include <string.h>     // memcpy


#include <lib2/util.h>      // UNUSED, ARRAY_SIZE
#include <lib2/stats.h>
#include <lib2/format.h>  // format_with_commas



#include <data/decode.h>
#include <data/buffer.h>
#include <data/range.h>



/*
  should just use a wrapped deque.
  contiguous.  -

  and maintain order rather than a modul
  and ability to resize as we are working.
  -------------

  No. want allocation of max buffer once at the start.
  So we should pass the buffer as a dependency of buffer_t.
  -------------

  write modulus is  good.
  if really want a historically continguous buffer also
  by just decrementing backwards.  from the current point.
*/





void buffer_init( buffer_t *buffer, decode_t *data, double *values, size_t max_n )
{

/*
  could inject range here.

*/

  assert(buffer);
  assert(data && data->magic == DECODE_MAGIC);

  memset( buffer, 0, sizeof( buffer_t));
  buffer->magic = BUFFERS_MAGIC;

  assert(data);
  buffer->data = data;

  buffer->values = values;
  buffer->max_n = max_n;

  // initial buffer size
  buffer->size = 10;

  buffer->show = true;
}



// want a separate buffer_decode_init();


/*
  could only do the modulo  for the indexing action,  to know how many values there are.
  but issue of integer wrap around...
  so use a separate variable
*/

void buffer_update( buffer_t *buffer)
{
  assert(buffer);
  assert(buffer->magic == BUFFERS_MAGIC);

  decode_t *data = buffer->data;
  assert(data && data->magic == DECODE_MAGIC);

  range_t *range = &data->ranges[ *data->range_idx ];
  assert(range);



  if(data->status.first) {

    assert(!data->valid);

    // could clear data
    // memset( buffer->values, 0, sizeof(double) * buffer->max_n );

    buffer->i = 0;
    buffer->count = 0;
  }

  if(data->valid) {

    assert(!data->status.first);
    // push buffer.

    // printf("buffer i %u, count %u, ", buffer->i, buffer->count);
    if(buffer->show)
      printf("(%u, %u), ", buffer->i, buffer->count);

    // record value
    buffer->values[ buffer->i ] = data->reading;

    // update index and count
    buffer->i     = (buffer->i + 1 ) % buffer->size;
    buffer->count = MIN( buffer->count + 1, buffer->size) ;

    // calc some stats
    buffer->mean   = mean(   buffer->values, buffer->count);
    buffer->stddev = stddev( buffer->values, buffer->count);

    char buf[100 + 1];

    if(buffer->show) {
      // printf( "(n %u) ", buffer->count);
      printf( "mean   %s", str_format_float_with_commas(buf, 100, 8, buffer->mean));
      printf( "%s, ", range->unit );

      printf( "stddev %s", str_format_float_with_commas(buf, 100, 8, buffer->stddev));
      printf( "%s, ", range->unit );
    }
  }

}





bool buffer_repl_statement( buffer_t *buffer, const char *cmd)
{
  assert(buffer);
  assert(buffer->magic == BUFFERS_MAGIC);
  UNUSED(cmd);

  uint32_t u0;



  if(strcmp(cmd, "buffer show") == 0)
    buffer->show = true;

  else if(strcmp(cmd, "buffer unshow") == 0)
    buffer->show = false;




  if( sscanf(cmd, "buffer size %lu", &u0 ) == 1) {

    assert(u0 < buffer->max_n);

    buffer->size = 10;

    // reset buffer, by clearing the index and count...
    // preserving buffer contents on resize is tricky with modulo index
    buffer->i = 0;
    buffer->count = 0;

  }


  return 0;
}







#if 0



bool decode_repl_statement( decode_t *data,  const char *cmd )
{
  assert(data);
  assert(data->magic == DECODE_MAGIC);


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
    decode_reset( data );
  }

  else if( strcmp(cmd, "data buffer reset") == 0) {

    // set buffer size, efault
    data->buffer = buffer_reset( data->buffer, 10 );
    assert( data->buffer);
    decode_reset( data );
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


    decode_cal_show( data );

  }

  else
    return 0;

  return 1;


}


#endif




