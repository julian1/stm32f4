
#include <stdio.h>
#include <assert.h>

#include <data.h>




#define DATA_MAGIC 123


void data_init ( data_t *data )
{
  data->magic = DATA_MAGIC;

}


void data_rdy_interupt( data_t *data)
{
  // interupt context. don't do anything compliicated here.

  assert(data);
  assert(data->magic == DATA_MAGIC) ;


//   printf("got data\n");


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
  assert(data->magic == DATA_MAGIC) ;

  if(data->adc_measure_valid ) {

    // clear flag as first thing, in order to better catch missed data, if get interupt while still processing
    data->adc_measure_valid  = false;

  }
}


