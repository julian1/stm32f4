


#include <stdlib.h>     // malloc
#include <string.h>     // memcpy


#include <data/data.h>




void data_init( data_t *data )
{
  memset( data, 0, sizeof( data_t));
  data->magic = DATA_MAGIC;



}



