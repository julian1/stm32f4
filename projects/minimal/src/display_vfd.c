

#include <ctype.h>        // toupper
#include <stdio.h>
#include <assert.h>
#include <math.h>       // fabs
#include <string.h>       // memcpy
#include <stdbool.h>





#include <lib3/util.h>  // UNUSED
#include <lib3/format.h>  // format_float


#include <support.h>         // format
#include <data/data.h>
#include <data/range.h>
#include <data/buffer.h>

// vfd
#include <peripheral/vfd.h> // magic

#include <peripheral/vfd-font-small.h>
#include <peripheral/vfd-font-large.h>

#include <display-vfd.h>




// STTCPW




void display_vfd_init( display_vfd_t *display, vfd_t *vfd1, buffer_t *buffer)
{
  memset( display, 0, sizeof( display_vfd_t));
  display->magic = VFD_DISPLAY_MAGIC;

  assert( buffer && buffer->magic == BUFFER_MAGIC);

  display->vfd = vfd1;
  display->buffer = buffer;
}




void display_vfd_update( display_vfd_t *display)
{
  assert( display && display->magic == VFD_DISPLAY_MAGIC);

  // do nothing
}


void display_vfd_update_500ms( display_vfd_t *display)
{
  assert( display && display->magic == VFD_DISPLAY_MAGIC);

  // do nothing

}




void display_vfd_update_data( display_vfd_t *display, data_t *data)
{
  assert( display && display->magic == VFD_DISPLAY_MAGIC);
  assert( data && data->magic == DATA_MAGIC);


  vfd_t *vfd = display->vfd;
  assert( vfd);
  // assert(vfd && vfd->magic == VFD_MAGIC);

  // buffer is in the display....
  // not sure if it would be better to pass
  buffer_t *buffer = display->buffer;
  assert( buffer && buffer->magic == BUFFER_MAGIC);


  // feb 2026

  // no valid measurement reading
  if( !data->valid) {

    // clear the reading star, to indicate no measurement data
    char buf[101];
    snprintf( buf, 100, "%c", ' ');
    // vfd_font_small_write( vfd, buf, 0, 3 );

    vfd_font_small_write( vfd, buf, 0, 7 );

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

    // if(ndigits == 9) ...

  // write sign using small font
  vfd_font_small_write( vfd, data->reading >= 0 ? "+" : "-", 0, 0 );

  /*
    vfd. would be really nice to,  intersperse commas, or even just a column of pixels herue.
  */

  // write value in alrge font, starting with offset for the sign
  // vfd_font_large_write_proportional( vfd, val.s, 7, 0 );
  vfd_font_large_write_special( vfd, val.s, 7, 0 );


  char buf[ 130 + 1 ];

  // format the value multiplier and unit, left align, chars are 7bit wide.
  snprintf( buf, 100, "%c%s", val.m, val.u);
  vfd_font_small_write( vfd, buf, (18 - strlen( buf)) * 7, 3 );




  // wont work. until we assert valid for a ZERO.
  uint8_t status_sample_idx = data->status.sample_idx;


  char star  = status_sample_idx % 2 == 1 ? '*' : ' ';


  /*
    TODO .  add the noaz. flag. to the status register
    so we can display mode. here. if AZ. or NOAZ.
    ---
    EXTR  ... could put the 10Meg. flag in the status register as well

    10Meg. is external to the fpga, SA or ADC. so dont add to sr.

    or dig them out of the mode?
    do we just need a better synchronization systm?
      so we dont associate wrong state with reading?

    eg. use the seqence number.  and expected sequence number

    if write the mode/  then need to invalidate

    or on state_transition.   we reset the value.

  */
  bool  noaz = 0; //  data->status.noaz;  or dig out from the mode?

  double nplc = aperture_to_nplc( data->adc_clk_count_sigmux, data->line_freq );

  snprintf( buf, 100, "%s-%s %s",  range->name, range->arg,  noaz ? "NOAZ" : "AZ" );
  vfd_font_small_write( vfd, buf, 0, 3 );



  snprintf( buf, 100, "nplc %.1lf n=%u", nplc,   buffer->count );
  vfd_font_small_write( vfd, buf, 0, 4 );


  // use the range formatting function to format the mean
  range->range_reading_format( range, &val, 9, buffer->mean);
  // snprintf( buf, ARRAY_SIZE(buf), "mean(%u) %s", buffer->count, val.all);
  snprintf( buf, ARRAY_SIZE(buf), "mean %s", val.all);

  vfd_font_small_write_special( vfd, buf, 0, 5 );


  char buf2[ 101];

  // this includes the unit
  // TODO fix me.   multiplying unit (eg. k) is correct but 'V' is not.
  // need
  snprintf( buf, ARRAY_SIZE(buf), "stddev %sV", str_format_value_dynamic( buf2, 100, buffer->stddev, 4 ));

  // snprintf( buf, 100, "stddev %.8f", buffer->stddev);
  vfd_font_small_write_special( vfd, buf, 0, 6 );


  snprintf( buf, 100, "%c", star );
  vfd_font_small_write( vfd, buf, 0, 7 );


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


