
#include <stdio.h>
#include <assert.h>

#include <data.h>




#define DATA_MAGIC 123


void data_init ( data_t *data )
{
  data->magic = DATA_MAGIC;


  printf("whoot data init!\n");
}


void data_rdy_interupt( data_t *data) // runtime context
{
  /* interupt context.  don't do anything compliicated here.
    but called relatively infrequent.
  */

  assert(data);
  assert(data->magic == DATA_MAGIC) ;


  // if flag is still active, then record we missed processing some data.
  if(data->adc_measure_valid == true) {
    data->adc_measure_valid_missed = true;
    // ++data->adc_measure_valid_missed;     // count better? but harder to report.
  }

  // set adc_measure_valid flag so that update() knows to read the adc...
  data->adc_measure_valid = true;
}




void data_update(data_t *data)
{
  /* called in main loop.
    eg. 1M / s.
  */
  assert(data->magic == DATA_MAGIC) ;


/*
  static uint32_t count = 0;
  ++count;
  if(count > 1000000) {
    printf("%lu\n", count);
    count -= 1000000;
  } */


  if(data->adc_measure_valid ) {


    // clear flag as first thing, in order to better catch missed data, if get interupt while still processing
    data->adc_measure_valid  = false;

    // printf("got data\n");

  }
}


