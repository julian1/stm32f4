/*

  very simple circular buffer for vals.
  use to calc some basic stats
    maintain own size


*/


#include <stdio.h>
#include <assert.h>
#include <string.h>     // strcmp


#include <lib3/util.h>      // MIN
#include <lib3/stats.h>
#include <lib3/format.h>  // format_with_commas



#include <data/data.h>
#include <support.h>      // str_format

#include <data/buffer.h>



/*
  consider, just use a C-api wrapped c++ stl deque.
  contiguous.
  but it will allocate/deallocate behind the scenes

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





void buffer_init( buffer_t *buffer, double *values, size_t max_sz )
{

  *buffer = (const buffer_t) {

    .magic    = BUFFER_MAGIC,
    .values   = values,
    .max_sz   = max_sz,
    .size     = 10,
    .verbose  = 1,
  };

}





/*
  could only do the modulo  for the indexing action,  to know how many values there are.
  but issue of integer wrap around...
  so use a separate variable
*/

void buffer_update_data( buffer_t *buffer, const data_t *data)
{

  assert(buffer && buffer->magic == BUFFER_MAGIC);
  assert( data && data->magic == DATA_MAGIC );



  if( data->status.sample.first) {

    // clear the current buffer
    buffer->i     = 0;
    buffer->count = 0;
  }


  if( !data->reading_valid
    || data->term.oob)
    return;


  assert( data->reading_valid && !data->term.oob);


  // printf("buffer i %u, count %u, ", buffer->i, buffer->count);

  // record value
  buffer->values[ buffer->i] = data->reading;

  // update index and count
  buffer->i     = ( buffer->i + 1) % buffer->size;
  buffer->count = MIN( buffer->count + 1, buffer->size) ;

  // calc some stats
  buffer->mean   = mean(   buffer->values, buffer->count);
  buffer->stddev = stddev( buffer->values, buffer->count);

  const range_t *range = data->range;
  assert(range);

  char buf[100 + 1];

  if( buffer->verbose) {

    printf( "mean %s ", str_format_float_with_commas(buf, 100, 8, buffer->mean));
    // printf( "%s, ", range->unit );

    printf("(n=%u/%u), ", buffer->count, buffer->size);

    // this includes the unit
    printf( "stddev %s", str_format_value_dynamic( buf, 100, buffer->stddev, 4 ));


    // printf( "stddev %s", str_format_float_with_commas(buf, 100, 8, buffer->stddev));
    // printf( "stddev %s", str_format_float_with_commas(buf, 100, 8, buffer->stddev));
    // printf( "%s, ", range->unit );
  }

}





bool buffer_repl_statement( buffer_t *buffer, const char *cmd)
{
  assert(buffer);
  assert(buffer->magic == BUFFER_MAGIC);
  UNUSED(cmd);

  uint32_t u0;

  // buffer verbosity
  if( sscanf(cmd, "buffer verbose %u", &buffer->verbose) == 1) {

  }

/*
  if( strcmp(cmd, "buffer show") == 0)
    buffer->show = true;

  else if( strcmp(cmd, "buffer unshow") == 0)
    buffer->show = false;
*/

  else if( sscanf(cmd, "buffer %lu", &u0 ) == 1) {

    if( u0 >= buffer->max_sz) {
      printf("exceeds buffer max_sz!\n");
      return 1;
    }


    buffer->size  = u0;

    /* just reset buffer, by clearing the index and count for now.
      preserving current buffer contents on resize action is tricky with modulo index
    */
    buffer->i     = 0;
    buffer->count = 0;

  }
  else
    return 0;


  return 1;
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




