

#include <ctype.h>        // toupper
#include <stdio.h>
#include <assert.h>
#include <math.h>       // fabs
#include <string.h>       // memcpy
#include <stdbool.h>





#include <lib2/util.h>  // UNUSED
#include <lib2/format.h>  // format_float


#include <support.h>         // format

#include <data/data.h>
#include <data/range.h>
#include <data/buffer.h>

// vfd
#include <peripheral/vfd.h> // magic
#include <peripheral/vfd-fonts.h>
#include <vfd-display.h>




// STTCPW




void vfd_display_init( vfd_display_t *vfd_display, vfd_t *vfd1, buffer_t *buffer)
{
  memset( vfd_display, 0, sizeof( vfd_display_t));
  vfd_display->magic = VFD_DISPLAY_MAGIC;

  assert( buffer && buffer->magic == BUFFER_MAGIC);

  vfd_display->vfd = vfd1;
  vfd_display->buffer = buffer;
}




void vfd_display_update( vfd_display_t *vfd_display)
{
  assert( vfd_display && vfd_display->magic == VFD_DISPLAY_MAGIC);

  // do nothing
}


void vfd_display_update_500ms( vfd_display_t *vfd_display)
{
  assert( vfd_display && vfd_display->magic == VFD_DISPLAY_MAGIC);

  // do nothing

}




void vfd_display_update_data( vfd_display_t *vfd_display, data_t *data)
{
  assert( vfd_display && vfd_display->magic == VFD_DISPLAY_MAGIC);
  assert( data && data->magic == DATA_MAGIC);


  vfd_t *vfd = vfd_display->vfd;
  assert( vfd);
  // assert(vfd && vfd->magic == VFD_MAGIC);

  buffer_t *buffer = vfd_display->buffer;
  assert( buffer && buffer->magic == BUFFER_MAGIC);



  // no valid measurement reading
  if( !data->valid) {

    // just clear the star, to indicate no measurement data
    char buf[101];
    snprintf( buf, 100, "%c", ' ');
    vfd_write_string2( vfd, buf, 0, 3 );

    return;
  }

  // only have range if data is valid
  const range_t *range = data->range;
  assert( range && range->magic == RANGE_MAGIC);



  // feb 2026


  char buf[100 + 1];
  // char buf2[100 + 1];

#if 0
  // from util.  should deprecate
  sprintf( buf, "%s%s",
    str_format_float_with_commas( buf2, 100, 7, data->reading),
    "" // range->unit
  );
#endif


  range->range_format_value( range, buf, 100, 8, data->reading);



  vfd_clear( vfd);


  // write value
  vfd_write_bitmap_string2( vfd, buf, 0 , 0 );



  // wont work. until we assert valid for a ZERO.
  uint8_t status_sample_idx = data->status.sample_idx;


  // write a star, for the sample
  vfd_write_string2( vfd, (status_sample_idx == 0) ? "x" : "z", 0, 4 );



  char star  = status_sample_idx % 2 == 1 ? '*' : ' ';


  /*
    TODO .  add the noaz. flag. to the status register
    so we can display mode. here. if AZ. or NOAZ.
    ---
    EXTR>  ... putting the 10Meg. flag in the status register as well
      easy if put in cr.
      so it is available in the display.
  */
  bool  noaz = 0;

  double nplc = aper_n_to_nplc( data->adc_clk_count_sigmux, data->line_freq );

  snprintf( buf, 100, "%c %s-%s %s   %s", star, range->name, range->arg,  noaz ? "NOAZ" : "AZ",   range->unit);
  vfd_write_string2( vfd, buf, 0, 3 );

// TODO.   the x offset is in pix. not characters.
//  snprintf( buf, 100, "here");
//  vfd_write_string2( vfd, buf, 10, 3 );



  snprintf( buf, 100, "nplc %.1lf ", nplc );
  vfd_write_string2( vfd, buf, 0, 4 );


  // str_format_float_with_commas(buf, 100, 7, data->reading);
  // snprintf( buf, 100, "n %u, mean %f", buffer->count, buffer->mean);
  snprintf( buf, 100, "mean   %.8f", buffer->mean);
  vfd_write_string2( vfd, buf, 0, 5 );


  char buf2[ 101];

  // this includes the unit
  snprintf( buf, 100, "stddev %s", str_format_value_dynamic( buf2, 100, buffer->stddev, 4 ));

  // snprintf( buf, 100, "stddev %.8f", buffer->stddev);
  vfd_write_string2( vfd, buf, 0, 6 );


}


/*
  if(status_seq_mode == SEQ_MODE_RATIO)
    printf(" meas %s", str_format_float_with_commas(buf, 100, 7, data->reading));
  else
    printf(" meas %sV", buf );
*/






