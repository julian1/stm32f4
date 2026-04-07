
// consider rename  support.h

#pragma once


#ifdef __cplusplus
extern "C" {
#endif



#include <stdbool.h>
#include <stdint.h>



// move to ice40_reg.h ?
#define CLK_FREQ 20000000

// rename aper_n  to just aperture which is always expressed in clock count

uint32_t nplc_to_aperture( double nplc, uint32_t lfreq );

// TODO rename to aperture.  eg. aperture_to_nplc.
// or aper_cc_to_nplc
double aper_n_to_nplc( uint32_t aper_n, uint32_t lfreq);
double aper_n_to_period( uint32_t aper_n);

uint32_t period_to_aper_n(  double period );

bool nplc_valid( double nplc );

void aper_cc_print( uint32_t aperture,  uint32_t lfreq);

//

unsigned str_decode_uint( const char *s, uint32_t *val);
unsigned str_decode_int( const char *s, int32_t *val);
unsigned str_decode_float( const char *s, double *val);


unsigned str_decode_mux( const char *s, uint32_t *val);

char * mux_to_str( unsigned val,  char *buf, unsigned n  );


///////


char * str_format_value( char *s, size_t n,  unsigned ndigits, unsigned leading, double value );
void val_adjust_multiplier( double *val, char *c) ;

// change name val_set_multipler
void val_force_multiplier( double *val, char c);
unsigned val_leading_digits( double val_);
char * str_format_value_dynamic( char *s, size_t sz, double val, unsigned ndigits);


#ifdef __cplusplus
}
#endif


