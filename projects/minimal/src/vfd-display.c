

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


  // feb 2026

  // no valid measurement reading
  if( !data->valid) {

    // clear the reading star, to indicate no measurement data
    char buf[101];
    snprintf( buf, 100, "%c", ' ');
    // vfd_write_string2( vfd, buf, 0, 3 );

    vfd_write_string2( vfd, buf, 0, 7 );

    return;
  }

  // range only available if data valid
  const range_t *range = data->range;
  assert( range && range->magic == RANGE_MAGIC);


  vfd_clear( vfd);


  /*
    extr. could do this formatting once, at the time of decode.
  */

  format_val_t  val;
  range->range_reading_format( range, &val, 9, data->reading);


  // write sign in small font
  vfd_write_string2( vfd, data->reading >= 0 ? "+" : "-", 0, 0 );

  /*
    vfd. would be really nice to,  intersperse commas, or even just a column of pixels herue.
  */

  // write value in alrge font, starting with offset for the sign
  // vfd_write_bitmap_string_proportional( vfd, val.s, 7, 0 );
  vfd_write_bitmap_string_mono( vfd, val.s, 7, 0 );


  char buf[ 100 + 1 ];

  // format the value multiplier and unit, left align, chars are 7bit wide.
  snprintf( buf, 100, "%c%s", val.m, val.u);
  vfd_write_string2( vfd, buf, (18 - strlen( buf)) * 7, 3 );




  // wont work. until we assert valid for a ZERO.
  uint8_t status_sample_idx = data->status.sample_idx;


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

  snprintf( buf, 100, "%s-%s %s",  range->name, range->arg,  noaz ? "NOAZ" : "AZ" );
  vfd_write_string2( vfd, buf, 0, 3 );



  snprintf( buf, 100, "nplc %.1lf ", nplc );
  vfd_write_string2( vfd, buf, 0, 4 );


  // str_format_float_with_commas(buf, 100, 7, data->reading);
  // snprintf( buf, 100, "n %u, mean %f", buffer->count, buffer->mean);
  snprintf( buf, 100, "mean   %.8f", buffer->mean);
  vfd_write_string2( vfd, buf, 0, 5 );


  char buf2[ 101];

  // this includes the unit
  // TODO fix me.   multiplying unit (eg. k) is correct but 'V' is not.
  // need
  snprintf( buf, 100, "stddev %sV", str_format_value_dynamic( buf2, 100, buffer->stddev, 4 ));

  // snprintf( buf, 100, "stddev %.8f", buffer->stddev);
  vfd_write_string2( vfd, buf, 0, 6 );


  snprintf( buf, 100, "%c", star );
  vfd_write_string2( vfd, buf, 0, 7 );


}


/*
  if(status_seq_mode == SEQ_MODE_RATIO)
    printf(" meas %s", str_format_float_with_commas(buf, 100, 7, data->reading));
  else
    printf(" meas %sV", buf );
*/






#if 0
  // from util.  should deprecate
  sprintf( buf, "%s%s",
    str_format_float_with_commas( buf2, 100, 7, data->reading),
    "" // range->unit
  );
#endif


