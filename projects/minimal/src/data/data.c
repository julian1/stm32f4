/*
  consider rename this file. perhaps to measurement.
  and move the more cal specific functions to cal. perhaps.

*/
#include <stdio.h>
#include <assert.h>


#include <lib2/util.h>      // ARRAY_SIZE

#include <data/data.h>
#include <data/matrix.h>     // m_from_scalar
#include <data/buffer.h>     // m_from_scalar


#include <util.h>     // aper_n_to_period



#include <lib2/format.h>  // format_float

#include <peripheral/spi-ice40.h>


#include <device/spi-fpga0-reg.h>   // for seq mode



/*
    feb 2026.
  HANG ON. no reason to expose this structure in the header...
  when we are create ing it...
  ----

  because cal-flash accesses it.
  but that is not very good.
  ---------


  EXTR. feb 2026. solution would be to just move the repl.. code in here.

*/


// consider constrain in data.c
#define DATA_MAGIC 123



typedef struct data_t
{
  /*
  // TODO move to own file.
  // the MAT structures are annoying to deal with, cannot be easily opaquely prototyped.

  // EXTR> the cal structure should be injected into data.
  // then all the flash cal stuff can be kept independent.

  */

  uint32_t magic;


  // feb 2026.
  // move line_freq to app


  cal_t *cal;
  spi_t *spi ;

  bool show_counts;
  bool show_stats;
  bool show_extra;


} data_t;






data_t * data_create( cal_t * cal, spi_t *spi  )
{
  // called once at initialization

  data_t *data = malloc( sizeof(data_t));
  assert(data);
  memset( data, 0, sizeof( data_t));

  data->magic = DATA_MAGIC;
  // data->line_freq = 50;

  /*
    pass spi in constructor.  or at update time.

  */

  // TODO move this
  // buffer is separate external concept.  inject low level data into buffer.

/*
  data->buffer = buffer_reset( data->buffer, 10);
  assert( data->buffer);
  data_reset( data );
*/

  data->cal = cal;
  data->spi = spi;



  return data;
}









bool data_repl_statement( data_t *data,  const char *cmd )
{
  assert(data);
  assert(data->magic == DATA_MAGIC);


  // could be called, 'buffer show stats', 'buffer show extra' etc.

  if(strcmp(cmd, "data show counts") == 0)
    data->show_counts = 1;

  else if(strcmp(cmd, "data show extra") == 0)
    data->show_extra = 1;

  else if(strcmp(cmd, "data show stats") == 0)
    data->show_stats = 1;

#if 0
  else if(strcmp(cmd, "data cal show") == 0) {

    data_cal_show( data );

  }
#endif

  else
    return 0;

  return 1;
}







