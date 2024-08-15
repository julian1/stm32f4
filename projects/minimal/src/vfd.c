

#include <ctype.h>        // toupper
#include <stdio.h>
#include <assert.h>



// vfd
#include <peripheral/vfd.h>
#include <vfd.h>


#include <ice40-reg.h>    // for seq mode
#include <data/data.h>
#include <util.h>

#include <lib2/format.h>  // format_float


#define UNUSED(x) (void)(x)




static void stoupper( char *s)
{
  // inplace
  size_t n  = strlen(s);
  for(unsigned i = 0; i < n; ++i)   // stoupper
    s[i] = toupper( s[i]);

}


// STTCPW


void vfd_update_new_reading(data_t *data)
{
  UNUSED(data);


  char buf[100];

  str_format_float_with_commas(buf, 100, 7, data->computed_val);

/*
  if(sample_seq_mode == SEQ_MODE_RATIO)
    printf(" meas %s", str_format_float_with_commas(buf, 100, 7, data->computed_val));
  else
    printf(" meas %sV", buf );
*/



  uint8_t sample_idx      =  ADC_STATUS_SAMPLE_IDX( data->adc_status) ;       

  uint8_t sample_seq_mode =  ADC_STATUS_SAMPLE_SEQ_MODE( data->adc_status);  



  // write value
  vfd_write_bitmap_string2( buf, 0 , 0 );


  // write mode
  seq_mode_str( sample_seq_mode, buf, 8 );
  stoupper( buf);
  vfd_write_string2( buf, 0, 3 );



  // write nplc
  double nplc = aper_n_to_nplc( data->adc_clk_count_mux_sig, data->line_freq );
  snprintf(buf, 100, "nplc %.1lf ", nplc );
  vfd_write_string2( buf, 0, 4 );


  // write a star, for the sample
  vfd_write_string2( sample_idx % 2 == 0 ? "*" : " ", 0, 5 );


  // write/ dummy other stuff.
  snprintf(buf, 100, "local, range DCV, 10M" );   
  vfd_write_string2( buf, 0, 6 );


}




