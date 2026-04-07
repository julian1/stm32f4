
// consider rename  support.h

#pragma once


#ifdef __cplusplus
extern "C" {
#endif



#include <stdbool.h>
#include <stdint.h>



// move to ice40_reg.h ?
#define CLK_FREQ 20000000

// rename aperture  to just aperture which is always expressed in clock count

uint32_t nplc_to_aperture( double nplc, uint32_t lfreq );

double aperture_to_nplc( uint32_t aperture, uint32_t lfreq);
double aperture_to_period( uint32_t aperture);

uint32_t period_to_aperture(  double period);

bool nplc_valid( double nplc );

void aperture_print( uint32_t aperture,  uint32_t lfreq);

//

unsigned str_decode_uint( const char *s, uint32_t *val);
unsigned str_decode_int( const char *s, int32_t *val);
unsigned str_decode_float( const char *s, double *val);


unsigned str_decode_mux( const char *s, uint32_t *val);

char * mux_to_str( unsigned val,  char *buf, unsigned n);


///////


char * str_format_value( char *s, size_t n,  unsigned ndigits, unsigned leading, double value);
void val_adjust_multiplier( double *val, char *c) ;

void val_set_multiplier( double *val, char c);
unsigned val_leading_digits( double val_);
char * str_format_value_dynamic( char *s, size_t sz, double val, unsigned ndigits);


#ifdef __cplusplus
}
#endif


