
// consider rename filter?

#include <stdio.h>
#include <assert.h>


#include <lib3/stats.h>


#include <data/data.h>
#include <data/aggregate.h>



void aggregate_init( aggregate_t *aggregate, double *values, size_t max_sz )
{

  *aggregate = (const aggregate_t) {

    .magic  = AGGREGATE_MAGIC,
    .values = values,
    .max_sz = max_sz,
    .size   = 1,
  };
}


void aggregate_update_data( aggregate_t *aggregate, /*const */ data_t *data)
{
  assert( aggregate && aggregate->magic == AGGREGATE_MAGIC);
  assert( data && data->magic == DATA_MAGIC );


  if( data->status.sample.first) {

    aggregate->i     = 0;
  }


  if( data->term.oob_aperture)
    return;


  if( aggregate->size == 0
    || aggregate->size == 1)
    return;



  if( data->reading_valid) {

    // store/record value
    aggregate->values[ aggregate->i] = data->reading;


    if( aggregate->i == aggregate->size - 1) {

      // buffer full, so compute mean, and override the value
      data->reading = mean( aggregate->values, aggregate->size);
      aggregate->i = 0;

      /*
      instead of clearing  the reading_valid flag.
      could just check this, and then read it, when need to consider whether to display the value.

      data->aggregate_valid = true;

      alternatively just have a aggregate_reading.  that is set to NAN.
      and forget the flags.
      */

    } else {

      // clear data for this update
      data->reading_valid = false;
      data->reading       = 0;

      ++aggregate->i;
    }
  }
}



bool aggregate_repl_statement( aggregate_t *aggregate, const char *cmd)
{
  assert(aggregate);
  assert(aggregate->magic == AGGREGATE_MAGIC);

  uint32_t u0;

  if( sscanf(cmd, "aggregate %lu", &u0) == 1) {

    assert(u0 < aggregate->max_sz);

    aggregate->size  = u0;
    aggregate->i     = 0;

  }
  else
    return 0;


  return 1;
}








